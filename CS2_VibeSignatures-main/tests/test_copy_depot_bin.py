import argparse
import os
import tempfile
import unittest
from unittest.mock import patch

import copy_depot_bin


class TestCopyDepotBin(unittest.TestCase):
    def _write_config(self, root: str, *, include_linux: bool = False) -> str:
        lines = [
            "modules:",
            "  - name: server",
            "    path_windows: game/bin/win64/server.dll",
        ]
        if include_linux:
            lines.append("    path_linux: game/bin/linuxsteamrt64/libserver.so")

        config_path = os.path.join(root, "config.yaml")
        with open(config_path, "w", encoding="utf-8") as handle:
            handle.write("\n".join(lines) + "\n")
        return config_path

    def _make_args(
        self,
        *,
        bindir: str,
        gamever: str,
        platform: str,
        depotdir: str,
        config: str,
        checkonly: bool,
    ) -> argparse.Namespace:
        return argparse.Namespace(
            bindir=bindir,
            gamever=gamever,
            platform=platform,
            depotdir=depotdir,
            config=config,
            checkonly=checkonly,
        )

    def test_main_checkonly_returns_zero_when_all_expected_targets_exist(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            config_path = self._write_config(temp_dir)
            bindir = os.path.join(temp_dir, "bin")
            target_dir = os.path.join(bindir, "14141", "server")
            os.makedirs(target_dir, exist_ok=True)

            with open(os.path.join(target_dir, "server.dll"), "wb") as handle:
                handle.write(b"ok")

            args = self._make_args(
                bindir=bindir,
                gamever="14141",
                platform="windows",
                depotdir=os.path.join(temp_dir, "missing_depot"),
                config=config_path,
                checkonly=True,
            )

            with patch("copy_depot_bin.parse_args", return_value=args):
                self.assertEqual(0, copy_depot_bin.main())

    def test_main_checkonly_returns_one_when_any_expected_target_is_missing(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            config_path = self._write_config(temp_dir, include_linux=True)
            bindir = os.path.join(temp_dir, "bin")
            target_dir = os.path.join(bindir, "14141", "server")
            os.makedirs(target_dir, exist_ok=True)

            with open(os.path.join(target_dir, "server.dll"), "wb") as handle:
                handle.write(b"ok")

            args = self._make_args(
                bindir=bindir,
                gamever="14141",
                platform="all-platform",
                depotdir=os.path.join(temp_dir, "missing_depot"),
                config=config_path,
                checkonly=True,
            )

            with patch("copy_depot_bin.parse_args", return_value=args):
                self.assertEqual(1, copy_depot_bin.main())

    def test_main_checkonly_returns_two_when_config_is_missing(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            bindir = os.path.join(temp_dir, "bin")
            args = self._make_args(
                bindir=bindir,
                gamever="14141",
                platform="all-platform",
                depotdir=os.path.join(temp_dir, "missing_depot"),
                config=os.path.join(temp_dir, "missing.yaml"),
                checkonly=True,
            )

            with patch("copy_depot_bin.parse_args", return_value=args):
                self.assertEqual(2, copy_depot_bin.main())

    def test_main_checkonly_returns_two_when_config_root_is_not_mapping(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            config_path = os.path.join(temp_dir, "config.yaml")
            bindir = os.path.join(temp_dir, "bin")
            with open(config_path, "w", encoding="utf-8") as handle:
                handle.write("- name: server\n")

            args = self._make_args(
                bindir=bindir,
                gamever="14141",
                platform="all-platform",
                depotdir=os.path.join(temp_dir, "missing_depot"),
                config=config_path,
                checkonly=True,
            )

            with patch("copy_depot_bin.parse_args", return_value=args):
                self.assertEqual(2, copy_depot_bin.main())

    def test_main_copy_mode_still_requires_existing_depot_directory(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            config_path = self._write_config(temp_dir)
            bindir = os.path.join(temp_dir, "bin")
            args = self._make_args(
                bindir=bindir,
                gamever="14141",
                platform="windows",
                depotdir=os.path.join(temp_dir, "missing_depot"),
                config=config_path,
                checkonly=False,
            )

            with patch("copy_depot_bin.parse_args", return_value=args):
                self.assertEqual(1, copy_depot_bin.main())


if __name__ == "__main__":
    unittest.main()
