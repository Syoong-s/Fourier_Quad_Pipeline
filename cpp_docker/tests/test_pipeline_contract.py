"""Validate the launcher against this repository's C++ Standard contract."""
import re
import sys
import json
import tempfile
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "cpp_docker"))
from schema import sections, path_keys
import fqctl


class PipelineContractTests(unittest.TestCase):
    def test_real_standard_header_is_accepted_by_launcher(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "input").mkdir()
            (root / "output").mkdir()
            project = root / "project.json"
            project.write_text(json.dumps({
                "version": 1,
                "source": str(ROOT / "cpp_Standard"),
                "run_directory": str(root / "generated"),
                "mounts": [
                    {"host": str(root / "input"), "container": "/data/input", "mode": "ro"},
                    {"host": str(root / "output"), "container": "/data/output", "mode": "rw"}
                ],
                "paths": {
                    "init.science_root": "/data/input",
                    "init.output_root": "/data/output"
                }
            }), encoding="utf-8")
            result = fqctl.load_project(project)
            self.assertEqual(result["source"], (ROOT / "cpp_Standard").resolve())

    def test_supported_source_tree(self):
        cpp = ROOT / "cpp_Standard"
        for relative in ("Makefile", "main.cpp", "src/RuntimeConfig.cpp",
                         "include/RuntimeConfig.hpp", "pipeline.example.ini"):
            with self.subTest(relative=relative):
                self.assertTrue((cpp / relative).is_file(), relative)

    def test_schema_matches_ini_template(self):
        template = (ROOT / "cpp_Standard/pipeline.example.ini").read_text(encoding="utf-8")
        all_keys = {key for keys in sections().values() for key in keys}
        # Template contains duplicate section headers; every field is commented.
        for section, keys in sections().items():
            for key in keys:
                with self.subTest(section=section, key=key):
                    self.assertRegex(template, r"(?m)^# " + re.escape(key) + r" =$")
        self.assertNotIn("mag_g_column", all_keys)
        self.assertNotIn("mag_i_column", all_keys)
        self.assertIn("lensing.flat_path", path_keys())
        self.assertNotIn("extcat.total_columns", path_keys())
        self.assertIn("total_columns", sections()["extcat"])

    def test_launcher_calls_config_only(self):
        main = (ROOT / "cpp_Standard/main.cpp").read_text(encoding="utf-8")
        self.assertIn("findRuntimeConfigPath(", main)
        self.assertNotIn("parseCommandLine(", main)

    def test_no_lite_mode(self):
        self.assertNotIn("variant", (ROOT / "cpp_docker/fqctl.py").read_text(encoding="utf-8"))


if __name__ == "__main__":
    unittest.main()
