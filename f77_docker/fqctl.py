#!/usr/bin/env python3
"""Fortran77 pipeline deployment: JSON -> isolated build source + HPC env.

Python 3.8+ standard library only. No INI parser is added to the F77 program.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path, PurePosixPath
import re
import shlex
import shutil
import subprocess
import sys
import tempfile

HERE = Path(__file__).resolve().parent
ROOT = HERE.parent
SOURCE_DEST = '/workspace/f77'
IMAGE = 'ghcr.io/syoong-s/fourier_quad_pipeline/f77pipeline:gnu4.8.5'
PATH_FIELDS = {'astrometry_cat': 'ASTROMETRY_CAT',
               'source_cat': 'SOURCE_CAT', 'flat_path': 'FLAT_PATH',
               'psf_path': 'PSF_PATH'}
RESERVED = ('/bin', '/sbin', '/usr', '/lib', '/lib64', '/etc', '/opt',
            '/proc', '/sys', '/dev', '/tmp', '/home', '/root', '/workspace')

class ConfigError(ValueError):
    pass


def require(ok, message):
    if not ok:
        raise ConfigError(message)


def fields(obj, allowed, name):
    require(isinstance(obj, dict), name + ' must be a JSON object')
    other = set(obj) - set(allowed)
    require(not other, name + ': unknown keys: ' + ', '.join(sorted(other)))


def nonempty(value, name):
    require(isinstance(value, str) and bool(value.strip()), name + ' must be a nonempty string')
    require(not any(ord(x) < 32 or ord(x) == 127 for x in value), name + ' contains control characters')
    return value


def integer(value, name, minimum=1):
    require(type(value) is int and value >= minimum, name + ' must be an integer >= ' + str(minimum))
    return value


def host_path(value, base, name):
    value = nonempty(value, name)
    require(':' not in value and ',' not in value, name + ': host paths cannot contain : or ,')
    p = Path(value).expanduser()
    p = (p if p.is_absolute() else base / p).resolve()
    require(':' not in str(p) and ',' not in str(p), name + ' resolves to an unsupported path')
    return p


def container_path(value, name, reserved=False):
    value = nonempty(value, name)
    p = PurePosixPath(value)
    require(p.is_absolute() and value != '/' and not value.startswith('//') and '..' not in p.parts,
            name + ' must be an absolute non-root path without ..')
    require(':' not in value and ',' not in value and not any(ord(x) < 32 for x in value),
            name + ': container paths cannot contain : or ,')
    path = str(p)
    if reserved:
        require(not any(path == v or path.startswith(v + '/') or v.startswith(path + '/') for v in RESERVED),
                name + ' overlaps a protected container directory: ' + path)
    return path


def document_path(value):
    p = Path(value).expanduser()
    if p.is_dir() or p.suffix.lower() != '.json':
        p = p / 'project.json'
    return p.resolve()


def parse_json(path):
    def no_duplicate(pairs):
        result = {}
        for k, v in pairs:
            require(k not in result, 'duplicate JSON key: ' + k)
            result[k] = v
        return result
    try:
        return json.loads(path.read_text(encoding='utf-8'), object_pairs_hook=no_duplicate)
    except (OSError, ValueError) as exc:
        raise ConfigError('Unable to read ' + str(path) + ': ' + str(exc))


def digest(data):
    return hashlib.sha256(data).hexdigest()


def cfg_hash(cfg):
    return digest(json.dumps(cfg, sort_keys=True, ensure_ascii=False).encode('utf-8'))


def check_env_vars(obj, name):
    require(isinstance(obj, dict), name + ' must be a JSON object')
    for k, v in obj.items():
        require(bool(re.fullmatch(r'[A-Za-z_][A-Za-z0-9_]*', k)), name + ': invalid variable ' + k)
        require(not k.startswith(('SLURM_', 'PMI_', 'PMI2_', 'APPTAINER', 'SINGULARITY')),
                name + ': reserved variable ' + k)
        require(isinstance(v, str) and '\n' not in v and '\r' not in v and '\x00' not in v,
                name + ': values must be single-line strings')


def read_project(spec):
    path = document_path(spec)
    cfg = parse_json(path)
    fields(cfg, ('version', 'source', 'run_directory', 'image', 'ranks',
                 'build_jobs', 'mounts', 'paths', 'environment', 'hpc'), 'project')
    require(type(cfg.get('version')) is int and cfg['version'] == 1, 'version must be 1')
    source = host_path(cfg.get('source', '../f77'), path.parent, 'source')
    rundir = host_path(cfg.get('run_directory', './fq-run'), path.parent, 'run_directory')
    require(source.is_dir() and (source / 'Makefile').is_file() and (source / 'para.inc').is_file(),
            'source must be a Fortran77 source tree containing Makefile and para.inc: ' + str(source))
    require((source / '00_psf_module.f').is_file(),
            'source must be the complete f77 program (missing 00_psf_module.f)')
    require(source != rundir and source not in rundir.parents and rundir not in source.parents,
            'run_directory and source must not contain one another')
    image = nonempty(cfg.get('image', IMAGE), 'image')
    require(not image.startswith('-') and not any(x.isspace() for x in image), 'invalid image reference')
    integer(cfg.get('ranks', 4), 'ranks', 2)
    integer(cfg.get('build_jobs', 4), 'build_jobs')
    paths = cfg.get('paths', {})
    fields(paths, tuple(PATH_FIELDS) + ('exposure_list',), 'paths')
    for k, v in paths.items():
        if v is not None:
            require(isinstance(v, str), 'paths.' + k + ' must be a string or null')
            container_path(v, 'paths.' + k)
            if k != 'exposure_list':
                require(len(v.encode('utf-8')) <= 200 and "\x00" not in v,
                        'paths.' + k + ' exceeds Fortran CHARACTER*200 limit')
            if k == 'exposure_list':
                require(len(v.encode('utf-8')) <= 512, 'exposure_list exceeds compiled strl=512')
    mounts = cfg.get('mounts', [])
    require(isinstance(mounts, list), 'mounts must be a JSON array')
    seen = set()
    result = []
    for i, item in enumerate(mounts):
        label = 'mounts[' + str(i) + ']'
        fields(item, ('host', 'container', 'mode', 'create', 'kind'), label)
        host = host_path(item.get('host'), path.parent, label + '.host')
        target = container_path(item.get('container'), label + '.container', reserved=True)
        mode = item.get('mode', 'ro')
        kind = item.get('kind', 'dir')
        create = item.get('create', False)
        require(mode in ('ro', 'rw') and kind in ('dir', 'file'), label + ': invalid mode/kind')
        require(type(create) is bool and (not create or (mode == 'rw' and kind == 'dir')),
                label + ': create requires mode=rw, kind=dir')
        require(target not in seen, 'duplicate container mount destination: ' + target)
        seen.add(target)
        result.append(dict(host=host, container=target, mode=mode, kind=kind, create=create))
    env = cfg.get('environment', {})
    check_env_vars(env, 'environment')
    hpc = cfg.get('hpc', {})
    fields(hpc, ('sif', 'runtime', 'mpi', 'srun_args', 'environment'), 'hpc')
    runtime = nonempty(hpc.get('runtime', 'apptainer'), 'hpc.runtime')
    require('/' not in runtime or runtime.startswith('/'), 'hpc.runtime must be a name or absolute path')
    require(hpc.get('mpi', 'pmi2') == 'pmi2', 'the provided HPC launch profile is pmi2 only')
    args = hpc.get('srun_args', [])
    require(isinstance(args, list) and all(isinstance(a, str) and re.fullmatch(r'--(?:cpu-bind|distribution|hint|mem-bind)=[^\s]+', a) for a in args),
            'hpc.srun_args accepts only --cpu-bind=, --distribution=, --hint= or --mem-bind=')
    check_env_vars(hpc.get('environment', {}), 'hpc.environment')
    sif = host_path(hpc.get('sif', './f77pipeline.sif'), path.parent, 'hpc.sif')
    return path, cfg, source, rundir, result, sif


def mount_for(target, mounts):
    matches = []
    for m in mounts:
        base = m['container']
        if target == base:
            matches.append((len(base), m, Path(m['host'])))
        elif m['kind'] == 'dir' and target.startswith(base + '/'):
            matches.append((len(base), m, Path(m['host']) / target[len(base) + 1:]))
    require(matches, 'no mount covers container path: ' + target)
    _, mount, host = max(matches, key=lambda x: x[0])
    return mount, host


def check_mounts(mounts, create=False):
    for m in mounts:
        host = m['host']
        if create and m['create'] and not host.exists():
            host.mkdir(parents=True, exist_ok=True)
        require(host.is_dir() if m['kind'] == 'dir' else host.is_file(),
                'mount source is missing or has the wrong type: ' + str(host))
        require(os.access(str(host), os.R_OK) and (m['kind'] == 'file' or os.access(str(host), os.X_OK)),
                'mount is not readable: ' + str(host))
        if m['mode'] == 'rw':
            require(os.access(str(host), os.W_OK), 'mount is not writable: ' + str(host))


def check_named_paths(cfg, mounts):
    paths = cfg.get('paths', {})
    require(bool(paths.get('exposure_list')), 'paths.exposure_list is required: F77 main.f takes a positional exposure-list path')
    for name, value in paths.items():
        if not value:
            continue
        mount, host = mount_for(value, mounts)
        if name == 'exposure_list':
            require(host.is_file(), 'exposure_list does not exist as a file: ' + str(host))
        else:
            require(host.is_dir(), name + ' does not exist as a directory: ' + str(host))
        require(os.access(str(host), os.R_OK), name + ': input is not readable: ' + str(host))
    return paths['exposure_list']


def source_fingerprint(source):
    h = hashlib.sha256()
    for p in sorted(source.iterdir()):
        if p.is_file() and (p.name in ('Makefile', 'para.inc', 'cust_para.inc', 'sig_para.inc', 'path_layout.inc') or p.suffix.lower() in ('.f', '.f90')):
            require(not p.is_symlink(), 'source symlinks are not supported: ' + str(p))
            h.update(p.name.encode('utf-8') + b'\0')
            h.update(p.read_bytes())
    return h.hexdigest()


def fixed_form_override(contents, mapping):
    """Replace only named Fortran PARAMETER statements, including continuations."""
    lines = contents.splitlines(keepends=True)
    replacements = {}
    for name, value in mapping.items():
        matches = []
        for i, line in enumerate(lines):
            if re.match(r'^      parameter\s*\(\s*' + re.escape(name) + r'(?:\s|\n|=)', line, re.I):
                matches.append(i)
        require(len(matches) == 1, 'expected exactly one Fortran PARAMETER statement for ' + name)
        start = matches[0]
        end = start + 1
        while end < len(lines) and len(lines[end]) > 5 and lines[end][5] not in (' ', '0', '\n') and lines[end][:5].strip() == '':
            end += 1
        # Keep doubled Fortran quotes together when chunking fixed-form strings.
        chunks = []
        chunk = ''
        for char in value:
            token = "''" if char == "'" else char
            if len(chunk) + len(token) > 48:
                chunks.append(chunk)
                chunk = ''
            chunk += token
        chunks.append(chunk)
        expression = '      parameter (' + name + '=\n'
        for j, chunk in enumerate(chunks):
            expression += "     .'" + chunk + "'" + ('//' if j != len(chunks) - 1 else ')') + '\n'
        require(all(len(l.rstrip('\n')) <= 72 for l in expression.splitlines(keepends=True)),
                name + ' generated Fortran line exceeds fixed-form column 72')
        replacements[start] = (end, expression)
    output = []
    i = 0
    while i < len(lines):
        if i in replacements:
            end, expression = replacements[i]
            output.append(expression)
            i = end
        else:
            output.append(lines[i])
            i += 1
    return ''.join(output)


def expected_para(cfg, source):
    paths = cfg.get('paths', {})
    requested = {name: paths[k] for k, name in PATH_FIELDS.items() if paths.get(k) is not None}
    return fixed_form_override((source / 'para.inc').read_text(encoding='utf-8'), requested)


def shell_array(name, values):
    return name + '=(' + ' '.join(shlex.quote(str(v)) for v in values) + ')\n'


def env_text(path, cfg, source, rundir, mounts, sif):
    hpc = cfg.get('hpc', {})
    pairs = {
        'F77_ENV_VERSION': '1', 'F77_PROJECT_JSON': str(path),
        'F77_PROJECT_HASH': cfg_hash(cfg), 'F77_SOURCE_HASH': source_fingerprint(source),
        'F77_STAGED_SOURCE_HOST': str(rundir / 'source'),
        'F77_SOURCE_CONTAINER': SOURCE_DEST, 'F77_EXECUTABLE': SOURCE_DEST + '/Fourier_Quad_Pipe',
        'F77_EXPO_LIST_CONTAINER': cfg.get('paths', {}).get('exposure_list', ''),
        'F77_SIF': str(sif), 'F77_OCI_IMAGE': cfg.get('image', IMAGE),
        'F77_RUNTIME': hpc.get('runtime', 'apptainer'),
        'F77_BUILD_JOBS': cfg.get('build_jobs', 4),
    }
    text = '# Generated from project.json. This file is trusted Bash input; do not edit it.\n'
    text += ''.join(k + '=' + shlex.quote(str(v)) + '\n' for k, v in pairs.items())
    text += shell_array('F77_MOUNT_HOSTS', [m['host'] for m in mounts])
    text += shell_array('F77_MOUNT_TARGETS', [m['container'] for m in mounts])
    text += shell_array('F77_MOUNT_MODES', [m['mode'] for m in mounts])
    text += shell_array('F77_SRUN_ARGS', hpc.get('srun_args', []))
    env = {'OMP_NUM_THREADS': '1', 'OPENBLAS_NUM_THREADS': '1'}
    env.update(cfg.get('environment', {}))
    env.update(hpc.get('environment', {}))
    text += shell_array('F77_CONTAINER_ENV', [k + '=' + v for k, v in sorted(env.items())])
    return text


def atomic_write(file, content, mode=0o600):
    file.parent.mkdir(parents=True, exist_ok=True)
    fd, tmp = tempfile.mkstemp(prefix='.' + file.name + '-', dir=str(file.parent))
    try:
        with os.fdopen(fd, 'w', encoding='utf-8', newline='\n') as fp:
            fp.write(content)
        os.chmod(tmp, mode)
        os.replace(tmp, file)
    finally:
        if os.path.exists(tmp):
            os.unlink(tmp)


def configure(path, cfg, source, rundir, mounts, sif):
    check_mounts(mounts, create=True)
    check_named_paths(cfg, mounts)
    para = expected_para(cfg, source)
    stamp = source_fingerprint(source) + ':' + digest(para.encode('utf-8'))
    stage = rundir / 'source'
    old_stamp = (stage / '.fq-stage-stamp').read_text() if (stage / '.fq-stage-stamp').is_file() else ''
    if old_stamp != stamp:
        rundir.mkdir(parents=True, exist_ok=True)
        staging = Path(tempfile.mkdtemp(prefix='.source-', dir=str(rundir)))
        try:
            for original in source.iterdir():
                if original.is_file() and not original.is_symlink() and (original.suffix.lower() in ('.f', '.f90', '.inc') or original.name == 'Makefile'):
                    shutil.copy2(original, staging / original.name)
            (staging / 'para.inc').write_text(para, encoding='utf-8')
            (staging / '.fq-stage-stamp').write_text(stamp)
            if stage.exists():
                shutil.rmtree(stage)
            os.replace(staging, stage)
        finally:
            if staging.exists():
                shutil.rmtree(staging)
    content = env_text(path, cfg, source, rundir, mounts, sif)
    atomic_write(rundir / 'f77pipeline.env', content)
    print('Configured source: ' + str(stage))
    print('Generated HPC env: ' + str(rundir / 'f77pipeline.env'))
    print('Compile the staged source once. Scientific parameters remain in the original include files.')


def check(path, cfg, source, rundir, mounts, sif, announce=True):
    check_mounts(mounts)
    check_named_paths(cfg, mounts)
    stage = rundir / 'source'
    para = expected_para(cfg, source)
    expected_stamp = source_fingerprint(source) + ':' + digest(para.encode('utf-8'))
    require((stage / '.fq-stage-stamp').is_file() and (stage / '.fq-stage-stamp').read_text() == expected_stamp,
            'staged source is missing or stale; run configure')
    require((stage / 'para.inc').is_file() and (stage / 'para.inc').read_text(encoding='utf-8') == para,
            'staged para.inc differs from configured paths; run configure')
    env = rundir / 'f77pipeline.env'
    require(env.is_file() and env.read_text(encoding='utf-8') == env_text(path, cfg, source, rundir, mounts, sif),
            'generated HPC env is missing or stale; run configure')
    if announce:
        print('Configuration check passed: project JSON, source snapshot, Fortran path constants, mounts, and HPC env.')
        print('Scientific FITS contents and cluster MPI compatibility are not checked.')


def docker_args(path, cfg, source, rundir, mounts, action):
    stage = rundir / 'source'
    command = ['docker', 'run', '--rm', '--init', '--platform', 'linux/amd64',
               '--user', str(os.getuid()) + ':' + str(os.getgid()), '--workdir', SOURCE_DEST]
    for gid in sorted(set(os.getgroups()) - {os.getgid()}):
        command += ['--group-add', str(gid)]
    env = {'HOME': '/tmp', 'OMP_NUM_THREADS': '1', 'OPENBLAS_NUM_THREADS': '1'}
    env.update(cfg.get('environment', {}))
    for k, v in sorted(env.items()):
        command += ['--env', k + '=' + v]
    stagebind = 'type=bind,src=' + str(stage) + ',dst=' + SOURCE_DEST
    if action == 'run':
        stagebind += ',readonly'
    command += ['--mount', stagebind]
    if action == 'run':
        for m in mounts:
            bind = 'type=bind,src=' + str(m['host']) + ',dst=' + m['container']
            if m['mode'] == 'ro':
                bind += ',readonly'
            command += ['--mount', bind]
    command.append(cfg.get('image', IMAGE))
    if action == 'build':
        command += ['bash', '-c',
                    'set -euo pipefail; make clean; exec make -j "$1" '
                    'LAPACK_LIB_DIR=/opt/f77stack/lib CFITSIO_LIB_DIR=/opt/f77stack/lib '
                    'CFITSIO_LIB=/opt/f77stack/lib/libcfitsio.so',
                    'fq-build', str(cfg.get('build_jobs', 4))]
    else:
        command += ['mpiexec', '-n', str(cfg.get('ranks', 4)),
                    SOURCE_DEST + '/Fourier_Quad_Pipe', cfg['paths']['exposure_list']]
    return command


def execute(args):
    require(shutil.which(args[0]) is not None, 'required executable not found: ' + args[0])
    os.execvp(args[0], args)


def init(args):
    target = document_path(args.project)
    require(not target.exists(), 'project JSON already exists: ' + str(target))
    cfg = dict(version=1, source=str(ROOT / 'f77'),
               run_directory='./fq-run', image=IMAGE, ranks=4, build_jobs=4,
               mounts=[], paths=dict(astrometry_cat=None, source_cat=None,
                                     flat_path=None, psf_path=None, exposure_list=None),
               environment={}, hpc=dict(sif='./f77pipeline.sif', runtime='apptainer',
                                        mpi='pmi2', srun_args=[], environment={}))
    atomic_write(target, json.dumps(cfg, indent=2, ensure_ascii=False) + '\n', 0o644)
    print('Created ' + str(target))
    print('Set mounts and paths.exposure_list; then run configure.')


def main(argv=None):
    parser = argparse.ArgumentParser(description='Configure and launch the Fortran77 Fourier_Quad pipeline')
    sub = parser.add_subparsers(dest='command', required=True)
    for name in ('init', 'configure', 'check', 'pull', 'build', 'run'):
        p = sub.add_parser(name)
        p.add_argument('--project', default=str(HERE / 'project.json'), help='JSON file or containing directory')
    args = parser.parse_args(argv)
    try:
        if args.command == 'init':
            init(args)
        else:
            parsed = read_project(args.project)
            path, cfg, source, rundir, mounts, sif = parsed
            if args.command == 'configure':
                configure(*parsed)
            elif args.command == 'pull':
                execute(['docker', 'pull', '--platform', 'linux/amd64', cfg.get('image', IMAGE)])
            else:
                check(*parsed, announce=args.command == 'check')
                if args.command in ('build', 'run'):
                    if args.command == 'run':
                        require(os.access(str(rundir / 'source' / 'Fourier_Quad_Pipe'), os.X_OK),
                                'compiled executable is missing; run build first')
                    execute(docker_args(path, cfg, source, rundir, mounts, args.command))
        return 0
    except (OSError, ConfigError, ValueError) as exc:
        print('Error: ' + str(exc), file=sys.stderr)
        return 2

if __name__ == '__main__':
    sys.exit(main())
