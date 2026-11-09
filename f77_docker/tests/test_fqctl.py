import contextlib
import importlib.util
import io
import json
import os
import shlex
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

HERE = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location('fqctl', HERE / 'fqctl.py')
FQ = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(FQ)

PARA = '''      character*200 ASTROMETRY_CAT,SOURCE_CAT,FLAT_PATH,PSF_PATH
      parameter (ASTROMETRY_CAT
     .='/old/site/gaia')
      parameter (SOURCE_CAT='/old/site/source'
     .//'/deep')
      parameter (FLAT_PATH='/old/site/flat')
      parameter (PSF_PATH='hahahaha')
      integer PROCESS_stage
      parameter (PROCESS_stage=2*3*5*7*11*13*17*19*23)
      real pixel_size
      parameter (pixel_size=0.2628)
      integer PreprocsType
      parameter (PreprocsType=1)
'''

class LauncherContracts(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory(prefix='f77 paths with spaces ')
        self.base = Path(self.tmp.name)
        self.source = self.base / 'f77'
        self.source.mkdir()
        (self.source / 'Makefile').write_text('all:\n\t@true\nclean:\n\t@true\n')
        (self.source / 'para.inc').write_text(PARA)
        (self.source / '00_psf_module.f').write_text('      end\n')
        (self.source / 'main.f').write_text('      program check_par\n      include "para.inc"\n      print *, ASTROMETRY_CAT,PROCESS_stage\n      end\n')
        self.project = self.base / 'project'
        self.project.mkdir()
        self.data = self.base / 'survey data'
        self.data.mkdir()
        self.catalog = self.base / 'Gaia tiles'
        self.catalog.mkdir()
        (self.data / 'expo_gband.list').write_text('"/data/DataProcess/expolists/exp001.list" 2\n')
        self.file = self.project / 'project.json'
        self.cfg = dict(version=1, source=str(self.source), run_directory='run',
                        image='test:toolchain', ranks=4, build_jobs=3,
                        mounts=[dict(host=str(self.data), container='/data/DataProcess',mode='rw'),
                                dict(host=str(self.catalog), container='/data/catalogs/GaiaDir',mode='ro')],
                        paths=dict(exposure_list='/data/DataProcess/expo_gband.list',astrometry_cat='/data/catalogs/GaiaDir',source_cat=None,flat_path=None,psf_path=None),
                        hpc=dict(sif=str(self.base/'fake.sif'),runtime='apptainer',mpi='pmi2',srun_args=[],environment={}))
    def tearDown(self): self.tmp.cleanup()
    def parsed(self):
        self.file.write_text(json.dumps(self.cfg))
        return FQ.read_project(self.file)
    def configured(self):
        args=self.parsed()
        with contextlib.redirect_stdout(io.StringIO()): FQ.configure(*args)
        return args
    def test_generated_fortran_is_isolated_and_uses_compiled_rest(self):
        a=self.configured()
        para=(a[3]/'source/para.inc').read_text()
        self.assertIn('/data/catalogs/GaiaDir',para)
        self.assertIn('PROCESS_stage=2*3*5*7*11*13*17*19*23',para)
        self.assertIn('pixel_size=0.2628',para)
        self.assertIn('PreprocsType=1',para)
        self.assertEqual((self.source/'para.inc').read_text(), PARA)
        FQ.check(*a,announce=False)
    def test_fortran_fixed_format_compiles(self):
        a=self.configured()
        if not shutil.which('gfortran'): self.skipTest('gfortran not installed')
        x=subprocess.run(['gfortran','-ffixed-form','-fsyntax-only','main.f'],cwd=a[3]/'source',text=True,capture_output=True)
        self.assertEqual(x.returncode,0,x.stderr)
    def test_compile_path_with_quotes_and_long_value(self):
        v="/data/" + "segment/"*22 + "O'Brien"
        self.cfg['paths']['astrometry_cat'] = v
        self.cfg['mounts'].append(dict(host=str(self.catalog),container='/data',mode='ro'))
        args=self.parsed()
        text=FQ.expected_para(args[1], args[2])
        self.assertIn("O''Brien",text)
        for line in text.splitlines():
            if 'parameter' in line or line.startswith('     .'):
                self.assertLessEqual(len(line),72)
    def test_fixed_form_quote_pair_at_chunk_boundary(self):
        name='q'*41+"'"+'suffix'
        (self.catalog/name).mkdir()
        self.cfg['paths']['astrometry_cat']='/data/'+name
        self.cfg['mounts'].append(dict(host=str(self.catalog),container='/data',mode='ro'))
        a=self.configured()
        para=(a[3]/'source/para.inc').read_text()
        self.assertIn("''",para)
        if shutil.which('gfortran'):
            p=subprocess.run(['gfortran','-ffixed-form','-fsyntax-only','main.f'],cwd=a[3]/'source',capture_output=True,text=True)
            self.assertEqual(p.returncode,0,p.stderr)

    def test_rerun_configure_keeps_compiled_binary(self):
        a=self.configured()
        binary=a[3]/'source/Fourier_Quad_Pipe';binary.write_text('executable');binary.chmod(0o755)
        self.configured()
        self.assertEqual(binary.read_text(),'executable')
    def test_editing_source_requires_regeneration(self):
        a=self.configured()
        (self.source/'main.f').write_text('      end\n')
        with self.assertRaisesRegex(FQ.ConfigError,'stale'): FQ.check(*a,announce=False)
    def test_changed_path_updates_snapshot_without_modifying_source(self):
        self.configured()
        (self.base/'another').mkdir()
        self.cfg['mounts'].append(dict(host=str(self.base/'another'),container='/extra',mode='ro'))
        a=self.parsed()
        with self.assertRaises(FQ.ConfigError):FQ.check(*a,announce=False)
        FQ.configure(*a)
        FQ.check(*a,announce=False)
    def test_named_path_must_be_covered(self):
        self.cfg['paths']['exposure_list']='/not/mounted/list.txt'
        with self.assertRaisesRegex(FQ.ConfigError,'no mount'):self.configured()
    def test_top_exposure_list_must_exist(self):
        self.cfg['paths']['exposure_list']='/data/DataProcess/absent.list'
        with self.assertRaisesRegex(FQ.ConfigError,'does not exist'):self.configured()
    def test_extra_nested_mount_supported(self):
        sub=self.base/'nested';sub.mkdir()
        self.cfg['mounts'].append(dict(host=str(sub),container='/data/DataProcess/extra',mode='rw'))
        a=self.configured();FQ.check(*a,announce=False)
    def test_reserved_system_mount_rejected(self):
        self.cfg['mounts'][0]['container']='/workspace/f77'
        with self.assertRaisesRegex(FQ.ConfigError,'protected'):self.parsed()
    def test_duplicate_target_rejected(self):
        self.cfg['mounts'].append(dict(host=str(self.catalog),container='/data/DataProcess'))
        with self.assertRaisesRegex(FQ.ConfigError,'duplicate'):self.parsed()
    def test_only_full_f77_source_is_supported(self):
        (self.source/'00_psf_module.f').unlink()
        with self.assertRaisesRegex(FQ.ConfigError, '00_psf_module.f'):
            self.parsed()

    def test_legacy_variant_json_is_rejected(self):
        self.cfg['variant'] = 'f77'
        with self.assertRaisesRegex(FQ.ConfigError, 'unknown keys'):
            self.parsed()

    def test_variant_cli_is_removed(self):
        import sys
        result=subprocess.run([sys.executable,str(HERE/'fqctl.py'),'init',
                               '--variant','f77','--project',str(self.base/'new-project')],
                              capture_output=True,text=True)
        self.assertEqual(result.returncode,2)
        self.assertIn('unrecognized arguments',result.stderr)

    def test_docker_mpi_exposure_arg_contract(self):
        a=self.configured();cmd=FQ.docker_args(*a[:4],a[4],'run')
        self.assertEqual(cmd[-5:],['mpiexec','-n','4','/workspace/f77/Fourier_Quad_Pipe','/data/DataProcess/expo_gband.list'])
        self.assertNotIn('--config',cmd)
        self.assertTrue(any('readonly' in x and 'src=' in x for x in cmd))
    def test_build_command_uses_container_stack(self):
        a=self.configured();cmd=FQ.docker_args(*a[:4],a[4],'build')
        self.assertIn('make clean',cmd[-3]);self.assertIn('CFITSIO_LIB=/opt/f77stack/lib/libcfitsio.so',cmd[-3])
    def test_missing_required_exposure_list(self):
        self.cfg['paths']['exposure_list']=None
        with self.assertRaises(FQ.ConfigError):self.configured()
    def test_invalid_environment_object_has_clean_error(self):
        self.cfg['environment']=['not', 'an', 'object']
        with self.assertRaisesRegex(FQ.ConfigError,'JSON object'):
            self.parsed()

    def test_cli_init_creates_editable_json(self):
        import sys
        new_project=self.base/'another-project'
        process=subprocess.run([sys.executable,str(HERE/'fqctl.py'),'init',
                                '--project',str(new_project)],capture_output=True,text=True)
        self.assertEqual(process.returncode,0,process.stderr)
        config=json.loads((new_project/'project.json').read_text())
        self.assertNotIn('variant',config)
        self.assertEqual(Path(config['source']).resolve(),(HERE.parent/'f77').resolve())
        self.assertIsNone(config['paths']['exposure_list'])

    def test_oversized_path_rejected(self):
        self.cfg['paths']['astrometry_cat']='/data/'+'x'*200
        with self.assertRaises(FQ.ConfigError):self.parsed()
    def test_env_shell_quoting_prevents_command_substitution(self):
        marker=self.base/'injected'
        self.cfg['environment']={'NOTE':'$(touch '+str(marker)+')'}
        a=self.configured()
        env=a[3]/'f77pipeline.env'
        proc=subprocess.run(['bash','-c','source "$1"; printf "%s\n" "${F77_CONTAINER_ENV[@]}"','bash',str(env)],capture_output=True,text=True)
        self.assertEqual(proc.returncode,0,proc.stderr)
        self.assertFalse(marker.exists());self.assertIn('$(touch',proc.stdout)
    def test_hpc_explicit_check_no_sif_required(self):
        a=self.configured()
        proc=subprocess.run(['bash',str(HERE/'runner/run.sh'),'--check',str(a[3]/'f77pipeline.env')],capture_output=True,text=True)
        self.assertEqual(proc.returncode,0,proc.stderr)
    def test_slurm_entry_does_not_validate_job_identity(self):
        a=self.configured()
        d=self.base/'bin';d.mkdir()
        script=d/'srun'
        script.write_text('#!/bin/sh\nprintf "%s\\n" "$@" > "$RESULT_FILE"\n');script.chmod(0o755)
        result=self.base/'result'
        env=dict(os.environ,PATH=str(d)+os.pathsep+os.environ['PATH'],RESULT_FILE=str(result))
        for k in tuple(env):
            if k.startswith('SLURM_'):env.pop(k)
        proc=subprocess.run(['bash',str(HERE/'runner/run.sh'),str(a[3]/'f77pipeline.env')],env=env,capture_output=True,text=True)
        self.assertEqual(proc.returncode,0,proc.stderr)
        self.assertIn('--mpi=pmi2',result.read_text())
        self.assertNotIn('--ntasks=',result.read_text())
    def test_pull_sif_supports_generated_env_and_old_oci_image_variable(self):
        a=self.configured()
        fake=self.base/'runtime-bin'
        fake.mkdir()
        program=fake/'apptainer'
        program.write_text('#!/bin/sh\nprintf "%s\\n" "$@" > "$RESULT_FILE"\n')
        program.chmod(0o755)
        result=self.base/'pull.args'
        env=dict(os.environ,PATH=str(fake)+os.pathsep+os.environ['PATH'],RESULT_FILE=str(result))
        script=HERE/'runner/pull-sif.sh'
        proc=subprocess.run(['bash',str(script),str(a[3]/'f77pipeline.env')],env=env,text=True,capture_output=True)
        self.assertEqual(proc.returncode,0,proc.stderr)
        self.assertIn('docker://test:toolchain',result.read_text())
        legacy=self.base/'legacy.env'
        legacy.write_text('OCI_IMAGE_URI=old/toolchain:tag\nF77_SIF='+shlex.quote(str(self.base/'legacy.sif'))+'\n')
        env['F77_RUNNER_ENV_FILE']=str(legacy)
        proc=subprocess.run(['bash',str(script)],env=env,text=True,capture_output=True)
        self.assertEqual(proc.returncode,0,proc.stderr)
        self.assertIn('docker://old/toolchain:tag',result.read_text())

    def test_per_rank_pmi_forwarding_and_exit_status(self):
        a=self.configured();(self.base/'fake.sif').write_text('fake')
        d=self.base/'bin';d.mkdir();res=self.base/'env.json'
        app=d/'apptainer'
        app.write_text('#!/usr/bin/env python3\nimport json,os,sys\nopen(os.environ["RESULT_FILE"],"w").write(json.dumps({"argv":sys.argv,"rank":os.getenv("APPTAINERENV_PMI_RANK"),"old":os.getenv("APPTAINERENV_OLD")}))\nsys.exit(17)\n')
        app.chmod(0o755)
        env=dict(os.environ,PATH=str(d)+os.pathsep+os.environ['PATH'],RESULT_FILE=str(res),PMI_RANK='7',APPTAINERENV_OLD='poison')
        proc=subprocess.run(['bash',str(HERE/'runner/runtime.sh'),str(a[3]/'f77pipeline.env'),'run'],env=env,capture_output=True,text=True)
        self.assertEqual(proc.returncode,17,proc.stderr)
        obj=json.loads(res.read_text());self.assertEqual(obj['rank'],'7');self.assertIsNone(obj['old'])
        self.assertEqual(obj['argv'][-2:],['/workspace/f77/Fourier_Quad_Pipe','/data/DataProcess/expo_gband.list'])



class PipelineRepositoryContracts(unittest.TestCase):
    def test_default_image_matches_pipeline_registry(self):
        self.assertEqual(FQ.IMAGE,
                         'ghcr.io/syoong-s/fourier_quad_pipeline/f77pipeline:gnu4.8.5')

    def test_pipeline_full_source_has_matching_fortran_path_definitions(self):
        source = HERE.parent.parent / 'f77'
        if not (source / 'para.inc').is_file():
            self.skipTest('Full f77 source tree is not mounted into this fixture')
        full = (source / 'para.inc').read_text(encoding='utf-8')
        self.assertTrue((source / '00_psf_module.f').is_file())
        self.assertTrue((source / 'Makefile').is_file())
        paths = {name: '/data/test/' + key for key, name in FQ.PATH_FIELDS.items()}
        patched = FQ.fixed_form_override(full, paths)
        for key, name in FQ.PATH_FIELDS.items():
            self.assertIn('/data/test/' + key, patched)
        self.assertIn('PROCESS_stage', patched)
        self.assertEqual((source / 'para.inc').read_text(encoding='utf-8'), full)
        self.assertIn('parameter (PROCESS_stage',patched)

    def test_pipeline_main_accepts_exposure_list_positional_argument(self):
        source = HERE.parent.parent / 'f77'
        if not (source / 'main.f').is_file():
            self.skipTest('Full f77 source tree is not mounted into this fixture')
        main = (source / 'main.f').read_text(encoding='utf-8').lower()
        self.assertIn('get_command_argument(1,expo_list', main)
        self.assertNotIn('--config', main)


if __name__ == '__main__': unittest.main()
