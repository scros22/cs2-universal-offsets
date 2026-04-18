import argparse
import os
import tempfile
import unittest
from unittest.mock import call, patch

import download_depot


class TestDownloadDepot(unittest.TestCase):
    def _write_yaml(self, content: str) -> str:
        with tempfile.NamedTemporaryFile("w", encoding="utf-8", suffix=".yaml", delete=False) as tmp:
            tmp.write(content)
            return tmp.name

    def test_load_downloads_and_find_entry_exact_match(self) -> None:
        config_path = self._write_yaml(
            """
downloads:
  - tag: alpha
    manifests:
      "2347771": "111"
  - tag: beta
    manifests:
      "2347773": "222"
"""
        )
        try:
            downloads = download_depot.load_downloads(config_path)
            self.assertEqual(2, len(downloads))

            entry = download_depot.find_download_entry(downloads, "alpha")
            self.assertEqual("alpha", entry["tag"])
            self.assertEqual("111", entry["manifests"]["2347771"])
        finally:
            os.unlink(config_path)

    def test_find_download_entry_raises_when_tag_not_found(self) -> None:
        config_path = self._write_yaml(
            """
downloads:
  - tag: alpha
    manifests:
      "2347771": "111"
"""
        )
        try:
            downloads = download_depot.load_downloads(config_path)
            with self.assertRaises(download_depot.ConfigError):
                download_depot.find_download_entry(downloads, "missing")
        finally:
            os.unlink(config_path)

    def test_find_download_entry_raises_when_tag_duplicated(self) -> None:
        config_path = self._write_yaml(
            """
downloads:
  - tag: alpha
    manifests:
      "2347771": "111"
  - tag: alpha
    manifests:
      "2347773": "222"
"""
        )
        try:
            downloads = download_depot.load_downloads(config_path)
            with self.assertRaises(download_depot.ConfigError):
                download_depot.find_download_entry(downloads, "alpha")
        finally:
            os.unlink(config_path)

    def test_load_downloads_raises_when_top_level_is_not_mapping(self) -> None:
        config_path = self._write_yaml(
            """
- tag: alpha
  manifests:
    "2347771": "111"
"""
        )
        try:
            with self.assertRaises(download_depot.ConfigError):
                download_depot.load_downloads(config_path)
        finally:
            os.unlink(config_path)

    def test_download_manifests_dispatches_declared_depots_only(self) -> None:
        manifests = {
            "2347771": "111",
            "2347773": "222",
        }

        with patch("download_depot.subprocess.run") as mock_run:
            download_depot.download_manifests(
                manifests=manifests,
                app="730",
                os_name="all-platform",
                depot_dir="cs2_depot",
            )

        self.assertEqual(
            [
                call(
                    [
                        "DepotDownloader",
                        "-app",
                        "730",
                        "-depot",
                        "2347771",
                        "-os",
                        "all-platform",
                        "-dir",
                        "cs2_depot",
                        "-manifest",
                        "111",
                    ],
                    check=True,
                ),
                call(
                    [
                        "DepotDownloader",
                        "-app",
                        "730",
                        "-depot",
                        "2347773",
                        "-os",
                        "all-platform",
                        "-dir",
                        "cs2_depot",
                        "-manifest",
                        "222",
                    ],
                    check=True,
                ),
            ],
            mock_run.call_args_list,
        )

    def test_download_manifests_adds_branch_when_configured(self) -> None:
        manifests = {
            "2347771": "111",
        }

        with patch("download_depot.subprocess.run") as mock_run:
            download_depot.download_manifests(
                manifests=manifests,
                app="730",
                os_name="all-platform",
                depot_dir="cs2_depot",
                branch="animgraph_2_beta",
            )

        self.assertEqual(
            [
                call(
                    [
                        "DepotDownloader",
                        "-app",
                        "730",
                        "-depot",
                        "2347771",
                        "-os",
                        "all-platform",
                        "-dir",
                        "cs2_depot",
                        "-branch",
                        "animgraph_2_beta",
                        "-manifest",
                        "111",
                    ],
                    check=True,
                ),
            ],
            mock_run.call_args_list,
        )

    def test_main_returns_nonzero_when_depotdownloader_missing(self) -> None:
        fake_args = argparse.Namespace(
            tag="alpha",
            config="download.yaml",
            depotdir="cs2_depot",
            app="730",
            os="all-platform",
        )
        fake_entry = {
            "tag": "alpha",
            "manifests": {"2347771": "111"},
        }

        with patch("download_depot.parse_args", return_value=fake_args), \
             patch("download_depot.load_downloads", return_value=[fake_entry]), \
             patch("download_depot.find_download_entry", return_value=fake_entry), \
             patch("download_depot.subprocess.run", side_effect=FileNotFoundError("DepotDownloader")), \
             patch("builtins.print") as mock_print:
            exit_code = download_depot.main()

        self.assertNotEqual(0, exit_code)
        printed = [" ".join(str(part) for part in one_call.args) for one_call in mock_print.call_args_list]
        self.assertTrue(any("DepotDownloader executable not found" in line for line in printed))


if __name__ == "__main__":
    unittest.main()
