import json
import tempfile
import unittest
from pathlib import Path
from unittest.mock import AsyncMock, call, patch

import yaml

import ida_analyze_util


class _FakeTextContent:
    def __init__(self, text: str) -> None:
        self.text = text


class _FakeCallToolResult:
    def __init__(self, payload: dict[str, object]) -> None:
        self.content = [_FakeTextContent(json.dumps(payload))]


def _py_eval_payload(payload: object) -> _FakeCallToolResult:
    return _FakeCallToolResult(
        {
            "result": json.dumps(payload),
            "stdout": "",
            "stderr": "",
        }
    )


def _write_yaml(path: Path, payload: dict[str, object]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(yaml.safe_dump(payload, sort_keys=False), encoding="utf-8")


class TestPreprocessIndexBasedVfuncViaMcp(unittest.IsolatedAsyncioTestCase):
    async def test_reads_sibling_module_yaml_and_derives_index_from_offset(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            gamever_dir = Path(temp_dir) / "bin" / "14141"
            current_module_dir = gamever_dir / "schemasystem"
            sibling_module_dir = gamever_dir / "server"
            target_output = current_module_dir / "CDerived_CreateFieldChangedEventQueue.windows.yaml"

            _write_yaml(
                sibling_module_dir / "CFlattenedSerializers_CreateFieldChangedEventQueue.windows.yaml",
                {
                    "vtable_name": "CFlattenedSerializers",
                    "vfunc_offset": "0x118",
                },
            )
            _write_yaml(
                current_module_dir / "CDerived_vtable.windows.yaml",
                {
                    "vtable_entries": {
                        35: "0x180001180",
                    }
                },
            )

            session = AsyncMock()
            session.call_tool.return_value = _py_eval_payload(
                {
                    "func_va": "0x180001180",
                    "func_size": "0x40",
                }
            )

            result = await ida_analyze_util.preprocess_index_based_vfunc_via_mcp(
                session=session,
                target_func_name="CDerived_CreateFieldChangedEventQueue",
                target_output=str(target_output),
                old_yaml_map={},
                new_binary_dir=str(current_module_dir),
                platform="windows",
                image_base=0x180000000,
                base_vfunc_name="../server/CFlattenedSerializers_CreateFieldChangedEventQueue",
                inherit_vtable_class="CDerived",
                generate_func_sig=False,
                debug=False,
            )

            self.assertIsNotNone(result)
            assert result is not None
            self.assertEqual(35, result["vfunc_index"])
            self.assertEqual("0x118", result["vfunc_offset"])
            self.assertEqual("CDerived_CreateFieldChangedEventQueue", result["func_name"])
            self.assertEqual("0x1180", result["func_rva"])
            session.call_tool.assert_awaited_once()

    async def test_returns_none_for_misaligned_vfunc_offset(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            module_dir = Path(temp_dir) / "bin" / "14141" / "server"

            _write_yaml(
                module_dir / "CBaseEntity_Touch.windows.yaml",
                {
                    "vtable_name": "CBaseEntity",
                    "vfunc_offset": "0x11a",
                },
            )
            _write_yaml(
                module_dir / "CDerived_vtable.windows.yaml",
                {
                    "vtable_entries": {
                        35: "0x180001180",
                    }
                },
            )

            session = AsyncMock()

            result = await ida_analyze_util.preprocess_index_based_vfunc_via_mcp(
                session=session,
                target_func_name="CDerived_Touch",
                target_output=str(module_dir / "CDerived_Touch.windows.yaml"),
                old_yaml_map={},
                new_binary_dir=str(module_dir),
                platform="windows",
                image_base=0x180000000,
                base_vfunc_name="CBaseEntity_Touch",
                inherit_vtable_class="CDerived",
                generate_func_sig=False,
                debug=False,
            )

            self.assertIsNone(result)
            session.call_tool.assert_not_awaited()

    async def test_returns_none_for_mismatched_vfunc_index_and_offset(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            module_dir = Path(temp_dir) / "bin" / "14141" / "server"

            _write_yaml(
                module_dir / "CBaseEntity_Touch.windows.yaml",
                {
                    "vtable_name": "CBaseEntity",
                    "vfunc_index": 34,
                    "vfunc_offset": "0x118",
                },
            )
            _write_yaml(
                module_dir / "CDerived_vtable.windows.yaml",
                {
                    "vtable_entries": {
                        35: "0x180001180",
                    }
                },
            )

            session = AsyncMock()

            result = await ida_analyze_util.preprocess_index_based_vfunc_via_mcp(
                session=session,
                target_func_name="CDerived_Touch",
                target_output=str(module_dir / "CDerived_Touch.windows.yaml"),
                old_yaml_map={},
                new_binary_dir=str(module_dir),
                platform="windows",
                image_base=0x180000000,
                base_vfunc_name="CBaseEntity_Touch",
                inherit_vtable_class="CDerived",
                generate_func_sig=False,
                debug=False,
            )

            self.assertIsNone(result)
            session.call_tool.assert_not_awaited()

    async def test_returns_none_for_base_vfunc_path_outside_gamever_root(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            module_dir = Path(temp_dir) / "bin" / "14141" / "server"
            session = AsyncMock()

            result = await ida_analyze_util.preprocess_index_based_vfunc_via_mcp(
                session=session,
                target_func_name="CDerived_Touch",
                target_output=str(module_dir / "CDerived_Touch.windows.yaml"),
                old_yaml_map={},
                new_binary_dir=str(module_dir),
                platform="windows",
                image_base=0x180000000,
                base_vfunc_name="../../outside/CBaseEntity_Touch",
                inherit_vtable_class="CDerived",
                generate_func_sig=False,
                debug=False,
            )

            self.assertIsNone(result)
            session.call_tool.assert_not_awaited()


class TestVtableAliasSupport(unittest.IsolatedAsyncioTestCase):
    async def test_preprocess_common_skill_rejects_invalid_mangled_class_names(self) -> None:
        result = await ida_analyze_util.preprocess_common_skill(
            session="session",
            expected_outputs=["/tmp/Foo_vtable.windows.yaml"],
            vtable_class_names=["Foo"],
            mangled_class_names=["bad-config"],
            platform="windows",
            image_base=0x180000000,
            generate_yaml_desired_fields=[("Foo", ["vtable_class"])],
            debug=False,
        )

        self.assertFalse(result)

    def test_build_vtable_py_eval_embeds_candidate_symbols(self) -> None:
        py_code = ida_analyze_util._build_vtable_py_eval(
            "CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem",
            [
                "??_7?$CGameSystemReallocatingFactory@VCSpawnGroupMgrGameSystem@@V1@@@6B@",
                "_ZTV30CGameSystemReallocatingFactoryI24CSpawnGroupMgrGameSystemS0_E",
            ],
        )

        self.assertIn(
            '"CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem"',
            py_code,
        )
        self.assertIn("candidate_symbols = [", py_code)
        self.assertIn(
            "??_7?$CGameSystemReallocatingFactory@VCSpawnGroupMgrGameSystem@@V1@@@6B@",
            py_code,
        )
        self.assertIn(
            "_ZTV30CGameSystemReallocatingFactoryI24CSpawnGroupMgrGameSystemS0_E",
            py_code,
        )

    async def test_preprocess_common_skill_passes_aliases_to_vtable_lookup(self) -> None:
        alias_map = {
            "CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem": [
                "??_7?$CGameSystemReallocatingFactory@VCSpawnGroupMgrGameSystem@@V1@@@6B@",
                "_ZTV30CGameSystemReallocatingFactoryI24CSpawnGroupMgrGameSystemS0_E",
            ]
        }
        fake_vtable_data = {
            "vtable_class": "CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem",
            "vtable_symbol": "??_7?$CGameSystemReallocatingFactory@VCSpawnGroupMgrGameSystem@@V1@@@6B@",
            "vtable_va": "0x180010000",
            "vtable_rva": "0x10000",
            "vtable_size": "0x20",
            "vtable_numvfunc": 4,
            "vtable_entries": {0: "0x180020000"},
        }

        with patch.object(
            ida_analyze_util,
            "preprocess_vtable_via_mcp",
            AsyncMock(return_value=fake_vtable_data),
        ) as mock_preprocess_vtable, patch.object(
            ida_analyze_util,
            "write_vtable_yaml",
        ) as mock_write_vtable_yaml:
            result = await ida_analyze_util.preprocess_common_skill(
                session="session",
                expected_outputs=[
                    "/tmp/CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem_vtable.windows.yaml"
                ],
                vtable_class_names=[
                    "CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem"
                ],
                mangled_class_names=alias_map,
                platform="windows",
                image_base=0x180000000,
                generate_yaml_desired_fields=[
                    (
                        "CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem",
                        [
                            "vtable_class",
                            "vtable_symbol",
                            "vtable_va",
                            "vtable_rva",
                            "vtable_size",
                            "vtable_numvfunc",
                            "vtable_entries",
                        ],
                    )
                ],
                debug=True,
            )

        self.assertTrue(result)
        mock_preprocess_vtable.assert_awaited_once_with(
            session="session",
            class_name="CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem",
            image_base=0x180000000,
            platform="windows",
            debug=True,
            symbol_aliases=alias_map[
                "CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem"
            ],
        )
        mock_write_vtable_yaml.assert_called_once()

    async def test_preprocess_func_sig_uses_aliases_when_generating_missing_vtable_yaml(self) -> None:
        alias_map = {
            "CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem": [
                "??_7?$CGameSystemReallocatingFactory@VCSpawnGroupMgrGameSystem@@V1@@@6B@",
                "_ZTV30CGameSystemReallocatingFactoryI24CSpawnGroupMgrGameSystemS0_E",
            ]
        }

        with tempfile.TemporaryDirectory() as temp_dir:
            module_dir = Path(temp_dir) / "bin" / "14141" / "server"
            old_dir = Path(temp_dir) / "old"
            new_path = module_dir / "CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem_Think.windows.yaml"
            old_path = old_dir / "CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem_Think.windows.yaml"

            old_dir.mkdir(parents=True, exist_ok=True)
            module_dir.mkdir(parents=True, exist_ok=True)
            old_path.write_text(
                yaml.safe_dump(
                    {
                        "vtable_name": "CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem",
                        "vfunc_sig": "AA BB CC DD",
                        "vfunc_index": 1,
                        "func_va": "0x180001111",
                    },
                    sort_keys=False,
                ),
                encoding="utf-8",
            )

            session = AsyncMock()
            session.call_tool.side_effect = [
                _FakeCallToolResult([{"matches": ["0x180003000"], "n": 1}]),
                _py_eval_payload(
                    {
                        "func_va": "0x180004000",
                        "func_size": "0x40",
                    }
                ),
            ]

            with patch.object(
                ida_analyze_util,
                "preprocess_vtable_via_mcp",
                AsyncMock(
                    return_value={
                        "vtable_class": "CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem",
                        "vtable_symbol": "_ZTV30CGameSystemReallocatingFactoryI24CSpawnGroupMgrGameSystemS0_E + 0x10",
                        "vtable_va": "0x180001000",
                        "vtable_rva": "0x1000",
                        "vtable_size": "0x10",
                        "vtable_numvfunc": 2,
                        "vtable_entries": {
                            0: "0x180002000",
                            1: "0x180004000",
                        },
                    }
                ),
            ) as mock_preprocess_vtable:
                result = await ida_analyze_util.preprocess_func_sig_via_mcp(
                    session=session,
                    new_path=str(new_path),
                    old_path=str(old_path),
                    image_base=0x180000000,
                    new_binary_dir=str(module_dir),
                    platform="windows",
                    func_name="CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem_Think",
                    debug=True,
                    mangled_class_names=alias_map,
                )

        self.assertIsNotNone(result)
        mock_preprocess_vtable.assert_awaited_once_with(
            session,
            "CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem",
            0x180000000,
            "windows",
            True,
            alias_map[
                "CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem"
            ],
        )

    async def test_func_vtable_relations_use_aliases_for_index_enrichment(self) -> None:
        alias_map = {
            "CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem": [
                "??_7?$CGameSystemReallocatingFactory@VCSpawnGroupMgrGameSystem@@V1@@@6B@",
                "_ZTV30CGameSystemReallocatingFactoryI24CSpawnGroupMgrGameSystemS0_E",
            ]
        }

        with patch.object(
            ida_analyze_util,
            "preprocess_func_sig_via_mcp",
            AsyncMock(
                return_value={
                    "func_name": "CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem_Think",
                    "func_va": "0x180004000",
                    "func_rva": "0x4000",
                    "func_size": "0x40",
                    "func_sig": "AA BB",
                }
            ),
        ) as mock_preprocess_func, patch.object(
            ida_analyze_util,
            "preprocess_vtable_via_mcp",
            AsyncMock(
                return_value={
                    "vtable_class": "CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem",
                    "vtable_symbol": "??_7?$CGameSystemReallocatingFactory@VCSpawnGroupMgrGameSystem@@V1@@@6B@",
                    "vtable_va": "0x180001000",
                    "vtable_rva": "0x1000",
                    "vtable_size": "0x20",
                    "vtable_numvfunc": 4,
                    "vtable_entries": {
                        0: "0x180003000",
                        1: "0x180004000",
                    },
                }
            ),
        ) as mock_preprocess_vtable, patch.object(
            ida_analyze_util,
            "write_func_yaml",
        ) as mock_write_func_yaml, patch.object(
            ida_analyze_util,
            "_rename_func_in_ida",
            AsyncMock(return_value=None),
        ):
            result = await ida_analyze_util.preprocess_common_skill(
                session="session",
                expected_outputs=[
                    "/tmp/CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem_Think.windows.yaml"
                ],
                old_yaml_map={},
                new_binary_dir="/tmp",
                platform="windows",
                image_base=0x180000000,
                func_names=[
                    "CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem_Think"
                ],
                func_vtable_relations=[
                    (
                        "CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem_Think",
                        "CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem",
                    )
                ],
                generate_yaml_desired_fields=[
                    (
                        "CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem_Think",
                        ["func_name", "vtable_name", "vfunc_offset", "vfunc_index"],
                    )
                ],
                mangled_class_names=alias_map,
                debug=True,
            )

        self.assertTrue(result)
        mock_preprocess_func.assert_awaited_once()
        mock_preprocess_vtable.assert_awaited_once_with(
            session="session",
            class_name="CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem",
            image_base=0x180000000,
            platform="windows",
            debug=True,
            symbol_aliases=alias_map[
                "CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem"
            ],
        )
        written_payload = mock_write_func_yaml.call_args.args[1]
        self.assertEqual(1, written_payload["vfunc_index"])
        self.assertEqual("0x8", written_payload["vfunc_offset"])


class TestGenerateYamlDesiredFieldsContract(unittest.IsolatedAsyncioTestCase):
    async def test_preprocess_common_skill_rejects_missing_generate_yaml_desired_fields(
        self,
    ) -> None:
        with patch.object(
            ida_analyze_util,
            "preprocess_func_sig_via_mcp",
            AsyncMock(
                return_value={
                    "func_name": "Foo",
                    "func_va": "0x180004000",
                    "func_rva": "0x4000",
                    "func_size": "0x40",
                    "func_sig": "AA BB",
                }
            ),
        ) as mock_preprocess_func_sig, patch.object(
            ida_analyze_util,
            "write_func_yaml",
        ) as mock_write_func_yaml, patch.object(
            ida_analyze_util,
            "_rename_func_in_ida",
            AsyncMock(return_value=None),
        ):
            result = await ida_analyze_util.preprocess_common_skill(
                session="session",
                expected_outputs=["/tmp/Foo.windows.yaml"],
                old_yaml_map={},
                new_binary_dir="/tmp",
                platform="windows",
                image_base=0x180000000,
                func_names=["Foo"],
                generate_yaml_desired_fields=None,
                debug=True,
            )

        self.assertFalse(result)
        mock_preprocess_func_sig.assert_not_awaited()
        mock_write_func_yaml.assert_not_called()

    async def test_normalize_generate_yaml_desired_fields_parses_vfunc_sig_max_match_directive(
        self,
    ) -> None:
        result = ida_analyze_util._normalize_generate_yaml_desired_fields(
            [
                (
                    "Foo",
                    [
                        "func_name",
                        "vfunc_sig",
                        "vfunc_sig_max_match:10",
                    ],
                )
            ],
            debug=True,
        )

        self.assertEqual(
            {
                "Foo": {
                    "desired_output_fields": [
                        "func_name",
                        "vfunc_sig",
                        "vfunc_sig_max_match",
                    ],
                    "generation_options": {
                        "vfunc_sig_max_match": 10,
                    },
                }
            },
            result,
        )

    async def test_normalize_generate_yaml_desired_fields_rejects_vfunc_sig_max_match_without_vfunc_sig(
        self,
    ) -> None:
        result = ida_analyze_util._normalize_generate_yaml_desired_fields(
            [
                (
                    "Foo",
                    [
                        "func_name",
                        "vfunc_sig_max_match:10",
                    ],
                )
            ],
            debug=True,
        )

        self.assertIsNone(result)

    async def test_normalize_generate_yaml_desired_fields_rejects_invalid_vfunc_sig_max_match_value(
        self,
    ) -> None:
        result = ida_analyze_util._normalize_generate_yaml_desired_fields(
            [
                (
                    "Foo",
                    [
                        "func_name",
                        "vfunc_sig",
                        "vfunc_sig_max_match:abc",
                    ],
                )
            ],
            debug=True,
        )

        self.assertIsNone(result)

    async def test_normalize_generate_yaml_desired_fields_rejects_bare_vfunc_sig_max_match_field(
        self,
    ) -> None:
        result = ida_analyze_util._normalize_generate_yaml_desired_fields(
            [
                (
                    "Foo",
                    [
                        "func_name",
                        "vfunc_sig",
                        "vfunc_sig_max_match",
                    ],
                )
            ],
            debug=True,
        )

        self.assertIsNone(result)

    async def test_normalize_generate_yaml_desired_fields_rejects_duplicate_vfunc_sig_max_match_directive(
        self,
    ) -> None:
        result = ida_analyze_util._normalize_generate_yaml_desired_fields(
            [
                (
                    "Foo",
                    [
                        "func_name",
                        "vfunc_sig",
                        "vfunc_sig_max_match:10",
                        "vfunc_sig_max_match:12",
                    ],
                )
            ],
            debug=True,
        )

        self.assertIsNone(result)

    async def test_normalize_generate_yaml_desired_fields_rejects_zero_vfunc_sig_max_match_value(
        self,
    ) -> None:
        result = ida_analyze_util._normalize_generate_yaml_desired_fields(
            [
                (
                    "Foo",
                    [
                        "func_name",
                        "vfunc_sig",
                        "vfunc_sig_max_match:0",
                    ],
                )
            ],
            debug=True,
        )

        self.assertIsNone(result)

    async def test_normalize_generate_yaml_desired_fields_rejects_negative_vfunc_sig_max_match_value(
        self,
    ) -> None:
        result = ida_analyze_util._normalize_generate_yaml_desired_fields(
            [
                (
                    "Foo",
                    [
                        "func_name",
                        "vfunc_sig",
                        "vfunc_sig_max_match:-1",
                    ],
                )
            ],
            debug=True,
        )

        self.assertIsNone(result)

    async def test_preprocess_common_skill_rejects_missing_desired_fields_before_any_write(
        self,
    ) -> None:
        with patch.object(
            ida_analyze_util,
            "preprocess_func_sig_via_mcp",
            AsyncMock(
                return_value={
                    "func_name": "Foo",
                    "func_va": "0x180004000",
                    "func_rva": "0x4000",
                    "func_size": "0x40",
                    "func_sig": "AA BB",
                }
            ),
        ) as mock_preprocess_func_sig, patch.object(
            ida_analyze_util,
            "preprocess_vtable_via_mcp",
            AsyncMock(
                return_value={
                    "vtable_class": "Bar",
                    "vtable_symbol": "??_7Bar@@6B@",
                    "vtable_va": "0x180001000",
                    "vtable_rva": "0x1000",
                    "vtable_size": "0x20",
                    "vtable_numvfunc": 2,
                    "vtable_entries": {0: "0x180003000", 1: "0x180004000"},
                }
            ),
        ) as mock_preprocess_vtable, patch.object(
            ida_analyze_util,
            "write_func_yaml",
        ) as mock_write_func_yaml, patch.object(
            ida_analyze_util,
            "write_vtable_yaml",
        ) as mock_write_vtable_yaml:
            result = await ida_analyze_util.preprocess_common_skill(
                session="session",
                expected_outputs=[
                    "/tmp/Foo.windows.yaml",
                    "/tmp/Bar_vtable.windows.yaml",
                ],
                old_yaml_map={},
                new_binary_dir="/tmp",
                platform="windows",
                image_base=0x180000000,
                func_names=["Foo"],
                vtable_class_names=["Bar"],
                generate_yaml_desired_fields=[("Foo", ["func_name"])],
                debug=True,
            )

        self.assertFalse(result)
        mock_preprocess_func_sig.assert_not_awaited()
        mock_preprocess_vtable.assert_not_awaited()
        mock_write_func_yaml.assert_not_called()
        mock_write_vtable_yaml.assert_not_called()

    async def test_preprocess_common_skill_filters_func_payload_by_desired_fields(
        self,
    ) -> None:
        with patch.object(
            ida_analyze_util,
            "preprocess_func_sig_via_mcp",
            AsyncMock(
                return_value={
                    "func_name": "Foo",
                    "func_va": "0x180004000",
                    "func_rva": "0x4000",
                    "func_size": "0x40",
                    "func_sig": "AA BB",
                }
            ),
        ), patch.object(
            ida_analyze_util,
            "preprocess_vtable_via_mcp",
            AsyncMock(
                return_value={
                    "vtable_class": "Bar",
                    "vtable_symbol": "??_7Bar@@6B@",
                    "vtable_va": "0x180001000",
                    "vtable_rva": "0x1000",
                    "vtable_size": "0x20",
                    "vtable_numvfunc": 2,
                    "vtable_entries": {
                        0: "0x180003000",
                        1: "0x180004000",
                    },
                }
            ),
        ) as mock_preprocess_vtable, patch.object(
            ida_analyze_util,
            "write_func_yaml",
        ) as mock_write_func_yaml, patch.object(
            ida_analyze_util,
            "_rename_func_in_ida",
            AsyncMock(return_value=None),
        ):
            result = await ida_analyze_util.preprocess_common_skill(
                session="session",
                expected_outputs=["/tmp/Foo.windows.yaml"],
                old_yaml_map={},
                new_binary_dir="/tmp",
                platform="windows",
                image_base=0x180000000,
                func_names=["Foo"],
                func_vtable_relations=[("Foo", "Bar")],
                generate_yaml_desired_fields=[
                    ("Foo", ["func_name", "vtable_name", "vfunc_offset", "vfunc_index"])
                ],
                debug=True,
            )

        self.assertTrue(result)
        mock_preprocess_vtable.assert_awaited_once_with(
            session="session",
            class_name="Bar",
            image_base=0x180000000,
            platform="windows",
            debug=True,
            symbol_aliases=None,
        )
        mock_write_func_yaml.assert_called_once()
        self.assertEqual(
            {
                "func_name": "Foo",
                "vtable_name": "Bar",
                "vfunc_offset": "0x8",
                "vfunc_index": 1,
            },
            mock_write_func_yaml.call_args.args[1],
        )

    async def test_preprocess_common_skill_writes_vfunc_sig_max_match_field(
        self,
    ) -> None:
        with patch.object(
            ida_analyze_util,
            "preprocess_func_sig_via_mcp",
            AsyncMock(
                return_value={
                    "func_name": "Foo",
                    "func_va": "0x180004000",
                    "func_sig": "AA BB",
                    "vfunc_sig": "48 89 5C 24 ? ? 57",
                    "vfunc_sig_max_match": 10,
                    "vtable_name": "Bar",
                }
            ),
        ), patch.object(
            ida_analyze_util,
            "write_func_yaml",
        ) as mock_write_func_yaml, patch.object(
            ida_analyze_util,
            "_rename_func_in_ida",
            AsyncMock(return_value=None),
        ):
            result = await ida_analyze_util.preprocess_common_skill(
                session="session",
                expected_outputs=["/tmp/Foo.windows.yaml"],
                old_yaml_map={},
                new_binary_dir="/tmp",
                platform="windows",
                image_base=0x180000000,
                func_names=["Foo"],
                generate_yaml_desired_fields=[
                    (
                        "Foo",
                        [
                            "vfunc_sig_max_match:10",
                            "func_name",
                            "vfunc_sig",
                        ],
                    )
                ],
                debug=True,
            )

        self.assertTrue(result)
        mock_write_func_yaml.assert_called_once()
        written_payload = mock_write_func_yaml.call_args.args[1]
        self.assertEqual(
            ["func_name", "vfunc_sig", "vfunc_sig_max_match"],
            list(written_payload.keys()),
        )
        self.assertEqual(
            {
                "func_name": "Foo",
                "vfunc_sig": "48 89 5C 24 ? ? 57",
                "vfunc_sig_max_match": 10,
            },
            written_payload,
        )

    async def test_preprocess_common_skill_rejects_missing_requested_func_field(
        self,
    ) -> None:
        with patch.object(
            ida_analyze_util,
            "preprocess_func_sig_via_mcp",
            AsyncMock(
                return_value={
                    "func_name": "Foo",
                    "func_va": "0x180004000",
                    "func_sig": "AA BB",
                }
            ),
        ), patch.object(
            ida_analyze_util,
            "write_func_yaml",
        ) as mock_write_func_yaml, patch.object(
            ida_analyze_util,
            "_rename_func_in_ida",
            AsyncMock(return_value=None),
        ) as mock_rename_func:
            result = await ida_analyze_util.preprocess_common_skill(
                session="session",
                expected_outputs=["/tmp/Foo.windows.yaml"],
                old_yaml_map={},
                new_binary_dir="/tmp",
                platform="windows",
                image_base=0x180000000,
                func_names=["Foo"],
                generate_yaml_desired_fields=[("Foo", ["func_name", "func_va", "func_rva"])],
                debug=True,
            )

        self.assertFalse(result)
        mock_write_func_yaml.assert_not_called()
        mock_rename_func.assert_not_awaited()

    async def test_preprocess_common_skill_does_not_rename_when_write_fails(
        self,
    ) -> None:
        with patch.object(
            ida_analyze_util,
            "preprocess_func_sig_via_mcp",
            AsyncMock(
                return_value={
                    "func_name": "Foo",
                    "func_va": "0x180004000",
                    "func_sig": "AA BB",
                }
            ),
        ), patch.object(
            ida_analyze_util,
            "write_func_yaml",
            side_effect=OSError("boom"),
        ), patch.object(
            ida_analyze_util,
            "_rename_func_in_ida",
            AsyncMock(return_value=None),
        ) as mock_rename_func:
            with self.assertRaises(OSError):
                await ida_analyze_util.preprocess_common_skill(
                    session="session",
                    expected_outputs=["/tmp/Foo.windows.yaml"],
                    old_yaml_map={},
                    new_binary_dir="/tmp",
                    platform="windows",
                    image_base=0x180000000,
                    func_names=["Foo"],
                    generate_yaml_desired_fields=[("Foo", ["func_name"])],
                    debug=True,
                )

        mock_rename_func.assert_not_awaited()

    async def test_preprocess_common_skill_defers_rename_until_all_targets_succeed(
        self,
    ) -> None:
        async def _preprocess_func_side_effect(**kwargs):
            func_name = kwargs["func_name"]
            if func_name == "Foo":
                return {
                    "func_name": "Foo",
                    "func_va": "0x180004000",
                }
            if func_name == "Bar":
                return {
                    "func_name": "Bar",
                    "func_va": "0x180005000",
                }
            return None

        with patch.object(
            ida_analyze_util,
            "preprocess_func_sig_via_mcp",
            AsyncMock(side_effect=_preprocess_func_side_effect),
        ), patch.object(
            ida_analyze_util,
            "write_func_yaml",
        ) as mock_write_func_yaml, patch.object(
            ida_analyze_util,
            "_rename_func_in_ida",
            AsyncMock(return_value=None),
        ) as mock_rename_func:
            result = await ida_analyze_util.preprocess_common_skill(
                session="session",
                expected_outputs=[
                    "/tmp/Foo.windows.yaml",
                    "/tmp/Bar.windows.yaml",
                ],
                old_yaml_map={},
                new_binary_dir="/tmp",
                platform="windows",
                image_base=0x180000000,
                func_names=["Foo", "Bar"],
                generate_yaml_desired_fields=[
                    ("Foo", ["func_name"]),
                    ("Bar", ["func_name", "func_rva"]),
                ],
                debug=True,
            )

        self.assertFalse(result)
        mock_write_func_yaml.assert_called_once_with(
            "/tmp/Foo.windows.yaml",
            {"func_name": "Foo"},
        )
        mock_rename_func.assert_not_awaited()

    async def test_preprocess_common_skill_renames_after_all_writes_succeed(
        self,
    ) -> None:
        events = []

        async def _rename_side_effect(_session, _func_va_hex, func_name, _debug):
            events.append(("rename", func_name))

        with patch.object(
            ida_analyze_util,
            "preprocess_func_sig_via_mcp",
            AsyncMock(
                side_effect=[
                    {"func_name": "Foo", "func_va": "0x180001000", "func_sig": "AA"},
                    {"func_name": "Bar", "func_va": "0x180002000", "func_sig": "BB"},
                ]
            ),
        ), patch.object(
            ida_analyze_util,
            "write_func_yaml",
        ) as mock_write_func_yaml, patch.object(
            ida_analyze_util,
            "_rename_func_in_ida",
            AsyncMock(side_effect=_rename_side_effect),
        ):
            mock_write_func_yaml.side_effect = (
                lambda path, _data: events.append(("write", path))
            )
            result = await ida_analyze_util.preprocess_common_skill(
                session="session",
                expected_outputs=[
                    "/tmp/Foo.windows.yaml",
                    "/tmp/Bar.windows.yaml",
                ],
                old_yaml_map={},
                new_binary_dir="/tmp",
                platform="windows",
                image_base=0x180000000,
                func_names=["Foo", "Bar"],
                generate_yaml_desired_fields=[
                    ("Foo", ["func_name"]),
                    ("Bar", ["func_name"]),
                ],
                debug=True,
            )

        self.assertTrue(result)
        self.assertEqual(
            [
                ("write", "/tmp/Foo.windows.yaml"),
                ("write", "/tmp/Bar.windows.yaml"),
                ("rename", "Foo"),
                ("rename", "Bar"),
            ],
            events,
        )

    async def test_preprocess_common_skill_filters_vtable_payload_by_desired_fields(
        self,
    ) -> None:
        with patch.object(
            ida_analyze_util,
            "preprocess_vtable_via_mcp",
            AsyncMock(
                return_value={
                    "vtable_class": "Foo",
                    "vtable_symbol": "??_7Foo@@6B@",
                    "vtable_va": "0x180001000",
                    "vtable_rva": "0x1000",
                    "vtable_size": "0x20",
                    "vtable_numvfunc": 4,
                    "vtable_entries": {0: "0x180010000"},
                }
            ),
        ), patch.object(
            ida_analyze_util,
            "write_vtable_yaml",
        ) as mock_write_vtable_yaml:
            result = await ida_analyze_util.preprocess_common_skill(
                session="session",
                expected_outputs=["/tmp/Foo_vtable.windows.yaml"],
                vtable_class_names=["Foo"],
                platform="windows",
                image_base=0x180000000,
                generate_yaml_desired_fields=[
                    ("Foo", ["vtable_class", "vtable_entries"])
                ],
                debug=True,
            )

        self.assertTrue(result)
        mock_write_vtable_yaml.assert_called_once_with(
            "/tmp/Foo_vtable.windows.yaml",
            {
                "vtable_class": "Foo",
                "vtable_entries": {0: "0x180010000"},
            },
        )


class TestFuncXrefsSignatureSupport(unittest.IsolatedAsyncioTestCase):
    async def test_collect_xref_func_starts_for_string_uses_substring_by_default(
        self,
    ) -> None:
        session = AsyncMock()
        session.call_tool.return_value = _py_eval_payload(["0x180001000"])

        result = await ida_analyze_util._collect_xref_func_starts_for_string(
            session=session,
            xref_string="_projectile",
            debug=True,
        )

        self.assertEqual({0x180001000}, result)
        session.call_tool.assert_awaited_once()
        py_code = session.call_tool.await_args.kwargs["arguments"]["code"]
        self.assertIn('search_str = "_projectile"', py_code)
        self.assertIn("if search_str in current_str:", py_code)
        self.assertNotIn("if current_str == search_str:", py_code)

    async def test_collect_xref_func_starts_for_string_supports_fullmatch_prefix(
        self,
    ) -> None:
        session = AsyncMock()
        session.call_tool.return_value = _py_eval_payload(["0x180001000"])

        result = await ida_analyze_util._collect_xref_func_starts_for_string(
            session=session,
            xref_string="FULLMATCH:_projectile",
            debug=True,
        )

        self.assertEqual({0x180001000}, result)
        session.call_tool.assert_awaited_once()
        py_code = session.call_tool.await_args.kwargs["arguments"]["code"]
        self.assertIn('search_str = "_projectile"', py_code)
        self.assertIn("if current_str == search_str:", py_code)
        self.assertNotIn("if search_str in current_str:", py_code)
        self.assertNotIn("FULLMATCH:_projectile", py_code)

    async def test_preprocess_func_xrefs_intersects_string_and_signature_sets(
        self,
    ) -> None:
        with patch.object(
            ida_analyze_util,
            "_collect_xref_func_starts_for_string",
            AsyncMock(return_value={0x180100000, 0x180200000}),
        ) as mock_collect_string, patch.object(
            ida_analyze_util,
            "_collect_xref_func_starts_for_signature",
            AsyncMock(return_value={0x180200000}),
        ) as mock_collect_signature, patch.object(
            ida_analyze_util,
            "preprocess_gen_func_sig_via_mcp",
            AsyncMock(
                return_value={
                    "func_va": "0x180200000",
                    "func_rva": "0x200000",
                    "func_size": "0x40",
                    "func_sig": "48 89 5C 24 08",
                }
            ),
        ) as mock_gen_sig:
            result = await ida_analyze_util.preprocess_func_xrefs_via_mcp(
                session="session",
                func_name="LoggingChannel_Init",
                xref_strings=["Networking"],
                xref_signatures=["C7 44 24 40 64 FF FF FF"],
                xref_funcs=[],
                exclude_funcs=[],
                exclude_strings=[],
                new_binary_dir="bin_dir",
                platform="windows",
                image_base=0x180000000,
                debug=True,
            )

        self.assertEqual(
            {
                "func_name": "LoggingChannel_Init",
                "func_va": "0x180200000",
                "func_rva": "0x200000",
                "func_size": "0x40",
                "func_sig": "48 89 5C 24 08",
            },
            result,
        )
        mock_collect_string.assert_awaited_once_with(
            session="session",
            xref_string="Networking",
            debug=True,
        )
        mock_collect_signature.assert_awaited_once_with(
            session="session",
            xref_signature="C7 44 24 40 64 FF FF FF",
            debug=True,
        )
        mock_gen_sig.assert_awaited_once()

    async def test_preprocess_func_xrefs_fails_when_signature_set_is_empty(
        self,
    ) -> None:
        with patch.object(
            ida_analyze_util,
            "_collect_xref_func_starts_for_string",
            AsyncMock(return_value={0x180100000}),
        ), patch.object(
            ida_analyze_util,
            "_collect_xref_func_starts_for_signature",
            AsyncMock(return_value=set()),
        ), patch.object(
            ida_analyze_util,
            "preprocess_gen_func_sig_via_mcp",
            AsyncMock(return_value=None),
        ) as mock_gen_sig:
            result = await ida_analyze_util.preprocess_func_xrefs_via_mcp(
                session="session",
                func_name="LoggingChannel_Init",
                xref_strings=["Networking"],
                xref_signatures=["C7 44 24 40 64 FF FF FF"],
                xref_funcs=[],
                exclude_funcs=[],
                exclude_strings=[],
                new_binary_dir="bin_dir",
                platform="windows",
                image_base=0x180000000,
                debug=True,
            )

        self.assertIsNone(result)
        mock_gen_sig.assert_not_called()

    async def test_preprocess_common_skill_forwards_xref_signatures(
        self,
    ) -> None:
        with patch.object(
            ida_analyze_util,
            "preprocess_func_sig_via_mcp",
            AsyncMock(return_value=None),
        ), patch.object(
            ida_analyze_util,
            "preprocess_func_xrefs_via_mcp",
            AsyncMock(
                return_value={
                    "func_name": "LoggingChannel_Init",
                    "func_va": "0x180200000",
                    "func_rva": "0x200000",
                    "func_size": "0x40",
                    "func_sig": "48 89 5C 24 08",
                }
            ),
        ) as mock_func_xrefs, patch.object(
            ida_analyze_util,
            "write_func_yaml",
        ) as mock_write_func_yaml, patch.object(
            ida_analyze_util,
            "_rename_func_in_ida",
            AsyncMock(return_value=None),
        ):
            result = await ida_analyze_util.preprocess_common_skill(
                session="session",
                expected_outputs=["/tmp/LoggingChannel_Init.windows.yaml"],
                old_yaml_map={},
                new_binary_dir="/tmp",
                platform="windows",
                image_base=0x180000000,
                func_names=["LoggingChannel_Init"],
                func_xrefs=[
                    (
                        "LoggingChannel_Init",
                        ["Networking"],
                        ["C7 44 24 40 64 FF FF FF"],
                        [],
                        [],
                        [],
                    )
                ],
                generate_yaml_desired_fields=[
                    (
                        "LoggingChannel_Init",
                        [
                            "func_name",
                            "func_va",
                            "func_rva",
                            "func_size",
                            "func_sig",
                        ],
                    )
                ],
                debug=True,
            )

        self.assertTrue(result)
        mock_func_xrefs.assert_awaited_once()
        self.assertEqual(
            ["C7 44 24 40 64 FF FF FF"],
            mock_func_xrefs.call_args.kwargs["xref_signatures"],
        )
        mock_write_func_yaml.assert_called_once()

    async def test_preprocess_common_skill_rejects_legacy_five_item_func_xrefs(
        self,
    ) -> None:
        result = await ida_analyze_util.preprocess_common_skill(
            session="session",
            expected_outputs=["/tmp/LoggingChannel_Init.windows.yaml"],
            old_yaml_map={},
            new_binary_dir="/tmp",
            platform="windows",
            image_base=0x180000000,
            func_names=["LoggingChannel_Init"],
            func_xrefs=[
                (
                    "LoggingChannel_Init",
                    ["Networking"],
                    [],
                    [],
                    [],
                )
            ],
            generate_yaml_desired_fields=[
                (
                    "LoggingChannel_Init",
                    ["func_name", "func_va", "func_rva", "func_size", "func_sig"],
                )
            ],
            debug=True,
        )

        self.assertFalse(result)

    async def test_preprocess_common_skill_rejects_empty_positive_xref_sources(
        self,
    ) -> None:
        result = await ida_analyze_util.preprocess_common_skill(
            session="session",
            expected_outputs=["/tmp/LoggingChannel_Init.windows.yaml"],
            old_yaml_map={},
            new_binary_dir="/tmp",
            platform="windows",
            image_base=0x180000000,
            func_names=["LoggingChannel_Init"],
            func_xrefs=[
                (
                    "LoggingChannel_Init",
                    [],
                    [],
                    [],
                    [],
                    [],
                )
            ],
            generate_yaml_desired_fields=[
                (
                    "LoggingChannel_Init",
                    ["func_name", "func_va", "func_rva", "func_size", "func_sig"],
                )
            ],
            debug=True,
        )

        self.assertFalse(result)


class TestLlmDecompileSupport(unittest.IsolatedAsyncioTestCase):
    def test_parse_llm_decompile_response_normalizes_found_funcptr(self) -> None:
        response_text = """
```yaml
found_funcptr:
  - insn_va: 0x180666600
    insn_disasm: " lea     rdx, sub_15BC910 "
    funcptr_name: " CLoopModeGame_OnClientPollNetworking "
  - insn_va: 0x180666601
```
""".strip()

        parsed = ida_analyze_util.parse_llm_decompile_response(response_text)

        self.assertEqual(
            [
                {
                    "insn_va": "0x180666600",
                    "insn_disasm": "lea     rdx, sub_15BC910",
                    "funcptr_name": "CLoopModeGame_OnClientPollNetworking",
                }
            ],
            parsed["found_funcptr"],
        )
        self.assertEqual([], parsed["found_call"])
        self.assertEqual([], parsed["found_vcall"])

    def test_parse_llm_decompile_response_normalizes_all_sections(self) -> None:
        response_text = """
```yaml
found_vcall:
  - insn_va: 0x180777700
    insn_disasm: " call    [rax+68h] "
    vfunc_offset: 0x68
    func_name: " ILoopMode_OnLoopActivate "
  - invalid: true
found_call:
  - insn_va: 0x180888800
    insn_disasm: " call    sub_180999900 "
    func_name: " CLoopModeGame_RegisterEventMapInternal "
  - insn_disasm: call    sub_missing
found_gv:
  - insn_va: 0x180444400
    insn_disasm: " mov     rcx, cs:qword_180666600 "
    gv_name: " s_GameEventManager "
  - insn_va: 0x180000001
found_struct_offset:
  - insn_va: 0x1801BA12A
    insn_disasm: " mov     rcx, [r14+58h] "
    offset: 0x58
    size: 8
    struct_name: " CGameResourceService "
    member_name: " m_pEntitySystem "
  - member_name: only_member
```
""".strip()

        parsed = ida_analyze_util.parse_llm_decompile_response(response_text)

        self.assertEqual(
            {
                "found_vcall": [
                    {
                        "insn_va": "0x180777700",
                        "insn_disasm": "call    [rax+68h]",
                        "vfunc_offset": "0x68",
                        "func_name": "ILoopMode_OnLoopActivate",
                    }
                ],
                "found_call": [
                    {
                        "insn_va": "0x180888800",
                        "insn_disasm": "call    sub_180999900",
                        "func_name": "CLoopModeGame_RegisterEventMapInternal",
                    }
                ],
                "found_funcptr": [],
                "found_gv": [
                    {
                        "insn_va": "0x180444400",
                        "insn_disasm": "mov     rcx, cs:qword_180666600",
                        "gv_name": "s_GameEventManager",
                    }
                ],
                "found_struct_offset": [
                    {
                        "insn_va": "0x1801BA12A",
                        "insn_disasm": "mov     rcx, [r14+58h]",
                        "offset": "0x58",
                        "size": "8",
                        "struct_name": "CGameResourceService",
                        "member_name": "m_pEntitySystem",
                    }
                ],
            },
            parsed,
        )

    async def test_call_llm_decompile_uses_shared_llm_helper_and_parses_yaml(
        self,
    ) -> None:
        response_text = """
```yaml
found_vcall:
  - insn_va: 0x180777700
    insn_disasm: " call    [rax+68h] "
    vfunc_offset: 0x68
    func_name: " ILoopMode_OnLoopActivate "
found_call: []
found_gv: []
found_struct_offset: []
```
""".strip()

        with patch.object(
            ida_analyze_util,
            "call_llm_text",
            return_value=response_text,
            create=True,
        ) as mock_call_llm_text:
            parsed = await ida_analyze_util.call_llm_decompile(
                client=object(),
                model="gpt-4.1-mini",
                symbol_name_list=["ILoopMode_OnLoopActivate"],
                disasm_code="call    [rax+68h]",
                procedure="(*v1->lpVtbl->OnLoopActivate)(v1);",
            )

        self.assertEqual(
            {
                "found_vcall": [
                    {
                        "insn_va": "0x180777700",
                        "insn_disasm": "call    [rax+68h]",
                        "vfunc_offset": "0x68",
                        "func_name": "ILoopMode_OnLoopActivate",
                    }
                ],
                "found_call": [],
                "found_funcptr": [],
                "found_gv": [],
                "found_struct_offset": [],
            },
            parsed,
        )
        mock_call_llm_text.assert_called_once()
        self.assertEqual("gpt-4.1-mini", mock_call_llm_text.call_args.kwargs["model"])
        self.assertNotIn("temperature", mock_call_llm_text.call_args.kwargs)

    async def test_call_llm_decompile_forwards_explicit_temperature(
        self,
    ) -> None:
        response_text = """
```yaml
found_vcall: []
found_call: []
found_gv: []
found_struct_offset: []
```
""".strip()

        with patch.object(
            ida_analyze_util,
            "call_llm_text",
            return_value=response_text,
            create=True,
        ) as mock_call_llm_text:
            parsed = await ida_analyze_util.call_llm_decompile(
                client=object(),
                model="gpt-4.1-mini",
                symbol_name_list=["ILoopMode_OnLoopActivate"],
                disasm_code="call    [rax+68h]",
                procedure="(*v1->lpVtbl->OnLoopActivate)(v1);",
                temperature=0.35,
            )

        self.assertEqual(
            {
                "found_vcall": [],
                "found_call": [],
                "found_funcptr": [],
                "found_gv": [],
                "found_struct_offset": [],
            },
            parsed,
        )
        mock_call_llm_text.assert_called_once()
        self.assertEqual(0.35, mock_call_llm_text.call_args.kwargs["temperature"])

    async def test_call_llm_decompile_fails_closed_when_shared_helper_raises(
        self,
    ) -> None:
        with patch.object(
            ida_analyze_util,
            "call_llm_text",
            side_effect=RuntimeError("llm unavailable"),
            create=True,
        ):
            parsed = await ida_analyze_util.call_llm_decompile(
                client=object(),
                model="gpt-4.1-mini",
                symbol_name_list=["ILoopMode_OnLoopActivate"],
                disasm_code="call    [rax+68h]",
                procedure="(*v1->lpVtbl->OnLoopActivate)(v1);",
            )

        self.assertEqual(
            {
                "found_vcall": [],
                "found_call": [],
                "found_funcptr": [],
                "found_gv": [],
                "found_struct_offset": [],
            },
            parsed,
        )

    async def test_preprocess_func_sig_via_mcp_supports_direct_vtable_generation(
        self,
    ) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            session = AsyncMock()
            session.call_tool.return_value = _py_eval_payload(
                {
                    "func_va": "0x180123450",
                    "func_size": "0x40",
                }
            )

            with patch.object(
                ida_analyze_util,
                "preprocess_vtable_via_mcp",
                AsyncMock(
                    return_value={
                        "vtable_class": "CLoopModeGame",
                        "vtable_symbol": "??_7CLoopModeGame@@6B@",
                        "vtable_va": "0x180001000",
                        "vtable_rva": "0x1000",
                        "vtable_size": "0x90",
                        "vtable_numvfunc": 32,
                        "vtable_entries": {13: "0x180123450"},
                    }
                ),
            ), patch.object(
                ida_analyze_util,
                "preprocess_gen_func_sig_via_mcp",
                AsyncMock(
                    return_value={
                        "func_va": "0x180123450",
                        "func_rva": "0x123450",
                        "func_size": "0x40",
                        "func_sig": "48 89 ??",
                    }
                ),
            ):
                result = await ida_analyze_util.preprocess_func_sig_via_mcp(
                    session=session,
                    new_path=f"{temp_dir}/CLoopModeGame_OnLoopActivate.windows.yaml",
                    old_path=None,
                    image_base=0x180000000,
                    new_binary_dir=temp_dir,
                    platform="windows",
                    func_name="CLoopModeGame_OnLoopActivate",
                    direct_vtable_class="CLoopModeGame",
                    direct_vfunc_offset="0x68",
                    debug=False,
                )

        self.assertIsNotNone(result)
        assert result is not None
        self.assertEqual("CLoopModeGame_OnLoopActivate", result["func_name"])
        self.assertEqual("0x180123450", result["func_va"])
        self.assertEqual("48 89 ??", result["func_sig"])
        self.assertEqual("CLoopModeGame", result["vtable_name"])
        self.assertEqual("0x68", result["vfunc_offset"])
        self.assertEqual(13, result["vfunc_index"])

    async def test_preprocess_gen_struct_offset_sig_via_mcp_generates_current_version_sig(
        self,
    ) -> None:
        session = AsyncMock()

        def _fake_call_tool(*, name: str, arguments: dict[str, object]):
            if name == "py_eval":
                self.assertIn("DecodeInstruction", arguments["code"])
                self.assertIn("ida_bytes.get_bytes", arguments["code"])
                return _py_eval_payload(
                    [
                        {
                            "offset_inst_va": "0x1801BA12A",
                            "insts": [
                                {
                                    "size": 4,
                                    "bytes": "498b4e58",
                                    "wild": [3],
                                },
                                {
                                    "size": 3,
                                    "bytes": "4885c9",
                                    "wild": [],
                                },
                            ],
                        }
                    ]
                )
            if name == "find_bytes":
                self.assertEqual(
                    ["49 8B 4E ??"],
                    arguments["patterns"],
                )
                return _FakeCallToolResult(
                    [
                        {
                            "matches": ["0x1801BA12A"],
                            "n": 1,
                        }
                    ]
                )
            raise AssertionError(f"unexpected MCP tool: {name}")

        session.call_tool.side_effect = _fake_call_tool

        result = await ida_analyze_util.preprocess_gen_struct_offset_sig_via_mcp(
            session=session,
            struct_name="CGameResourceService",
            member_name="m_pEntitySystem",
            offset="0x58",
            offset_inst_va="0x1801BA12A",
            image_base=0x180000000,
            size=8,
            min_sig_bytes=4,
            debug=False,
        )

        self.assertEqual(
            {
                "struct_name": "CGameResourceService",
                "member_name": "m_pEntitySystem",
                "offset": "0x58",
                "size": 8,
                "offset_sig": "49 8B 4E ??",
                "offset_sig_disp": 0,
            },
            result,
        )

    async def test_preprocess_gen_gv_sig_via_mcp_syncs_py_eval_locals_into_globals(
        self,
    ) -> None:
        session = AsyncMock()

        def _fake_call_tool(*, name: str, arguments: dict[str, object]):
            if name == "py_eval":
                self.assertIn("def _resolve_disp_off", arguments["code"])
                self.assertIn("def _collect_sig_stream", arguments["code"])
                self.assertIn("def _try_add", arguments["code"])
                self.assertIn("globals().update(locals())", arguments["code"])
                return _py_eval_payload(
                    [
                        {
                            "gv_inst_va": "0x1801BA12A",
                            "gv_inst_length": 6,
                            "gv_inst_disp": 2,
                            "insts": [
                                {
                                    "ea": "0x1801BA12A",
                                    "size": 6,
                                    "bytes": "8b0d78563412",
                                    "wild": [2, 3, 4, 5],
                                },
                                {
                                    "ea": "0x1801BA130",
                                    "size": 3,
                                    "bytes": "4885c9",
                                    "wild": [],
                                },
                            ],
                        }
                    ]
                )
            if name == "find_bytes":
                self.assertEqual(
                    ["8B 0D ?? ?? ?? ??"],
                    arguments["patterns"],
                )
                return _FakeCallToolResult(
                    [
                        {
                            "matches": ["0x1801BA12A"],
                            "n": 1,
                        }
                    ]
                )
            raise AssertionError(f"unexpected MCP tool: {name}")

        session.call_tool.side_effect = _fake_call_tool

        result = await ida_analyze_util.preprocess_gen_gv_sig_via_mcp(
            session=session,
            gv_va="0x180123456",
            image_base=0x180000000,
            gv_access_inst_va="0x1801BA12A",
            min_sig_bytes=6,
            debug=False,
        )

        self.assertEqual(
            {
                "gv_va": "0x180123456",
                "gv_rva": "0x123456",
                "gv_sig": "8B 0D ?? ?? ?? ??",
                "gv_sig_va": "0x1801ba12a",
                "gv_inst_offset": 0,
                "gv_inst_length": 6,
                "gv_inst_disp": 2,
            },
            result,
        )

    async def test_preprocess_gen_vfunc_sig_via_mcp_generates_current_version_sig(
        self,
    ) -> None:
        session = AsyncMock()

        def _fake_call_tool(*, name: str, arguments: dict[str, object]):
            if name == "py_eval":
                self.assertIn("target_vfunc_offset = 120", arguments["code"])
                self.assertIn("target_inst = 6442757059", arguments["code"])
                self.assertIn("globals().update(locals())", arguments["code"])
                return _py_eval_payload(
                    {
                        "vfunc_sig_va": "0x18004abc3",
                        "vfunc_inst_length": 6,
                        "vfunc_disp_offset": 2,
                        "vfunc_disp_size": 4,
                        "insts": [
                            {
                                "ea": "0x18004abc3",
                                "size": 6,
                                "bytes": "ff9078000000",
                                "wild": [],
                            },
                            {
                                "ea": "0x18004abc9",
                                "size": 3,
                                "bytes": "4885c0",
                                "wild": [],
                            },
                        ],
                    }
                )
            if name == "find_bytes":
                self.assertEqual(
                    ["FF 90 78 00 00 00"],
                    arguments["patterns"],
                )
                return _FakeCallToolResult(
                    [
                        {
                            "matches": ["0x18004abc3"],
                            "n": 1,
                        }
                    ]
                )
            raise AssertionError(f"unexpected MCP tool: {name}")

        session.call_tool.side_effect = _fake_call_tool

        result = await ida_analyze_util.preprocess_gen_vfunc_sig_via_mcp(
            session=session,
            inst_va="0x18004ABC3",
            vfunc_offset="0x78",
            debug=False,
        )

        self.assertEqual(
            {
                "vfunc_sig": "FF 90 78 00 00 00",
                "vfunc_sig_va": "0x18004abc3",
                "vfunc_sig_disp": 0,
                "vfunc_inst_length": 6,
                "vfunc_disp_offset": 2,
                "vfunc_disp_size": 4,
                "vfunc_offset": "0x78",
                "vfunc_sig_max_match": 1,
            },
            result,
        )

    async def test_preprocess_gen_vfunc_sig_via_mcp_accepts_match_count_within_limit(
        self,
    ) -> None:
        session = AsyncMock()

        def _fake_call_tool(*, name: str, arguments: dict[str, object]):
            if name == "py_eval":
                return _py_eval_payload(
                    {
                        "vfunc_sig_va": "0x18004abc3",
                        "vfunc_inst_length": 6,
                        "vfunc_disp_offset": 2,
                        "vfunc_disp_size": 4,
                        "insts": [
                            {
                                "ea": "0x18004abc3",
                                "size": 6,
                                "bytes": "ff9078000000",
                                "wild": [],
                            },
                        ],
                    }
                )
            if name == "find_bytes":
                self.assertEqual(["FF 90 78 00 00 00"], arguments["patterns"])
                self.assertEqual(11, arguments["limit"])
                return _FakeCallToolResult(
                    [
                        {
                            "matches": ["0x18004abc3", "0x18010abc3"],
                            "n": 2,
                        }
                    ]
                )
            raise AssertionError(f"unexpected MCP tool: {name}")

        session.call_tool.side_effect = _fake_call_tool

        result = await ida_analyze_util.preprocess_gen_vfunc_sig_via_mcp(
            session=session,
            inst_va="0x18004ABC3",
            vfunc_offset="0x78",
            max_match_count=10,
            debug=False,
        )

        self.assertEqual("FF 90 78 00 00 00", result["vfunc_sig"])
        self.assertEqual(10, result["vfunc_sig_max_match"])

    async def test_preprocess_gen_vfunc_sig_via_mcp_rejects_match_count_over_limit(
        self,
    ) -> None:
        session = AsyncMock()

        def _fake_call_tool(*, name: str, arguments: dict[str, object]):
            if name == "py_eval":
                return _py_eval_payload(
                    {
                        "vfunc_sig_va": "0x18004abc3",
                        "vfunc_inst_length": 6,
                        "vfunc_disp_offset": 2,
                        "vfunc_disp_size": 4,
                        "insts": [
                            {
                                "ea": "0x18004abc3",
                                "size": 6,
                                "bytes": "ff9078000000",
                                "wild": [],
                            },
                        ],
                    }
                )
            if name == "find_bytes":
                self.assertEqual(["FF 90 78 00 00 00"], arguments["patterns"])
                self.assertEqual(11, arguments["limit"])
                return _FakeCallToolResult(
                    [
                        {
                            "matches": ["0x18004abc3"],
                            "n": 11,
                        }
                    ]
                )
            raise AssertionError(f"unexpected MCP tool: {name}")

        session.call_tool.side_effect = _fake_call_tool

        result = await ida_analyze_util.preprocess_gen_vfunc_sig_via_mcp(
            session=session,
            inst_va="0x18004ABC3",
            vfunc_offset="0x78",
            max_match_count=10,
            debug=False,
        )

        self.assertIsNone(result)

    async def test_preprocess_gen_vfunc_sig_via_mcp_rejects_match_set_without_target_inst(
        self,
    ) -> None:
        session = AsyncMock()

        def _fake_call_tool(*, name: str, arguments: dict[str, object]):
            if name == "py_eval":
                return _py_eval_payload(
                    {
                        "vfunc_sig_va": "0x18004abc3",
                        "vfunc_inst_length": 6,
                        "vfunc_disp_offset": 2,
                        "vfunc_disp_size": 4,
                        "insts": [
                            {
                                "ea": "0x18004abc3",
                                "size": 6,
                                "bytes": "ff9078000000",
                                "wild": [],
                            },
                        ],
                    }
                )
            if name == "find_bytes":
                self.assertEqual(["FF 90 78 00 00 00"], arguments["patterns"])
                self.assertEqual(11, arguments["limit"])
                return _FakeCallToolResult(
                    [
                        {
                            "matches": ["0x18010abc3", "0x18020abc3"],
                            "n": 2,
                        }
                    ]
                )
            raise AssertionError(f"unexpected MCP tool: {name}")

        session.call_tool.side_effect = _fake_call_tool

        result = await ida_analyze_util.preprocess_gen_vfunc_sig_via_mcp(
            session=session,
            inst_va="0x18004ABC3",
            vfunc_offset="0x78",
            max_match_count=10,
            debug=False,
        )

        self.assertIsNone(result)

    async def test_preprocess_common_skill_uses_llm_decompile_vcall_fallback_for_func_yaml(
        self,
    ) -> None:
        func_name = "CLoopModeGame_OnLoopActivate"
        output_path = f"/tmp/{func_name}.windows.yaml"
        target_detail_payload = {
            "func_name": "CNetworkGameClient_RecordEntityBandwidth",
            "func_va": "0x180555500",
            "disasm_code": "mov     rax, [rcx]\ncall    qword ptr [rax+68h]",
            "procedure": "return this->vfptr[13](this);",
        }
        normalized_payload = {
            "found_vcall": [
                {
                    "insn_va": "0x180777700",
                    "insn_disasm": "call    [rax+68h]",
                    "vfunc_offset": "0x68",
                    "func_name": func_name,
                }
            ],
            "found_call": [],
            "found_gv": [],
            "found_struct_offset": [],
        }
        with tempfile.TemporaryDirectory() as temp_dir:
            preprocessor_dir = Path(temp_dir) / "ida_preprocessor_scripts"
            session = AsyncMock()
            prompt_text = (
                "ref={disasm_for_reference}|{procedure_for_reference}|"
                "target={disasm_code}|{procedure}|symbols={symbol_name_list}"
            )
            (preprocessor_dir / "prompt").mkdir(parents=True, exist_ok=True)
            (preprocessor_dir / "prompt" / "call_llm_decompile.md").write_text(
                prompt_text,
                encoding="utf-8",
            )
            _write_yaml(
                preprocessor_dir / "references" / "reference.yaml",
                {
                    "func_name": target_detail_payload["func_name"],
                    "disasm_code": "mov rax, [rcx]",
                    "procedure": "return this->vfptr[13](this);",
                },
            )
            fake_client = object()

            async def _session_call_tool(*, name, arguments):
                self.assertEqual("py_eval", name)
                code = arguments["code"]
                if "candidate_names =" in code:
                    return _py_eval_payload(
                        [
                            {
                                "name": target_detail_payload["func_name"],
                                "func_va": target_detail_payload["func_va"],
                            }
                        ]
                    )
                if "'disasm_code': get_disasm(func_start)" in code:
                    return _py_eval_payload(target_detail_payload)
                raise AssertionError(f"unexpected py_eval code: {code}")

            session.call_tool.side_effect = _session_call_tool

            with patch.object(
                ida_analyze_util,
                "_get_preprocessor_scripts_dir",
                return_value=preprocessor_dir,
            ), patch.object(
                ida_analyze_util,
                "create_openai_client",
                return_value=fake_client,
                create=True,
            ), patch.object(
                ida_analyze_util,
                "preprocess_func_sig_via_mcp",
                AsyncMock(return_value=None),
            ), patch.object(
                ida_analyze_util,
                "_get_func_basic_info_via_mcp",
                AsyncMock(
                    return_value={
                        "func_va": "0x180123450",
                        "func_rva": "0x123450",
                        "func_size": "0x40",
                    }
                ),
            ), patch.object(
                ida_analyze_util,
                "preprocess_gen_func_sig_via_mcp",
                AsyncMock(return_value={"func_sig": "40 53"}),
            ), patch.object(
                ida_analyze_util,
                "call_llm_decompile",
                create=True,
                new_callable=AsyncMock,
                return_value=normalized_payload,
            ) as mock_call_llm_decompile, patch.object(
                ida_analyze_util,
                "preprocess_vtable_via_mcp",
                AsyncMock(
                    return_value={
                        "vtable_class": "CLoopModeGame",
                        "vtable_symbol": "??_7CLoopModeGame@@6B@",
                        "vtable_va": "0x180001000",
                        "vtable_rva": "0x1000",
                        "vtable_size": "0x90",
                        "vtable_numvfunc": 32,
                        "vtable_entries": {13: "0x180123450"},
                    }
                ),
            ), patch.object(
                ida_analyze_util,
                "write_func_yaml",
            ) as mock_write_func_yaml, patch.object(
                ida_analyze_util,
                "_rename_func_in_ida",
                AsyncMock(return_value=None),
            ):
                result = await ida_analyze_util.preprocess_common_skill(
                    session=session,
                    expected_outputs=[output_path],
                    old_yaml_map={},
                    new_binary_dir="/tmp",
                    platform="windows",
                    image_base=0x180000000,
                    func_names=[func_name],
                    func_vtable_relations=[(func_name, "CLoopModeGame")],
                    generate_yaml_desired_fields=[
                        (
                            func_name,
                            ["func_name", "vtable_name", "vfunc_offset", "vfunc_index"],
                        )
                    ],
                    llm_decompile_specs=[
                        (
                            func_name,
                            "prompt/call_llm_decompile.md",
                            "references/reference.yaml",
                        )
                    ],
                    llm_config={
                        "model": "gpt-4.1-mini",
                        "api_key": "test-api-key",
                    },
                    debug=True,
                )

        self.assertTrue(result)
        mock_call_llm_decompile.assert_awaited_once()
        self.assertIs(fake_client, mock_call_llm_decompile.call_args.kwargs["client"])
        self.assertEqual(
            "gpt-4.1-mini",
            mock_call_llm_decompile.call_args.kwargs["model"],
        )
        self.assertEqual(
            prompt_text,
            mock_call_llm_decompile.call_args.kwargs["prompt_template"],
        )
        self.assertEqual(
            "mov rax, [rcx]",
            mock_call_llm_decompile.call_args.kwargs["disasm_for_reference"],
        )
        self.assertEqual(
            "return this->vfptr[13](this);",
            mock_call_llm_decompile.call_args.kwargs["procedure_for_reference"],
        )
        self.assertEqual(
            target_detail_payload["disasm_code"],
            mock_call_llm_decompile.call_args.kwargs["disasm_code"],
        )
        self.assertEqual(
            target_detail_payload["procedure"],
            mock_call_llm_decompile.call_args.kwargs["procedure"],
        )
        self.assertEqual(
            [func_name],
            mock_call_llm_decompile.call_args.kwargs["symbol_name_list"],
        )
        mock_write_func_yaml.assert_called_once()
        written_payload = mock_write_func_yaml.call_args.args[1]
        self.assertEqual(func_name, written_payload["func_name"])
        self.assertEqual("0x68", written_payload["vfunc_offset"])
        self.assertEqual(13, written_payload["vfunc_index"])

    async def test_preprocess_common_skill_batches_same_llm_request_for_multiple_unresolved_targets(
        self,
    ) -> None:
        func_names = [
            "CNetworkMessages_FindNetworkGroup",
            "CNetworkMessages_FindMessage",
        ]
        output_paths = [f"/tmp/{func_name}.windows.yaml" for func_name in func_names]
        target_detail_payload = {
            "func_name": "CNetworkGameClient_RecordEntityBandwidth",
            "func_va": "0x180555500",
            "disasm_code": "call    sub_180222200",
            "procedure": "return CNetworkMessages::FindNetworkGroup(this, group);",
        }
        normalized_payload = {
            "found_vcall": [],
            "found_call": [
                {
                    "insn_va": "0x180777700",
                    "insn_disasm": "call    sub_180222200",
                    "func_name": func_names[0],
                },
                {
                    "insn_va": "0x180777710",
                    "insn_disasm": "call    sub_180222210",
                    "func_name": func_names[1],
                },
            ],
            "found_gv": [],
            "found_struct_offset": [],
        }

        with tempfile.TemporaryDirectory() as temp_dir:
            preprocessor_dir = Path(temp_dir) / "ida_preprocessor_scripts"
            (preprocessor_dir / "prompt").mkdir(parents=True, exist_ok=True)
            (preprocessor_dir / "prompt" / "call_llm_decompile.md").write_text(
                "{symbol_name_list}",
                encoding="utf-8",
            )
            _write_yaml(
                preprocessor_dir / "references" / "reference.yaml",
                {
                    "func_name": target_detail_payload["func_name"],
                    "disasm_code": target_detail_payload["disasm_code"],
                    "procedure": target_detail_payload["procedure"],
                },
            )
            fake_client = object()

            async def _fake_preprocess_direct_func_sig_via_mcp(**kwargs):
                return {
                    "func_name": kwargs["func_name"],
                    "func_va": str(kwargs["direct_func_va"]).strip().lower(),
                }

            with patch.object(
                ida_analyze_util,
                "_get_preprocessor_scripts_dir",
                return_value=preprocessor_dir,
            ), patch.object(
                ida_analyze_util,
                "create_openai_client",
                return_value=fake_client,
                create=True,
            ), patch.object(
                ida_analyze_util,
                "preprocess_func_sig_via_mcp",
                AsyncMock(return_value=None),
            ), patch.object(
                ida_analyze_util,
                "_load_llm_decompile_target_detail_via_mcp",
                AsyncMock(return_value=target_detail_payload),
            ), patch.object(
                ida_analyze_util,
                "_resolve_direct_call_target_via_mcp",
                AsyncMock(side_effect=["0x180123450", "0x180223450"]),
            ), patch.object(
                ida_analyze_util,
                "_preprocess_direct_func_sig_via_mcp",
                AsyncMock(side_effect=_fake_preprocess_direct_func_sig_via_mcp),
            ), patch.object(
                ida_analyze_util,
                "call_llm_decompile",
                create=True,
                new_callable=AsyncMock,
                return_value=normalized_payload,
            ) as mock_call_llm_decompile, patch.object(
                ida_analyze_util,
                "write_func_yaml",
            ) as mock_write_func_yaml, patch.object(
                ida_analyze_util,
                "_rename_func_in_ida",
                AsyncMock(return_value=None),
            ):
                result = await ida_analyze_util.preprocess_common_skill(
                    session="session",
                    expected_outputs=output_paths,
                    old_yaml_map={},
                    new_binary_dir="/tmp",
                    platform="windows",
                    image_base=0x180000000,
                    func_names=func_names,
                    generate_yaml_desired_fields=[
                        (func_names[0], ["func_name", "func_va"]),
                        (func_names[1], ["func_name", "func_va"]),
                    ],
                    llm_decompile_specs=[
                        (
                            func_names[0],
                            "prompt/call_llm_decompile.md",
                            "references/reference.yaml",
                        ),
                        (
                            func_names[1],
                            "prompt/call_llm_decompile.md",
                            "references/reference.yaml",
                        ),
                    ],
                    llm_config={
                        "model": "gpt-4.1-mini",
                        "api_key": "test-api-key",
                    },
                    debug=True,
                )

        self.assertTrue(result)
        mock_call_llm_decompile.assert_awaited_once()
        self.assertEqual(
            func_names,
            mock_call_llm_decompile.call_args.kwargs["symbol_name_list"],
        )
        self.assertEqual(2, mock_write_func_yaml.call_count)

    async def test_preprocess_common_skill_llm_batch_excludes_fast_path_targets(
        self,
    ) -> None:
        unresolved_func_name = "CNetworkMessages_FindNetworkGroup"
        fast_path_func_name = "CNetworkMessages_FindMessage"
        func_names = [unresolved_func_name, fast_path_func_name]
        output_paths = [f"/tmp/{func_name}.windows.yaml" for func_name in func_names]
        target_detail_payload = {
            "func_name": "CNetworkGameClient_RecordEntityBandwidth",
            "func_va": "0x180555500",
            "disasm_code": "call    sub_180222200",
            "procedure": "return CNetworkMessages::FindNetworkGroup(this, group);",
        }
        normalized_payload = {
            "found_vcall": [],
            "found_call": [
                {
                    "insn_va": "0x180777700",
                    "insn_disasm": "call    sub_180222200",
                    "func_name": unresolved_func_name,
                }
            ],
            "found_gv": [],
            "found_struct_offset": [],
        }

        with tempfile.TemporaryDirectory() as temp_dir:
            preprocessor_dir = Path(temp_dir) / "ida_preprocessor_scripts"
            (preprocessor_dir / "prompt").mkdir(parents=True, exist_ok=True)
            (preprocessor_dir / "prompt" / "call_llm_decompile.md").write_text(
                "{symbol_name_list}",
                encoding="utf-8",
            )
            _write_yaml(
                preprocessor_dir / "references" / "reference.yaml",
                {
                    "func_name": target_detail_payload["func_name"],
                    "disasm_code": target_detail_payload["disasm_code"],
                    "procedure": target_detail_payload["procedure"],
                },
            )

            async def _fake_preprocess_func_sig_via_mcp(*, func_name, **_kwargs):
                if func_name == fast_path_func_name:
                    return {
                        "func_name": fast_path_func_name,
                        "func_va": "0x180333333",
                    }
                return None

            async def _fake_preprocess_direct_func_sig_via_mcp(**kwargs):
                return {
                    "func_name": kwargs["func_name"],
                    "func_va": str(kwargs["direct_func_va"]).strip().lower(),
                }

            with patch.object(
                ida_analyze_util,
                "_get_preprocessor_scripts_dir",
                return_value=preprocessor_dir,
            ), patch.object(
                ida_analyze_util,
                "create_openai_client",
                return_value=object(),
                create=True,
            ), patch.object(
                ida_analyze_util,
                "preprocess_func_sig_via_mcp",
                AsyncMock(side_effect=_fake_preprocess_func_sig_via_mcp),
            ), patch.object(
                ida_analyze_util,
                "_load_llm_decompile_target_detail_via_mcp",
                AsyncMock(return_value=target_detail_payload),
            ), patch.object(
                ida_analyze_util,
                "_resolve_direct_call_target_via_mcp",
                AsyncMock(return_value="0x180123450"),
            ), patch.object(
                ida_analyze_util,
                "_preprocess_direct_func_sig_via_mcp",
                AsyncMock(side_effect=_fake_preprocess_direct_func_sig_via_mcp),
            ), patch.object(
                ida_analyze_util,
                "call_llm_decompile",
                create=True,
                new_callable=AsyncMock,
                return_value=normalized_payload,
            ) as mock_call_llm_decompile, patch.object(
                ida_analyze_util,
                "write_func_yaml",
            ) as mock_write_func_yaml, patch.object(
                ida_analyze_util,
                "_rename_func_in_ida",
                AsyncMock(return_value=None),
            ):
                result = await ida_analyze_util.preprocess_common_skill(
                    session="session",
                    expected_outputs=output_paths,
                    old_yaml_map={},
                    new_binary_dir="/tmp",
                    platform="windows",
                    image_base=0x180000000,
                    func_names=func_names,
                    generate_yaml_desired_fields=[
                        (unresolved_func_name, ["func_name", "func_va"]),
                        (fast_path_func_name, ["func_name", "func_va"]),
                    ],
                    llm_decompile_specs=[
                        (
                            unresolved_func_name,
                            "prompt/call_llm_decompile.md",
                            "references/reference.yaml",
                        ),
                        (
                            fast_path_func_name,
                            "prompt/call_llm_decompile.md",
                            "references/reference.yaml",
                        ),
                    ],
                    llm_config={
                        "model": "gpt-4.1-mini",
                        "api_key": "test-api-key",
                    },
                    debug=True,
                )

        self.assertTrue(result)
        mock_call_llm_decompile.assert_awaited_once()
        self.assertEqual(
            [unresolved_func_name],
            mock_call_llm_decompile.call_args.kwargs["symbol_name_list"],
        )
        self.assertEqual(2, mock_write_func_yaml.call_count)

    async def test_preprocess_common_skill_llm_batch_includes_unresolved_gv_targets(
        self,
    ) -> None:
        func_name = "CNetChan_ParseNetMessageShowFilter"
        gv_name = "g_pLoggingChannel"
        output_paths = [
            f"/tmp/{func_name}.windows.yaml",
            f"/tmp/{gv_name}.windows.yaml",
        ]
        target_detail_payload = {
            "func_name": "CNetChan_ParseMessagesDemoInternal",
            "func_va": "0x180555500",
            "disasm_code": "call    CNetChan_ParseNetMessageShowFilter\nmov     ecx, cs:g_pLoggingChannel",
            "procedure": "CNetChan_ParseNetMessageShowFilter(...); LoggingSystem_Log(g_pLoggingChannel, ...);",
        }
        normalized_payload = {
            "found_vcall": [],
            "found_call": [
                {
                    "insn_va": "0x180777700",
                    "insn_disasm": "call    sub_180222200",
                    "func_name": func_name,
                }
            ],
            "found_gv": [
                {
                    "insn_va": "0x180777710",
                    "insn_disasm": "mov     ecx, cs:g_pLoggingChannel",
                    "gv_name": gv_name,
                }
            ],
            "found_struct_offset": [],
        }

        async def _fake_preprocess_direct_func_sig_via_mcp(**kwargs):
            return {
                "func_name": kwargs["func_name"],
                "func_va": str(kwargs["direct_func_va"]).strip().lower(),
            }

        async def _fake_preprocess_direct_gv_sig_via_mcp(**kwargs):
            return {
                "gv_name": kwargs["gv_name"],
                "gv_va": str(kwargs["direct_gv_va"]).strip().lower(),
            }

        with tempfile.TemporaryDirectory() as temp_dir:
            preprocessor_dir = Path(temp_dir) / "ida_preprocessor_scripts"
            (preprocessor_dir / "prompt").mkdir(parents=True, exist_ok=True)
            (preprocessor_dir / "prompt" / "call_llm_decompile.md").write_text(
                "{symbol_name_list}",
                encoding="utf-8",
            )
            _write_yaml(
                preprocessor_dir / "references" / "reference.yaml",
                {
                    "func_name": target_detail_payload["func_name"],
                    "disasm_code": target_detail_payload["disasm_code"],
                    "procedure": target_detail_payload["procedure"],
                },
            )

            with patch.object(
                ida_analyze_util,
                "_get_preprocessor_scripts_dir",
                return_value=preprocessor_dir,
            ), patch.object(
                ida_analyze_util,
                "create_openai_client",
                return_value=object(),
                create=True,
            ), patch.object(
                ida_analyze_util,
                "preprocess_func_sig_via_mcp",
                AsyncMock(return_value=None),
            ), patch.object(
                ida_analyze_util,
                "preprocess_gv_sig_via_mcp",
                AsyncMock(return_value=None),
            ), patch.object(
                ida_analyze_util,
                "_load_llm_decompile_target_detail_via_mcp",
                AsyncMock(return_value=target_detail_payload),
            ), patch.object(
                ida_analyze_util,
                "_resolve_direct_call_target_via_mcp",
                AsyncMock(return_value="0x180123450"),
            ), patch.object(
                ida_analyze_util,
                "_resolve_direct_gv_target_via_mcp",
                AsyncMock(return_value="0x180223450"),
                create=True,
            ), patch.object(
                ida_analyze_util,
                "_preprocess_direct_func_sig_via_mcp",
                AsyncMock(side_effect=_fake_preprocess_direct_func_sig_via_mcp),
            ), patch.object(
                ida_analyze_util,
                "_preprocess_direct_gv_sig_via_mcp",
                AsyncMock(side_effect=_fake_preprocess_direct_gv_sig_via_mcp),
                create=True,
            ), patch.object(
                ida_analyze_util,
                "call_llm_decompile",
                create=True,
                new_callable=AsyncMock,
                return_value=normalized_payload,
            ) as mock_call_llm_decompile, patch.object(
                ida_analyze_util,
                "write_func_yaml",
            ) as mock_write_func_yaml, patch.object(
                ida_analyze_util,
                "write_gv_yaml",
            ) as mock_write_gv_yaml, patch.object(
                ida_analyze_util,
                "_rename_func_in_ida",
                AsyncMock(return_value=None),
            ), patch.object(
                ida_analyze_util,
                "_rename_gv_in_ida",
                AsyncMock(return_value=None),
            ):
                result = await ida_analyze_util.preprocess_common_skill(
                    session="session",
                    expected_outputs=output_paths,
                    old_yaml_map={},
                    new_binary_dir="/tmp",
                    platform="windows",
                    image_base=0x180000000,
                    func_names=[func_name],
                    gv_names=[gv_name],
                    generate_yaml_desired_fields=[
                        (func_name, ["func_name", "func_va"]),
                        (gv_name, ["gv_name", "gv_va"]),
                    ],
                    llm_decompile_specs=[
                        (
                            func_name,
                            "prompt/call_llm_decompile.md",
                            "references/reference.yaml",
                        ),
                        (
                            gv_name,
                            "prompt/call_llm_decompile.md",
                            "references/reference.yaml",
                        ),
                    ],
                    llm_config={
                        "model": "gpt-4.1-mini",
                        "api_key": "test-api-key",
                    },
                    debug=True,
                )

        self.assertTrue(result)
        mock_call_llm_decompile.assert_awaited_once()
        self.assertEqual(
            [func_name, gv_name],
            mock_call_llm_decompile.call_args.kwargs["symbol_name_list"],
        )
        mock_write_func_yaml.assert_called_once()
        mock_write_gv_yaml.assert_called_once()

    async def test_preprocess_common_skill_llm_batch_includes_unresolved_struct_targets(
        self,
    ) -> None:
        func_name = "CGameResourceService_BuildResourceManifest"
        struct_member_name = "CGameResourceService_m_pEntitySystem"
        output_paths = [
            f"/tmp/{func_name}.windows.yaml",
            f"/tmp/{struct_member_name}.windows.yaml",
        ]
        old_struct_yaml_path = Path(tempfile.gettempdir()) / f"{struct_member_name}.old.yaml"
        target_detail_payload = {
            "func_name": "CGameResourceService_BuildResourceManifest",
            "func_va": "0x180555500",
            "disasm_code": (
                "call    sub_180222200\n"
                "mov     rcx, [r14+58h]"
            ),
            "procedure": (
                "CGameResourceService_BuildResourceManifest(...);\n"
                "return this->m_pEntitySystem;"
            ),
        }
        normalized_payload = {
            "found_vcall": [],
            "found_call": [
                {
                    "insn_va": "0x180777700",
                    "insn_disasm": "call    sub_180222200",
                    "func_name": func_name,
                }
            ],
            "found_gv": [],
            "found_struct_offset": [
                {
                    "insn_va": "0x180777710",
                    "insn_disasm": "mov     rcx, [r14+58h]",
                    "offset": "0x58",
                    "size": "8",
                    "struct_name": "CGameResourceService",
                    "member_name": "m_pEntitySystem",
                }
            ],
        }

        async def _fake_preprocess_direct_func_sig_via_mcp(**kwargs):
            return {
                "func_name": kwargs["func_name"],
                "func_va": str(kwargs["direct_func_va"]).strip().lower(),
            }

        try:
            _write_yaml(
                old_struct_yaml_path,
                {
                    "struct_name": "CGameResourceService",
                    "member_name": "m_pEntitySystem",
                    "offset": "0x50",
                    "size": 8,
                    "offset_sig": "49 8B 4E 50",
                },
            )

            with tempfile.TemporaryDirectory() as temp_dir:
                preprocessor_dir = Path(temp_dir) / "ida_preprocessor_scripts"
                (preprocessor_dir / "prompt").mkdir(parents=True, exist_ok=True)
                (preprocessor_dir / "prompt" / "call_llm_decompile.md").write_text(
                    "{symbol_name_list}",
                    encoding="utf-8",
                )
                _write_yaml(
                    preprocessor_dir / "references" / "reference.yaml",
                    {
                        "func_name": target_detail_payload["func_name"],
                        "disasm_code": target_detail_payload["disasm_code"],
                        "procedure": target_detail_payload["procedure"],
                    },
                )

                with patch.object(
                    ida_analyze_util,
                    "_get_preprocessor_scripts_dir",
                    return_value=preprocessor_dir,
                ), patch.object(
                    ida_analyze_util,
                    "create_openai_client",
                    return_value=object(),
                    create=True,
                ), patch.object(
                    ida_analyze_util,
                    "preprocess_func_sig_via_mcp",
                    AsyncMock(return_value=None),
                ), patch.object(
                    ida_analyze_util,
                    "preprocess_struct_offset_sig_via_mcp",
                    AsyncMock(return_value=None),
                ), patch.object(
                    ida_analyze_util,
                    "_load_llm_decompile_target_detail_via_mcp",
                    AsyncMock(return_value=target_detail_payload),
                ), patch.object(
                    ida_analyze_util,
                    "_resolve_direct_call_target_via_mcp",
                    AsyncMock(return_value="0x180123450"),
                ), patch.object(
                    ida_analyze_util,
                    "_preprocess_direct_func_sig_via_mcp",
                    AsyncMock(side_effect=_fake_preprocess_direct_func_sig_via_mcp),
                ), patch.object(
                    ida_analyze_util,
                    "_preprocess_direct_struct_offset_sig_via_mcp",
                    AsyncMock(
                        return_value={
                            "struct_name": "CGameResourceService",
                            "member_name": "m_pEntitySystem",
                            "offset": "0x58",
                            "size": 8,
                            "offset_sig": "49 8B 4E ??",
                            "offset_sig_disp": 0,
                        }
                    ),
                    create=True,
                ) as mock_preprocess_direct_struct_offset_sig, patch.object(
                    ida_analyze_util,
                    "call_llm_decompile",
                    create=True,
                    new_callable=AsyncMock,
                    return_value=normalized_payload,
                ) as mock_call_llm_decompile, patch.object(
                    ida_analyze_util,
                    "write_func_yaml",
                ) as mock_write_func_yaml, patch.object(
                    ida_analyze_util,
                    "write_struct_offset_yaml",
                ) as mock_write_struct_offset_yaml, patch.object(
                    ida_analyze_util,
                    "_rename_func_in_ida",
                    AsyncMock(return_value=None),
                ):
                    result = await ida_analyze_util.preprocess_common_skill(
                        session="session",
                        expected_outputs=output_paths,
                        old_yaml_map={
                            output_paths[1]: str(old_struct_yaml_path),
                        },
                        new_binary_dir="/tmp",
                        platform="windows",
                        image_base=0x180000000,
                        func_names=[func_name],
                        struct_member_names=[struct_member_name],
                        generate_yaml_desired_fields=[
                            (func_name, ["func_name", "func_va"]),
                            (
                                struct_member_name,
                                [
                                    "struct_name",
                                    "member_name",
                                    "offset",
                                    "size",
                                    "offset_sig",
                                    "offset_sig_disp",
                                ],
                            ),
                        ],
                        llm_decompile_specs=[
                            (
                                func_name,
                                "prompt/call_llm_decompile.md",
                                "references/reference.yaml",
                            ),
                            (
                                struct_member_name,
                                "prompt/call_llm_decompile.md",
                                "references/reference.yaml",
                            ),
                        ],
                        llm_config={
                            "model": "gpt-4.1-mini",
                            "api_key": "test-api-key",
                        },
                        debug=True,
                    )
        finally:
            if old_struct_yaml_path.exists():
                old_struct_yaml_path.unlink()

        self.assertTrue(result)
        mock_call_llm_decompile.assert_awaited_once()
        self.assertEqual(
            [func_name, struct_member_name],
            mock_call_llm_decompile.call_args.kwargs["symbol_name_list"],
        )
        mock_preprocess_direct_struct_offset_sig.assert_awaited_once_with(
            session="session",
            new_path=output_paths[1],
            image_base=0x180000000,
            struct_member_name=struct_member_name,
            struct_name="CGameResourceService",
            member_name="m_pEntitySystem",
            offset="0x58",
            offset_inst_va="0x180777710",
            size="8",
            old_path=str(old_struct_yaml_path),
            debug=True,
        )
        mock_write_func_yaml.assert_called_once()
        mock_write_struct_offset_yaml.assert_called_once()
        written_payload = mock_write_struct_offset_yaml.call_args.args[1]
        self.assertEqual("CGameResourceService", written_payload["struct_name"])
        self.assertEqual("m_pEntitySystem", written_payload["member_name"])
        self.assertEqual("0x58", written_payload["offset"])
        self.assertEqual(8, written_payload["size"])
        self.assertEqual(0, written_payload["offset_sig_disp"])

    async def test_preprocess_common_skill_llm_batch_skips_future_targets_with_unready_xref_dependencies(
        self,
    ) -> None:
        first_func_name = "CNetworkMessages_FindNetworkGroup"
        second_func_name = "CNetworkMessages_FindMessage"
        func_names = [first_func_name, second_func_name]
        target_detail_payload = {
            "func_name": "CNetworkGameClient_RecordEntityBandwidth",
            "func_va": "0x180555500",
            "disasm_code": "call    sub_180222200",
            "procedure": "return CNetworkMessages::FindNetworkGroup(this, group);",
        }
        llm_payload = {
            "found_vcall": [],
            "found_call": [
                {
                    "insn_va": "0x180777700",
                    "insn_disasm": "call    sub_180222200",
                    "func_name": first_func_name,
                }
            ],
            "found_gv": [],
            "found_struct_offset": [],
        }

        with tempfile.TemporaryDirectory() as temp_dir:
            new_binary_dir = Path(temp_dir) / "current"
            preprocessor_dir = Path(temp_dir) / "ida_preprocessor_scripts"
            first_output = new_binary_dir / f"{first_func_name}.windows.yaml"
            second_output = new_binary_dir / f"{second_func_name}.windows.yaml"
            output_paths = [str(first_output), str(second_output)]
            dependent_yaml_path = new_binary_dir / f"{first_func_name}.windows.yaml"

            (preprocessor_dir / "prompt").mkdir(parents=True, exist_ok=True)
            (preprocessor_dir / "prompt" / "call_llm_decompile.md").write_text(
                "{symbol_name_list}",
                encoding="utf-8",
            )
            _write_yaml(
                preprocessor_dir / "references" / "reference.yaml",
                {
                    "func_name": target_detail_payload["func_name"],
                    "disasm_code": target_detail_payload["disasm_code"],
                    "procedure": target_detail_payload["procedure"],
                },
            )

            async def _fake_preprocess_func_xrefs_via_mcp(*, func_name, **_kwargs):
                if func_name != second_func_name:
                    return None
                if dependent_yaml_path.is_file():
                    return {
                        "func_name": second_func_name,
                        "func_va": "0x180333333",
                    }
                return None

            async def _fake_preprocess_direct_func_sig_via_mcp(**kwargs):
                return {
                    "func_name": kwargs["func_name"],
                    "func_va": str(kwargs["direct_func_va"]).strip().lower(),
                }

            def _fake_write_func_yaml(path, data):
                output_path = Path(path)
                output_path.parent.mkdir(parents=True, exist_ok=True)
                output_path.write_text(
                    yaml.safe_dump(data, sort_keys=False),
                    encoding="utf-8",
                )

            with patch.object(
                ida_analyze_util,
                "_get_preprocessor_scripts_dir",
                return_value=preprocessor_dir,
            ), patch.object(
                ida_analyze_util,
                "create_openai_client",
                return_value=object(),
                create=True,
            ), patch.object(
                ida_analyze_util,
                "preprocess_func_sig_via_mcp",
                AsyncMock(return_value=None),
            ), patch.object(
                ida_analyze_util,
                "preprocess_func_xrefs_via_mcp",
                AsyncMock(side_effect=_fake_preprocess_func_xrefs_via_mcp),
            ), patch.object(
                ida_analyze_util,
                "_load_llm_decompile_target_detail_via_mcp",
                AsyncMock(return_value=target_detail_payload),
            ), patch.object(
                ida_analyze_util,
                "_resolve_direct_call_target_via_mcp",
                AsyncMock(return_value="0x180123450"),
            ), patch.object(
                ida_analyze_util,
                "_preprocess_direct_func_sig_via_mcp",
                AsyncMock(side_effect=_fake_preprocess_direct_func_sig_via_mcp),
            ), patch.object(
                ida_analyze_util,
                "call_llm_decompile",
                create=True,
                new_callable=AsyncMock,
                return_value=llm_payload,
            ) as mock_call_llm_decompile, patch.object(
                ida_analyze_util,
                "write_func_yaml",
                side_effect=_fake_write_func_yaml,
            ) as mock_write_func_yaml, patch.object(
                ida_analyze_util,
                "_rename_func_in_ida",
                AsyncMock(return_value=None),
            ):
                result = await ida_analyze_util.preprocess_common_skill(
                    session="session",
                    expected_outputs=output_paths,
                    old_yaml_map={},
                    new_binary_dir=str(new_binary_dir),
                    platform="windows",
                    image_base=0x180000000,
                    func_names=func_names,
                    func_xrefs=[
                        (
                            second_func_name,
                            ["dummy-string"],
                            [],
                            [first_func_name],
                            [],
                            [],
                        ),
                    ],
                    generate_yaml_desired_fields=[
                        (first_func_name, ["func_name", "func_va"]),
                        (second_func_name, ["func_name", "func_va"]),
                    ],
                    llm_decompile_specs=[
                        (
                            first_func_name,
                            "prompt/call_llm_decompile.md",
                            "references/reference.yaml",
                        ),
                        (
                            second_func_name,
                            "prompt/call_llm_decompile.md",
                            "references/reference.yaml",
                        ),
                    ],
                    llm_config={
                        "model": "gpt-4.1-mini",
                        "api_key": "test-api-key",
                    },
                    debug=True,
                )

        self.assertTrue(result)
        mock_call_llm_decompile.assert_awaited_once()
        self.assertEqual(
            [first_func_name],
            mock_call_llm_decompile.call_args.kwargs["symbol_name_list"],
        )
        self.assertEqual(2, mock_write_func_yaml.call_count)

    async def test_preprocess_common_skill_llm_batch_issues_second_request_for_symbol_not_covered_in_first_batch(
        self,
    ) -> None:
        first_func_name = "CNetworkMessages_FindNetworkGroup"
        second_func_name = "CNetworkMessages_FindMessage"
        func_names = [first_func_name, second_func_name]
        target_detail_payload = {
            "func_name": "CNetworkGameClient_RecordEntityBandwidth",
            "func_va": "0x180555500",
            "disasm_code": "call    sub_180222200",
            "procedure": "return CNetworkMessages::FindNetworkGroup(this, group);",
        }
        first_llm_payload = {
            "found_vcall": [],
            "found_call": [
                {
                    "insn_va": "0x180777700",
                    "insn_disasm": "call    sub_180222200",
                    "func_name": first_func_name,
                }
            ],
            "found_gv": [],
            "found_struct_offset": [],
        }
        second_llm_payload = {
            "found_vcall": [],
            "found_call": [
                {
                    "insn_va": "0x180777710",
                    "insn_disasm": "call    sub_180222210",
                    "func_name": second_func_name,
                }
            ],
            "found_gv": [],
            "found_struct_offset": [],
        }

        with tempfile.TemporaryDirectory() as temp_dir:
            new_binary_dir = Path(temp_dir) / "current"
            preprocessor_dir = Path(temp_dir) / "ida_preprocessor_scripts"
            first_output = new_binary_dir / f"{first_func_name}.windows.yaml"
            second_output = new_binary_dir / f"{second_func_name}.windows.yaml"
            output_paths = [str(first_output), str(second_output)]
            dependent_yaml_path = new_binary_dir / f"{first_func_name}.windows.yaml"

            (preprocessor_dir / "prompt").mkdir(parents=True, exist_ok=True)
            (preprocessor_dir / "prompt" / "call_llm_decompile.md").write_text(
                "{symbol_name_list}",
                encoding="utf-8",
            )
            _write_yaml(
                preprocessor_dir / "references" / "reference.yaml",
                {
                    "func_name": target_detail_payload["func_name"],
                    "disasm_code": target_detail_payload["disasm_code"],
                    "procedure": target_detail_payload["procedure"],
                },
            )

            async def _fake_preprocess_func_xrefs_via_mcp(*, func_name, **_kwargs):
                if func_name != second_func_name:
                    return None
                if dependent_yaml_path.is_file():
                    return None
                return None

            async def _fake_preprocess_direct_func_sig_via_mcp(**kwargs):
                return {
                    "func_name": kwargs["func_name"],
                    "func_va": str(kwargs["direct_func_va"]).strip().lower(),
                }

            def _fake_write_func_yaml(path, data):
                output_path = Path(path)
                output_path.parent.mkdir(parents=True, exist_ok=True)
                output_path.write_text(
                    yaml.safe_dump(data, sort_keys=False),
                    encoding="utf-8",
                )

            with patch.object(
                ida_analyze_util,
                "_get_preprocessor_scripts_dir",
                return_value=preprocessor_dir,
            ), patch.object(
                ida_analyze_util,
                "create_openai_client",
                return_value=object(),
                create=True,
            ), patch.object(
                ida_analyze_util,
                "preprocess_func_sig_via_mcp",
                AsyncMock(return_value=None),
            ), patch.object(
                ida_analyze_util,
                "preprocess_func_xrefs_via_mcp",
                AsyncMock(side_effect=_fake_preprocess_func_xrefs_via_mcp),
            ), patch.object(
                ida_analyze_util,
                "_load_llm_decompile_target_detail_via_mcp",
                AsyncMock(return_value=target_detail_payload),
            ), patch.object(
                ida_analyze_util,
                "_resolve_direct_call_target_via_mcp",
                AsyncMock(side_effect=["0x180123450", "0x180223450"]),
            ), patch.object(
                ida_analyze_util,
                "_preprocess_direct_func_sig_via_mcp",
                AsyncMock(side_effect=_fake_preprocess_direct_func_sig_via_mcp),
            ), patch.object(
                ida_analyze_util,
                "call_llm_decompile",
                create=True,
                new_callable=AsyncMock,
                side_effect=[first_llm_payload, second_llm_payload],
            ) as mock_call_llm_decompile, patch.object(
                ida_analyze_util,
                "write_func_yaml",
                side_effect=_fake_write_func_yaml,
            ) as mock_write_func_yaml, patch.object(
                ida_analyze_util,
                "_rename_func_in_ida",
                AsyncMock(return_value=None),
            ):
                result = await ida_analyze_util.preprocess_common_skill(
                    session="session",
                    expected_outputs=output_paths,
                    old_yaml_map={},
                    new_binary_dir=str(new_binary_dir),
                    platform="windows",
                    image_base=0x180000000,
                    func_names=func_names,
                    func_xrefs=[
                        (
                            second_func_name,
                            ["dummy-string"],
                            [],
                            [first_func_name],
                            [],
                            [],
                        ),
                    ],
                    generate_yaml_desired_fields=[
                        (first_func_name, ["func_name", "func_va"]),
                        (second_func_name, ["func_name", "func_va"]),
                    ],
                    llm_decompile_specs=[
                        (
                            first_func_name,
                            "prompt/call_llm_decompile.md",
                            "references/reference.yaml",
                        ),
                        (
                            second_func_name,
                            "prompt/call_llm_decompile.md",
                            "references/reference.yaml",
                        ),
                    ],
                    llm_config={
                        "model": "gpt-4.1-mini",
                        "api_key": "test-api-key",
                    },
                    debug=True,
                )

        self.assertTrue(result)
        self.assertEqual(2, mock_call_llm_decompile.await_count)
        self.assertEqual(
            [first_func_name],
            mock_call_llm_decompile.await_args_list[0].kwargs["symbol_name_list"],
        )
        self.assertEqual(
            [second_func_name],
            mock_call_llm_decompile.await_args_list[1].kwargs["symbol_name_list"],
        )
        self.assertEqual(2, mock_write_func_yaml.call_count)

    async def test_preprocess_common_skill_llm_fallback_skips_missing_reference_yaml(
        self,
    ) -> None:
        func_name = "CLoopModeGame_OnLoopActivate"
        output_path = f"/tmp/{func_name}.windows.yaml"

        with tempfile.TemporaryDirectory() as temp_dir:
            preprocessor_dir = Path(temp_dir) / "ida_preprocessor_scripts"
            (preprocessor_dir / "prompt").mkdir(parents=True, exist_ok=True)
            (preprocessor_dir / "prompt" / "call_llm_decompile.md").write_text(
                "{symbol_name_list}",
                encoding="utf-8",
            )

            with patch.object(
                ida_analyze_util,
                "_get_preprocessor_scripts_dir",
                return_value=preprocessor_dir,
            ), patch.object(
                ida_analyze_util,
                "preprocess_func_sig_via_mcp",
                AsyncMock(return_value=None),
            ), patch.object(
                ida_analyze_util,
                "create_openai_client",
                create=True,
            ) as mock_create_openai_client, patch.object(
                ida_analyze_util,
                "call_llm_decompile",
                create=True,
                new_callable=AsyncMock,
            ) as mock_call_llm_decompile, patch.object(
                ida_analyze_util,
                "write_func_yaml",
            ) as mock_write_func_yaml:
                result = await ida_analyze_util.preprocess_common_skill(
                    session="session",
                    expected_outputs=[output_path],
                    old_yaml_map={},
                    new_binary_dir="/tmp",
                    platform="windows",
                    image_base=0x180000000,
                    func_names=[func_name],
                    func_vtable_relations=[(func_name, "CLoopModeGame")],
                    generate_yaml_desired_fields=[
                        (
                            func_name,
                            ["func_name", "vtable_name", "vfunc_offset", "vfunc_index"],
                        )
                    ],
                    llm_decompile_specs=[
                        (
                            func_name,
                            "prompt/call_llm_decompile.md",
                            "references/missing.yaml",
                        )
                    ],
                    llm_config={
                        "model": "gpt-4.1-mini",
                        "api_key": "test-api-key",
                    },
                    debug=True,
                )

                self.assertFalse(result)
                mock_create_openai_client.assert_not_called()
                mock_call_llm_decompile.assert_not_awaited()
                mock_write_func_yaml.assert_not_called()

    async def test_preprocess_common_skill_uses_llm_decompile_direct_call_fallback_without_vtable_relation(
        self,
    ) -> None:
        func_name = "CNetworkMessages_FindNetworkGroup"
        output_path = f"/tmp/{func_name}.windows.yaml"
        target_detail_payload = {
            "func_name": "CNetworkGameClient_RecordEntityBandwidth",
            "func_va": "0x180555500",
            "disasm_code": "call    sub_180222200",
            "procedure": "return CNetworkMessages::FindNetworkGroup(this, group);",
        }
        normalized_payload = {
            "found_vcall": [],
            "found_call": [
                {
                    "insn_va": "0x180777700",
                    "insn_disasm": "call    sub_180222200",
                    "func_name": func_name,
                }
            ],
            "found_gv": [],
            "found_struct_offset": [],
        }

        with tempfile.TemporaryDirectory() as temp_dir:
            preprocessor_dir = Path(temp_dir) / "ida_preprocessor_scripts"
            session = AsyncMock()
            (preprocessor_dir / "prompt").mkdir(parents=True, exist_ok=True)
            (preprocessor_dir / "prompt" / "call_llm_decompile.md").write_text(
                "{symbol_name_list}",
                encoding="utf-8",
            )
            _write_yaml(
                preprocessor_dir / "references" / "reference.yaml",
                {
                    "func_name": target_detail_payload["func_name"],
                    "disasm_code": "call    sub_180222200",
                    "procedure": target_detail_payload["procedure"],
                },
            )

            async def _session_call_tool(*, name, arguments):
                self.assertEqual("py_eval", name)
                code = arguments["code"]
                if "candidate_names =" in code:
                    return _py_eval_payload(
                        [
                            {
                                "name": target_detail_payload["func_name"],
                                "func_va": target_detail_payload["func_va"],
                            }
                        ]
                    )
                if "'disasm_code': get_disasm(func_start)" in code:
                    return _py_eval_payload(target_detail_payload)
                raise AssertionError(f"unexpected py_eval code: {code}")

            session.call_tool.side_effect = _session_call_tool

            with patch.object(
                ida_analyze_util,
                "_get_preprocessor_scripts_dir",
                return_value=preprocessor_dir,
            ), patch.object(
                ida_analyze_util,
                "create_openai_client",
                return_value=object(),
                create=True,
            ), patch.object(
                ida_analyze_util,
                "preprocess_func_sig_via_mcp",
                AsyncMock(return_value=None),
            ), patch.object(
                ida_analyze_util,
                "_resolve_direct_call_target_via_mcp",
                AsyncMock(return_value="0x180123450"),
            ) as mock_resolve_direct_call_target, patch.object(
                ida_analyze_util,
                "_get_func_basic_info_via_mcp",
                AsyncMock(
                    return_value={
                        "func_va": "0x180123450",
                        "func_rva": "0x123450",
                        "func_size": "0x40",
                    }
                ),
            ), patch.object(
                ida_analyze_util,
                "preprocess_gen_func_sig_via_mcp",
                AsyncMock(return_value={"func_sig": "40 53"}),
            ), patch.object(
                ida_analyze_util,
                "call_llm_decompile",
                create=True,
                new_callable=AsyncMock,
                return_value=normalized_payload,
            ) as mock_call_llm_decompile, patch.object(
                ida_analyze_util,
                "write_func_yaml",
            ) as mock_write_func_yaml, patch.object(
                ida_analyze_util,
                "_rename_func_in_ida",
                AsyncMock(return_value=None),
            ):
                result = await ida_analyze_util.preprocess_common_skill(
                    session=session,
                    expected_outputs=[output_path],
                    old_yaml_map={},
                    new_binary_dir="/tmp",
                    platform="windows",
                    image_base=0x180000000,
                    func_names=[func_name],
                    generate_yaml_desired_fields=[(func_name, ["func_name", "func_va"])],
                    llm_decompile_specs=[
                        (
                            func_name,
                            "prompt/call_llm_decompile.md",
                            "references/reference.yaml",
                        )
                    ],
                    llm_config={
                        "model": "gpt-4.1-mini",
                        "api_key": "test-api-key",
                    },
                    debug=True,
                )

        self.assertTrue(result)
        mock_call_llm_decompile.assert_awaited_once()
        mock_resolve_direct_call_target.assert_awaited_once_with(
            session,
            "0x180777700",
            debug=True,
        )
        mock_write_func_yaml.assert_called_once()
        written_payload = mock_write_func_yaml.call_args.args[1]
        self.assertEqual(func_name, written_payload["func_name"])
        self.assertEqual("0x180123450", written_payload["func_va"])
        self.assertNotIn("vtable_name", written_payload)

    async def test_preprocess_common_skill_uses_found_funcptr_to_generate_func_yaml(
        self,
    ) -> None:
        func_name = "CLoopModeGame_OnClientPollNetworking"
        output_paths = [f"/tmp/{func_name}.windows.yaml"]
        target_detail_payload = {
            "func_name": "CLoopModeGame_RegisterEventMapInternal",
            "func_va": "0x180555500",
            "disasm_code": "lea     rdx, sub_15BC910",
            "procedure": "v40 = sub_15BC910;",
        }
        normalized_payload = {
            "found_vcall": [],
            "found_call": [],
            "found_funcptr": [
                {
                    "insn_va": "0x180666600",
                    "insn_disasm": "lea     rdx, sub_15BC910",
                    "funcptr_name": func_name,
                }
            ],
            "found_gv": [],
            "found_struct_offset": [],
        }

        async def _fake_preprocess_direct_func_sig_via_mcp(**kwargs):
            return {
                "func_name": kwargs["func_name"],
                "func_va": str(kwargs["direct_func_va"]).strip().lower(),
            }

        with tempfile.TemporaryDirectory() as temp_dir:
            preprocessor_dir = Path(temp_dir) / "ida_preprocessor_scripts"
            (preprocessor_dir / "prompt").mkdir(parents=True, exist_ok=True)
            (preprocessor_dir / "prompt" / "call_llm_decompile.md").write_text(
                "{symbol_name_list}",
                encoding="utf-8",
            )
            _write_yaml(
                preprocessor_dir / "references" / "reference.yaml",
                {
                    "func_name": target_detail_payload["func_name"],
                    "disasm_code": target_detail_payload["disasm_code"],
                    "procedure": target_detail_payload["procedure"],
                },
            )

            with patch.object(
                ida_analyze_util,
                "_get_preprocessor_scripts_dir",
                return_value=preprocessor_dir,
            ), patch.object(
                ida_analyze_util,
                "create_openai_client",
                return_value=object(),
                create=True,
            ), patch.object(
                ida_analyze_util,
                "preprocess_func_sig_via_mcp",
                AsyncMock(return_value=None),
            ), patch.object(
                ida_analyze_util,
                "_load_llm_decompile_target_detail_via_mcp",
                AsyncMock(return_value=target_detail_payload),
            ), patch.object(
                ida_analyze_util,
                "_resolve_direct_funcptr_target_via_mcp",
                AsyncMock(return_value="0x180123450"),
                create=True,
            ) as mock_resolve_direct_funcptr_target, patch.object(
                ida_analyze_util,
                "_preprocess_direct_func_sig_via_mcp",
                AsyncMock(side_effect=_fake_preprocess_direct_func_sig_via_mcp),
            ), patch.object(
                ida_analyze_util,
                "call_llm_decompile",
                create=True,
                new_callable=AsyncMock,
                return_value=normalized_payload,
            ), patch.object(
                ida_analyze_util,
                "write_func_yaml",
            ) as mock_write_func_yaml, patch.object(
                ida_analyze_util,
                "_rename_func_in_ida",
                AsyncMock(return_value=None),
            ):
                result = await ida_analyze_util.preprocess_common_skill(
                    session="session",
                    expected_outputs=output_paths,
                    old_yaml_map={},
                    new_binary_dir="/tmp",
                    platform="windows",
                    image_base=0x180000000,
                    func_names=[func_name],
                    generate_yaml_desired_fields=[
                        (func_name, ["func_name", "func_va"]),
                    ],
                    llm_decompile_specs=[
                        (
                            func_name,
                            "prompt/call_llm_decompile.md",
                            "references/reference.yaml",
                        ),
                    ],
                    llm_config={
                        "model": "gpt-4.1-mini",
                        "api_key": "test-api-key",
                    },
                    debug=True,
                )

        self.assertTrue(result)
        mock_resolve_direct_funcptr_target.assert_awaited_once_with(
            "session",
            "0x180666600",
            debug=True,
        )
        self.assertEqual("0x180123450", mock_write_func_yaml.call_args.args[1]["func_va"])

    async def test_preprocess_common_skill_prefers_found_call_over_found_funcptr(
        self,
    ) -> None:
        func_name = "CLoopModeGame_OnClientPollNetworking"
        output_path = f"/tmp/{func_name}.windows.yaml"
        target_detail_payload = {
            "func_name": "CLoopModeGame_RegisterEventMapInternal",
            "func_va": "0x180555500",
            "disasm_code": "call    sub_180111111\nlea     rdx, sub_15BC910",
            "procedure": "sub_180111111(); v40 = sub_15BC910;",
        }
        normalized_payload = {
            "found_vcall": [],
            "found_call": [
                {
                    "insn_va": "0x180777700",
                    "insn_disasm": "call    sub_180111111",
                    "func_name": func_name,
                }
            ],
            "found_funcptr": [
                {
                    "insn_va": "0x180666600",
                    "insn_disasm": "lea     rdx, sub_15BC910",
                    "funcptr_name": func_name,
                }
            ],
            "found_gv": [],
            "found_struct_offset": [],
        }

        async def _fake_preprocess_direct_func_sig_via_mcp(**kwargs):
            return {
                "func_name": kwargs["func_name"],
                "func_va": str(kwargs["direct_func_va"]).strip().lower(),
            }

        with tempfile.TemporaryDirectory() as temp_dir:
            preprocessor_dir = Path(temp_dir) / "ida_preprocessor_scripts"
            (preprocessor_dir / "prompt").mkdir(parents=True, exist_ok=True)
            (preprocessor_dir / "prompt" / "call_llm_decompile.md").write_text(
                "{symbol_name_list}",
                encoding="utf-8",
            )
            _write_yaml(
                preprocessor_dir / "references" / "reference.yaml",
                {
                    "func_name": target_detail_payload["func_name"],
                    "disasm_code": target_detail_payload["disasm_code"],
                    "procedure": target_detail_payload["procedure"],
                },
            )

            with patch.object(
                ida_analyze_util,
                "_get_preprocessor_scripts_dir",
                return_value=preprocessor_dir,
            ), patch.object(
                ida_analyze_util,
                "create_openai_client",
                return_value=object(),
                create=True,
            ), patch.object(
                ida_analyze_util,
                "preprocess_func_sig_via_mcp",
                AsyncMock(return_value=None),
            ), patch.object(
                ida_analyze_util,
                "_load_llm_decompile_target_detail_via_mcp",
                AsyncMock(return_value=target_detail_payload),
            ), patch.object(
                ida_analyze_util,
                "_resolve_direct_call_target_via_mcp",
                AsyncMock(return_value="0x180111111"),
                create=True,
            ) as mock_resolve_direct_call_target, patch.object(
                ida_analyze_util,
                "_resolve_direct_funcptr_target_via_mcp",
                AsyncMock(return_value="0x180222222"),
                create=True,
            ) as mock_resolve_direct_funcptr_target, patch.object(
                ida_analyze_util,
                "_preprocess_direct_func_sig_via_mcp",
                AsyncMock(side_effect=_fake_preprocess_direct_func_sig_via_mcp),
            ), patch.object(
                ida_analyze_util,
                "call_llm_decompile",
                create=True,
                new_callable=AsyncMock,
                return_value=normalized_payload,
            ), patch.object(
                ida_analyze_util,
                "write_func_yaml",
            ) as mock_write_func_yaml, patch.object(
                ida_analyze_util,
                "_rename_func_in_ida",
                AsyncMock(return_value=None),
            ):
                result = await ida_analyze_util.preprocess_common_skill(
                    session="session",
                    expected_outputs=[output_path],
                    old_yaml_map={},
                    new_binary_dir="/tmp",
                    platform="windows",
                    image_base=0x180000000,
                    func_names=[func_name],
                    generate_yaml_desired_fields=[
                        (func_name, ["func_name", "func_va"]),
                    ],
                    llm_decompile_specs=[
                        (
                            func_name,
                            "prompt/call_llm_decompile.md",
                            "references/reference.yaml",
                        ),
                    ],
                    llm_config={
                        "model": "gpt-4.1-mini",
                        "api_key": "test-api-key",
                    },
                    debug=True,
                )

        self.assertTrue(result)
        mock_resolve_direct_call_target.assert_awaited_once_with(
            "session",
            "0x180777700",
            debug=True,
        )
        mock_resolve_direct_funcptr_target.assert_not_awaited()
        mock_write_func_yaml.assert_called_once()
        self.assertEqual("0x180111111", mock_write_func_yaml.call_args.args[1]["func_va"])

    async def test_preprocess_common_skill_skips_found_funcptr_when_resolver_is_non_unique(
        self,
    ) -> None:
        func_name = "CLoopModeGame_OnClientPollNetworking"
        output_path = f"/tmp/{func_name}.windows.yaml"
        target_detail_payload = {
            "func_name": "CLoopModeGame_RegisterEventMapInternal",
            "func_va": "0x180555500",
            "disasm_code": "lea     rdx, sub_15BC910",
            "procedure": "v40 = sub_15BC910;",
        }
        normalized_payload = {
            "found_vcall": [],
            "found_call": [],
            "found_funcptr": [
                {
                    "insn_va": "0x180666600",
                    "insn_disasm": "lea     rdx, sub_15BC910",
                    "funcptr_name": func_name,
                }
            ],
            "found_gv": [],
            "found_struct_offset": [],
        }

        with tempfile.TemporaryDirectory() as temp_dir:
            preprocessor_dir = Path(temp_dir) / "ida_preprocessor_scripts"
            (preprocessor_dir / "prompt").mkdir(parents=True, exist_ok=True)
            (preprocessor_dir / "prompt" / "call_llm_decompile.md").write_text(
                "{symbol_name_list}",
                encoding="utf-8",
            )
            _write_yaml(
                preprocessor_dir / "references" / "reference.yaml",
                {
                    "func_name": target_detail_payload["func_name"],
                    "disasm_code": target_detail_payload["disasm_code"],
                    "procedure": target_detail_payload["procedure"],
                },
            )

            with patch.object(
                ida_analyze_util,
                "_get_preprocessor_scripts_dir",
                return_value=preprocessor_dir,
            ), patch.object(
                ida_analyze_util,
                "create_openai_client",
                return_value=object(),
                create=True,
            ), patch.object(
                ida_analyze_util,
                "preprocess_func_sig_via_mcp",
                AsyncMock(return_value=None),
            ), patch.object(
                ida_analyze_util,
                "_load_llm_decompile_target_detail_via_mcp",
                AsyncMock(return_value=target_detail_payload),
            ), patch.object(
                ida_analyze_util,
                "_resolve_direct_funcptr_target_via_mcp",
                AsyncMock(return_value=None),
                create=True,
            ) as mock_resolve_direct_funcptr_target, patch.object(
                ida_analyze_util,
                "_preprocess_direct_func_sig_via_mcp",
                AsyncMock(),
            ) as mock_preprocess_direct_func_sig_via_mcp, patch.object(
                ida_analyze_util,
                "call_llm_decompile",
                create=True,
                new_callable=AsyncMock,
                return_value=normalized_payload,
            ), patch.object(
                ida_analyze_util,
                "write_func_yaml",
            ) as mock_write_func_yaml, patch.object(
                ida_analyze_util,
                "_rename_func_in_ida",
                AsyncMock(return_value=None),
            ):
                result = await ida_analyze_util.preprocess_common_skill(
                    session="session",
                    expected_outputs=[output_path],
                    old_yaml_map={},
                    new_binary_dir="/tmp",
                    platform="windows",
                    image_base=0x180000000,
                    func_names=[func_name],
                    generate_yaml_desired_fields=[
                        (func_name, ["func_name", "func_va"]),
                    ],
                    llm_decompile_specs=[
                        (
                            func_name,
                            "prompt/call_llm_decompile.md",
                            "references/reference.yaml",
                        ),
                    ],
                    llm_config={
                        "model": "gpt-4.1-mini",
                        "api_key": "test-api-key",
                    },
                    debug=True,
                )

        self.assertFalse(result)
        mock_resolve_direct_funcptr_target.assert_awaited_once_with(
            "session",
            "0x180666600",
            debug=True,
        )
        mock_preprocess_direct_func_sig_via_mcp.assert_not_awaited()
        mock_write_func_yaml.assert_not_called()

    async def test_preprocess_common_skill_skips_found_funcptr_when_vfunc_sig_required(
        self,
    ) -> None:
        func_name = "CBaseEntity_GetHammerUniqueId"
        output_path = f"/tmp/{func_name}.windows.yaml"
        target_detail_payload = {
            "func_name": "CBaseEntity_Spawn",
            "func_va": "0x180555500",
            "disasm_code": "lea     rdx, sub_15BC910\ncall    qword ptr [rax+370h]",
            "procedure": "v40 = sub_15BC910; return this->vfptr[110](this);",
        }
        normalized_payload = {
            "found_vcall": [
                {
                    "insn_va": "0x18016742cf",
                    "insn_disasm": "call    qword ptr [rax+370h]",
                    "vfunc_offset": "0x370",
                    "func_name": func_name,
                }
            ],
            "found_call": [],
            "found_funcptr": [
                {
                    "insn_va": "0x180666600",
                    "insn_disasm": "lea     rdx, sub_15BC910",
                    "funcptr_name": func_name,
                }
            ],
            "found_gv": [],
            "found_struct_offset": [],
        }

        async def _fake_preprocess_direct_func_sig_via_mcp(**kwargs):
            if kwargs.get("direct_vcall_inst_va"):
                return {
                    "func_name": kwargs["func_name"],
                    "func_va": "0x1809b8db0",
                    "vfunc_sig": "FF 90 70 03 00 00",
                    "vfunc_sig_max_match": 10,
                    "vfunc_offset": "0x370",
                    "vfunc_index": 110,
                }
            if kwargs.get("direct_func_va"):
                return {
                    "func_name": kwargs["func_name"],
                    "func_va": str(kwargs["direct_func_va"]).strip().lower(),
                }
            return None

        with tempfile.TemporaryDirectory() as temp_dir:
            preprocessor_dir = Path(temp_dir) / "ida_preprocessor_scripts"
            (preprocessor_dir / "prompt").mkdir(parents=True, exist_ok=True)
            (preprocessor_dir / "prompt" / "call_llm_decompile.md").write_text(
                "{symbol_name_list}",
                encoding="utf-8",
            )
            _write_yaml(
                preprocessor_dir / "references" / "reference.yaml",
                {
                    "func_name": target_detail_payload["func_name"],
                    "disasm_code": target_detail_payload["disasm_code"],
                    "procedure": target_detail_payload["procedure"],
                },
            )

            with patch.object(
                ida_analyze_util,
                "_get_preprocessor_scripts_dir",
                return_value=preprocessor_dir,
            ), patch.object(
                ida_analyze_util,
                "create_openai_client",
                return_value=object(),
                create=True,
            ), patch.object(
                ida_analyze_util,
                "preprocess_func_sig_via_mcp",
                AsyncMock(return_value=None),
            ), patch.object(
                ida_analyze_util,
                "_load_llm_decompile_target_detail_via_mcp",
                AsyncMock(return_value=target_detail_payload),
            ), patch.object(
                ida_analyze_util,
                "_resolve_direct_funcptr_target_via_mcp",
                AsyncMock(return_value="0x180123450"),
                create=True,
            ) as mock_resolve_direct_funcptr_target, patch.object(
                ida_analyze_util,
                "_preprocess_direct_func_sig_via_mcp",
                AsyncMock(side_effect=_fake_preprocess_direct_func_sig_via_mcp),
            ) as mock_preprocess_direct_func_sig_via_mcp, patch.object(
                ida_analyze_util,
                "call_llm_decompile",
                create=True,
                new_callable=AsyncMock,
                return_value=normalized_payload,
            ), patch.object(
                ida_analyze_util,
                "write_func_yaml",
            ) as mock_write_func_yaml, patch.object(
                ida_analyze_util,
                "_rename_func_in_ida",
                AsyncMock(return_value=None),
            ):
                result = await ida_analyze_util.preprocess_common_skill(
                    session="session",
                    expected_outputs=[output_path],
                    old_yaml_map={},
                    new_binary_dir="/tmp",
                    platform="windows",
                    image_base=0x180000000,
                    func_names=[func_name],
                    func_vtable_relations=[(func_name, "CBaseEntity")],
                    generate_yaml_desired_fields=[
                        (
                            func_name,
                            [
                                "func_name",
                                "func_va",
                                "vfunc_sig",
                                "vfunc_sig_max_match:10",
                                "vtable_name",
                                "vfunc_offset",
                                "vfunc_index",
                            ],
                        )
                    ],
                    llm_decompile_specs=[
                        (
                            func_name,
                            "prompt/call_llm_decompile.md",
                            "references/reference.yaml",
                        ),
                    ],
                    llm_config={
                        "model": "gpt-4.1-mini",
                        "api_key": "test-api-key",
                    },
                    debug=True,
                )

        self.assertTrue(result)
        mock_resolve_direct_funcptr_target.assert_not_awaited()
        mock_preprocess_direct_func_sig_via_mcp.assert_awaited_once()
        preprocess_kwargs = mock_preprocess_direct_func_sig_via_mcp.await_args.kwargs
        self.assertEqual("0x18016742cf", preprocess_kwargs["direct_vcall_inst_va"])
        self.assertEqual("0x370", preprocess_kwargs["direct_vfunc_offset"])
        self.assertNotIn("direct_func_va", preprocess_kwargs)
        mock_write_func_yaml.assert_called_once()
        written_payload = mock_write_func_yaml.call_args.args[1]
        self.assertEqual(func_name, written_payload["func_name"])
        self.assertEqual("FF 90 70 03 00 00", written_payload["vfunc_sig"])
        self.assertEqual(10, written_payload["vfunc_sig_max_match"])
        self.assertEqual("CBaseEntity", written_payload["vtable_name"])

    async def test_preprocess_common_skill_skips_found_call_when_vfunc_sig_required(
        self,
    ) -> None:
        func_name = "CBaseEntity_GetHammerUniqueId"
        output_path = f"/tmp/{func_name}.windows.yaml"
        target_detail_payload = {
            "func_name": "CBaseEntity_Spawn",
            "func_va": "0x180555500",
            "disasm_code": "call    sub_180111111\ncall    qword ptr [rax+370h]",
            "procedure": "sub_180111111(); return this->vfptr[110](this);",
        }
        normalized_payload = {
            "found_vcall": [
                {
                    "insn_va": "0x18016742cf",
                    "insn_disasm": "call    qword ptr [rax+370h]",
                    "vfunc_offset": "0x370",
                    "func_name": func_name,
                }
            ],
            "found_call": [
                {
                    "insn_va": "0x180777700",
                    "insn_disasm": "call    sub_180111111",
                    "func_name": func_name,
                }
            ],
            "found_funcptr": [],
            "found_gv": [],
            "found_struct_offset": [],
        }

        async def _fake_preprocess_direct_func_sig_via_mcp(**kwargs):
            if kwargs.get("direct_vcall_inst_va"):
                return {
                    "func_name": kwargs["func_name"],
                    "func_va": "0x1809b8db0",
                    "vfunc_sig": "FF 90 70 03 00 00",
                    "vfunc_sig_max_match": 10,
                    "vfunc_offset": "0x370",
                    "vfunc_index": 110,
                }
            if kwargs.get("direct_func_va"):
                return {
                    "func_name": kwargs["func_name"],
                    "func_va": str(kwargs["direct_func_va"]).strip().lower(),
                }
            return None

        with tempfile.TemporaryDirectory() as temp_dir:
            preprocessor_dir = Path(temp_dir) / "ida_preprocessor_scripts"
            (preprocessor_dir / "prompt").mkdir(parents=True, exist_ok=True)
            (preprocessor_dir / "prompt" / "call_llm_decompile.md").write_text(
                "{symbol_name_list}",
                encoding="utf-8",
            )
            _write_yaml(
                preprocessor_dir / "references" / "reference.yaml",
                {
                    "func_name": target_detail_payload["func_name"],
                    "disasm_code": target_detail_payload["disasm_code"],
                    "procedure": target_detail_payload["procedure"],
                },
            )

            with patch.object(
                ida_analyze_util,
                "_get_preprocessor_scripts_dir",
                return_value=preprocessor_dir,
            ), patch.object(
                ida_analyze_util,
                "create_openai_client",
                return_value=object(),
                create=True,
            ), patch.object(
                ida_analyze_util,
                "preprocess_func_sig_via_mcp",
                AsyncMock(return_value=None),
            ), patch.object(
                ida_analyze_util,
                "_load_llm_decompile_target_detail_via_mcp",
                AsyncMock(return_value=target_detail_payload),
            ), patch.object(
                ida_analyze_util,
                "_resolve_direct_call_target_via_mcp",
                AsyncMock(return_value="0x180111111"),
                create=True,
            ) as mock_resolve_direct_call_target, patch.object(
                ida_analyze_util,
                "_preprocess_direct_func_sig_via_mcp",
                AsyncMock(side_effect=_fake_preprocess_direct_func_sig_via_mcp),
            ) as mock_preprocess_direct_func_sig_via_mcp, patch.object(
                ida_analyze_util,
                "call_llm_decompile",
                create=True,
                new_callable=AsyncMock,
                return_value=normalized_payload,
            ), patch.object(
                ida_analyze_util,
                "write_func_yaml",
            ) as mock_write_func_yaml, patch.object(
                ida_analyze_util,
                "_rename_func_in_ida",
                AsyncMock(return_value=None),
            ):
                result = await ida_analyze_util.preprocess_common_skill(
                    session="session",
                    expected_outputs=[output_path],
                    old_yaml_map={},
                    new_binary_dir="/tmp",
                    platform="windows",
                    image_base=0x180000000,
                    func_names=[func_name],
                    func_vtable_relations=[(func_name, "CBaseEntity")],
                    generate_yaml_desired_fields=[
                        (
                            func_name,
                            [
                                "func_name",
                                "func_va",
                                "vfunc_sig",
                                "vfunc_sig_max_match:10",
                                "vtable_name",
                                "vfunc_offset",
                                "vfunc_index",
                            ],
                        )
                    ],
                    llm_decompile_specs=[
                        (
                            func_name,
                            "prompt/call_llm_decompile.md",
                            "references/reference.yaml",
                        ),
                    ],
                    llm_config={
                        "model": "gpt-4.1-mini",
                        "api_key": "test-api-key",
                    },
                    debug=True,
                )

        self.assertTrue(result)
        mock_resolve_direct_call_target.assert_not_awaited()
        mock_preprocess_direct_func_sig_via_mcp.assert_awaited_once()
        preprocess_kwargs = mock_preprocess_direct_func_sig_via_mcp.await_args.kwargs
        self.assertEqual("0x18016742cf", preprocess_kwargs["direct_vcall_inst_va"])
        self.assertEqual("0x370", preprocess_kwargs["direct_vfunc_offset"])
        self.assertNotIn("direct_func_va", preprocess_kwargs)
        mock_write_func_yaml.assert_called_once()
        written_payload = mock_write_func_yaml.call_args.args[1]
        self.assertEqual(func_name, written_payload["func_name"])
        self.assertEqual("FF 90 70 03 00 00", written_payload["vfunc_sig"])
        self.assertEqual(10, written_payload["vfunc_sig_max_match"])
        self.assertEqual("CBaseEntity", written_payload["vtable_name"])

    async def test_preprocess_common_skill_generates_vfunc_sig_from_vcall_even_when_vtable_available(
        self,
    ) -> None:
        func_name = "CBaseEntity_GetHammerUniqueId"
        output_path = f"/tmp/{func_name}.windows.yaml"
        target_detail_payload = {
            "func_name": "CBaseEntity_Spawn",
            "func_va": "0x180555500",
            "disasm_code": "call    qword ptr [rax+370h]",
            "procedure": "return this->vfptr[110](this);",
        }
        normalized_payload = {
            "found_vcall": [
                {
                    "insn_va": "0x18016742cf",
                    "insn_disasm": "call    qword ptr [rax+370h]",
                    "vfunc_offset": "0x370",
                    "func_name": func_name,
                }
            ],
            "found_call": [],
            "found_gv": [],
            "found_struct_offset": [],
        }

        with tempfile.TemporaryDirectory() as temp_dir:
            preprocessor_dir = Path(temp_dir) / "ida_preprocessor_scripts"
            session = AsyncMock()
            (preprocessor_dir / "prompt").mkdir(parents=True, exist_ok=True)
            (preprocessor_dir / "prompt" / "call_llm_decompile.md").write_text(
                "{symbol_name_list}",
                encoding="utf-8",
            )
            _write_yaml(
                preprocessor_dir / "references" / "reference.yaml",
                {
                    "func_name": target_detail_payload["func_name"],
                    "disasm_code": target_detail_payload["disasm_code"],
                    "procedure": target_detail_payload["procedure"],
                },
            )

            with patch.object(
                ida_analyze_util,
                "_get_preprocessor_scripts_dir",
                return_value=preprocessor_dir,
            ), patch.object(
                ida_analyze_util,
                "create_openai_client",
                return_value=object(),
                create=True,
            ), patch.object(
                ida_analyze_util,
                "preprocess_func_sig_via_mcp",
                AsyncMock(return_value=None),
            ), patch.object(
                ida_analyze_util,
                "preprocess_vtable_via_mcp",
                AsyncMock(
                    return_value={
                        "vtable_class": "CBaseEntity",
                        "vtable_symbol": "_ZTV11CBaseEntity + 0x10",
                        "vtable_va": "0x18021862e0",
                        "vtable_rva": "0x21862e0",
                        "vtable_size": "0x778",
                        "vtable_numvfunc": 239,
                        "vtable_entries": {110: "0x1809b8db0"},
                    }
                ),
            ), patch.object(
                ida_analyze_util,
                "_get_func_basic_info_via_mcp",
                AsyncMock(
                    return_value={
                        "func_va": "0x1809b8db0",
                        "func_rva": "0x9b8db0",
                        "func_size": "0x3",
                    }
                ),
            ), patch.object(
                ida_analyze_util,
                "preprocess_gen_vfunc_sig_via_mcp",
                AsyncMock(
                    return_value={
                        "vfunc_sig": "FF 90 70 03 00 00",
                        "vfunc_sig_max_match": 10,
                    }
                ),
            ) as mock_preprocess_gen_vfunc_sig, patch.object(
                ida_analyze_util,
                "preprocess_gen_func_sig_via_mcp",
                AsyncMock(return_value=None),
            ) as mock_preprocess_gen_func_sig, patch.object(
                ida_analyze_util,
                "_load_llm_decompile_target_detail_via_mcp",
                AsyncMock(return_value=target_detail_payload),
            ), patch.object(
                ida_analyze_util,
                "call_llm_decompile",
                create=True,
                new_callable=AsyncMock,
                return_value=normalized_payload,
            ), patch.object(
                ida_analyze_util,
                "write_func_yaml",
            ) as mock_write_func_yaml, patch.object(
                ida_analyze_util,
                "_rename_func_in_ida",
                AsyncMock(return_value=None),
            ):
                result = await ida_analyze_util.preprocess_common_skill(
                    session=session,
                    expected_outputs=[output_path],
                    old_yaml_map={},
                    new_binary_dir="/tmp",
                    platform="windows",
                    image_base=0x180000000,
                    func_names=[func_name],
                    func_vtable_relations=[(func_name, "CBaseEntity")],
                    generate_yaml_desired_fields=[
                        (
                            func_name,
                            [
                                "func_name",
                                "vfunc_sig",
                                "vfunc_sig_max_match:10",
                                "vtable_name",
                                "vfunc_offset",
                                "vfunc_index",
                            ],
                        )
                    ],
                    llm_decompile_specs=[
                        (
                            func_name,
                            "prompt/call_llm_decompile.md",
                            "references/reference.yaml",
                        )
                    ],
                    llm_config={
                        "model": "gpt-4.1-mini",
                        "api_key": "test-api-key",
                    },
                    debug=True,
                )

        self.assertTrue(result)
        mock_preprocess_gen_vfunc_sig.assert_awaited_once_with(
            session=session,
            inst_va="0x18016742cf",
            vfunc_offset="0x370",
            max_match_count=10,
            debug=True,
        )
        mock_preprocess_gen_func_sig.assert_not_awaited()
        mock_write_func_yaml.assert_called_once()
        written_payload = mock_write_func_yaml.call_args.args[1]
        self.assertEqual(func_name, written_payload["func_name"])
        self.assertEqual("FF 90 70 03 00 00", written_payload["vfunc_sig"])
        self.assertEqual(10, written_payload["vfunc_sig_max_match"])
        self.assertEqual("CBaseEntity", written_payload["vtable_name"])
        self.assertEqual("0x370", written_payload["vfunc_offset"])
        self.assertEqual(110, written_payload["vfunc_index"])
        self.assertNotIn("func_sig", written_payload)

    async def test_preprocess_common_skill_uses_slot_only_fallback_when_vtable_unavailable(
        self,
    ) -> None:
        func_name = "INetworkMessages_FindNetworkGroup"
        output_path = f"/tmp/{func_name}.windows.yaml"
        target_detail_payload = {
            "func_name": "CNetworkGameClient_RecordEntityBandwidth",
            "func_va": "0x180555500",
            "disasm_code": "call    qword ptr [rax+78h]",
            "procedure": "return this->vfptr[15](this, group);",
        }
        normalized_payload = {
            "found_vcall": [
                {
                    "insn_va": "0x18004ABC3",
                    "insn_disasm": "call    qword ptr [rax+78h]",
                    "vfunc_offset": "0x78",
                    "func_name": func_name,
                },
                {
                    "insn_va": "0x18004AC0A",
                    "insn_disasm": "call    qword ptr [rax+78h]",
                    "vfunc_offset": "0x78",
                    "func_name": func_name,
                },
            ],
            "found_call": [],
            "found_gv": [],
            "found_struct_offset": [],
        }

        with tempfile.TemporaryDirectory() as temp_dir:
            preprocessor_dir = Path(temp_dir) / "ida_preprocessor_scripts"
            session = AsyncMock()
            (preprocessor_dir / "prompt").mkdir(parents=True, exist_ok=True)
            (preprocessor_dir / "prompt" / "call_llm_decompile.md").write_text(
                "{symbol_name_list}",
                encoding="utf-8",
            )
            _write_yaml(
                preprocessor_dir / "references" / "reference.yaml",
                {
                    "func_name": target_detail_payload["func_name"],
                    "disasm_code": "call    qword ptr [rax+78h]",
                    "procedure": target_detail_payload["procedure"],
                },
            )

            async def _session_call_tool(*, name, arguments):
                self.assertEqual("py_eval", name)
                code = arguments["code"]
                if "candidate_names =" in code:
                    return _py_eval_payload(
                        [
                            {
                                "name": target_detail_payload["func_name"],
                                "func_va": target_detail_payload["func_va"],
                            }
                        ]
                    )
                if "'disasm_code': get_disasm(func_start)" in code:
                    return _py_eval_payload(target_detail_payload)
                raise AssertionError(f"unexpected py_eval code: {code}")

            session.call_tool.side_effect = _session_call_tool

            with patch.object(
                ida_analyze_util,
                "_get_preprocessor_scripts_dir",
                return_value=preprocessor_dir,
            ), patch.object(
                ida_analyze_util,
                "create_openai_client",
                return_value=object(),
                create=True,
            ), patch.object(
                ida_analyze_util,
                "preprocess_func_sig_via_mcp",
                AsyncMock(return_value=None),
            ), patch.object(
                ida_analyze_util,
                "preprocess_vtable_via_mcp",
                AsyncMock(return_value=None),
            ), patch.object(
                ida_analyze_util,
                "preprocess_gen_vfunc_sig_via_mcp",
                create=True,
                new_callable=AsyncMock,
                return_value={"vfunc_sig": "FF 90 78 00 00 00 48 8B C8"},
            ) as mock_preprocess_gen_vfunc_sig, patch.object(
                ida_analyze_util,
                "call_llm_decompile",
                create=True,
                new_callable=AsyncMock,
                return_value=normalized_payload,
            ), patch.object(
                ida_analyze_util,
                "write_func_yaml",
            ) as mock_write_func_yaml, patch.object(
                ida_analyze_util,
                "_rename_func_in_ida",
                AsyncMock(return_value=None),
            ):
                result = await ida_analyze_util.preprocess_common_skill(
                    session=session,
                    expected_outputs=[output_path],
                    old_yaml_map={},
                    new_binary_dir="/tmp",
                    platform="windows",
                    image_base=0x180000000,
                    func_names=[func_name],
                    func_vtable_relations=[(func_name, "CNetworkMessages")],
                    generate_yaml_desired_fields=[
                        (
                            func_name,
                            [
                                "func_name",
                                "vfunc_sig",
                                "vfunc_sig_max_match:10",
                                "vtable_name",
                                "vfunc_offset",
                                "vfunc_index",
                            ],
                        )
                    ],
                    llm_decompile_specs=[
                        (
                            func_name,
                            "prompt/call_llm_decompile.md",
                            "references/reference.yaml",
                        )
                    ],
                    llm_config={
                        "model": "gpt-4.1-mini",
                        "api_key": "test-api-key",
                    },
                    debug=True,
                )

        self.assertTrue(result)
        mock_preprocess_gen_vfunc_sig.assert_awaited_once_with(
            session=session,
            inst_va="0x18004abc3",
            vfunc_offset="0x78",
            max_match_count=10,
            debug=True,
        )
        mock_write_func_yaml.assert_called_once()
        written_payload = mock_write_func_yaml.call_args.args[1]
        self.assertEqual(func_name, written_payload["func_name"])
        self.assertEqual("FF 90 78 00 00 00 48 8B C8", written_payload["vfunc_sig"])
        self.assertEqual(10, written_payload["vfunc_sig_max_match"])
        self.assertEqual("CNetworkMessages", written_payload["vtable_name"])
        self.assertEqual("0x78", written_payload["vfunc_offset"])
        self.assertEqual(15, written_payload["vfunc_index"])
        self.assertNotIn("func_va", written_payload)

    async def test_preprocess_common_skill_fails_when_slot_only_vfunc_sig_generation_fails(
        self,
    ) -> None:
        func_name = "INetworkMessages_SetNetworkSerializationContextData"
        output_path = f"/tmp/{func_name}.linux.yaml"
        target_detail_payload = {
            "func_name": "CEntitySystem_Activate",
            "func_va": "0x1D85700",
            "disasm_code": "call    qword ptr [rax+0A8h]",
            "procedure": "return this->vfptr[21](this, ctx);",
        }
        normalized_payload = {
            "found_vcall": [
                {
                    "insn_va": "0x1D859BF",
                    "insn_disasm": "call    qword ptr [rax+0A8h]",
                    "vfunc_offset": "0xA8",
                    "func_name": func_name,
                },
                {
                    "insn_va": "0x1D85A10",
                    "insn_disasm": "call    qword ptr [rax+0A8h]",
                    "vfunc_offset": "0xA8",
                    "func_name": func_name,
                },
            ],
            "found_call": [],
            "found_gv": [],
            "found_struct_offset": [],
        }

        with tempfile.TemporaryDirectory() as temp_dir:
            preprocessor_dir = Path(temp_dir) / "ida_preprocessor_scripts"
            session = AsyncMock()
            (preprocessor_dir / "prompt").mkdir(parents=True, exist_ok=True)
            (preprocessor_dir / "prompt" / "call_llm_decompile.md").write_text(
                "{symbol_name_list}",
                encoding="utf-8",
            )
            _write_yaml(
                preprocessor_dir / "references" / "reference.yaml",
                {
                    "func_name": target_detail_payload["func_name"],
                    "disasm_code": "call    qword ptr [rax+0A8h]",
                    "procedure": target_detail_payload["procedure"],
                },
            )

            async def _session_call_tool(*, name, arguments):
                self.assertEqual("py_eval", name)
                code = arguments["code"]
                if "candidate_names =" in code:
                    return _py_eval_payload(
                        [
                            {
                                "name": target_detail_payload["func_name"],
                                "func_va": target_detail_payload["func_va"],
                            }
                        ]
                    )
                if "'disasm_code': get_disasm(func_start)" in code:
                    return _py_eval_payload(target_detail_payload)
                raise AssertionError(f"unexpected py_eval code: {code}")

            session.call_tool.side_effect = _session_call_tool

            with patch.object(
                ida_analyze_util,
                "_get_preprocessor_scripts_dir",
                return_value=preprocessor_dir,
            ), patch.object(
                ida_analyze_util,
                "create_openai_client",
                return_value=object(),
                create=True,
            ), patch.object(
                ida_analyze_util,
                "preprocess_func_sig_via_mcp",
                AsyncMock(return_value=None),
            ), patch.object(
                ida_analyze_util,
                "preprocess_vtable_via_mcp",
                AsyncMock(return_value=None),
            ), patch.object(
                ida_analyze_util,
                "preprocess_gen_vfunc_sig_via_mcp",
                create=True,
                new_callable=AsyncMock,
                return_value=None,
            ) as mock_preprocess_gen_vfunc_sig, patch.object(
                ida_analyze_util,
                "call_llm_decompile",
                create=True,
                new_callable=AsyncMock,
                return_value=normalized_payload,
            ), patch.object(
                ida_analyze_util,
                "write_func_yaml",
            ) as mock_write_func_yaml, patch.object(
                ida_analyze_util,
                "_rename_func_in_ida",
                AsyncMock(return_value=None),
            ):
                result = await ida_analyze_util.preprocess_common_skill(
                    session=session,
                    expected_outputs=[output_path],
                    old_yaml_map={},
                    new_binary_dir="/tmp",
                    platform="linux",
                    image_base=0,
                    func_names=[func_name],
                    func_vtable_relations=[(func_name, "INetworkMessages")],
                    generate_yaml_desired_fields=[
                        (
                            func_name,
                            [
                                "func_name",
                                "vfunc_sig",
                                "vtable_name",
                                "vfunc_offset",
                                "vfunc_index",
                            ],
                        )
                    ],
                    llm_decompile_specs=[
                        (
                            func_name,
                            "prompt/call_llm_decompile.md",
                            "references/reference.yaml",
                        )
                    ],
                    llm_config={
                        "model": "gpt-4.1-mini",
                        "api_key": "test-api-key",
                    },
                    debug=True,
                )

        self.assertFalse(result)
        self.assertEqual(2, mock_preprocess_gen_vfunc_sig.await_count)
        mock_preprocess_gen_vfunc_sig.assert_has_awaits(
            [
                call(
                    session=session,
                    inst_va="0x1d859bf",
                    vfunc_offset="0xa8",
                    max_match_count=1,
                    debug=True,
                ),
                call(
                    session=session,
                    inst_va="0x1d85a10",
                    vfunc_offset="0xa8",
                    max_match_count=1,
                    debug=True,
                ),
            ]
        )
        mock_write_func_yaml.assert_not_called()


class TestPreprocessFuncSigViaMcpVfuncSigMaxMatch(unittest.IsolatedAsyncioTestCase):
    async def _preprocess_with_vfunc_sig_max_match(self, max_match_count):
        with tempfile.TemporaryDirectory() as temp_dir:
            old_path = Path(temp_dir) / "INetworkMessages_GetLoggingChannel.windows.yaml"
            new_path = Path(temp_dir) / "INetworkMessages_GetLoggingChannel.new.windows.yaml"

            _write_yaml(
                old_path,
                {
                    "func_name": "INetworkMessages_GetLoggingChannel",
                    "vfunc_sig": "FF 90 20 01 00 00",
                    "vfunc_sig_max_match": max_match_count,
                    "vtable_name": "INetworkMessages",
                    "vfunc_offset": "0x120",
                    "vfunc_index": 36,
                    "func_va": "0x180111111",
                },
            )
            _write_yaml(
                Path(temp_dir) / "INetworkMessages_vtable.windows.yaml",
                {
                    "vtable_entries": {
                        36: "0x180222222",
                    }
                },
            )

            session = AsyncMock()

            async def _fake_call_tool(*, name: str, arguments: dict[str, object]):
                if name == "find_bytes":
                    return _FakeCallToolResult(
                        [
                            {
                                "matches": ["0x180012340"],
                                "n": 1,
                            }
                        ]
                    )
                if name == "py_eval":
                    return _py_eval_payload(
                        {
                            "func_va": "0x180222222",
                            "func_size": "0x40",
                        }
                    )
                raise AssertionError(f"unexpected MCP tool: {name}")

            session.call_tool.side_effect = _fake_call_tool

            result = await ida_analyze_util.preprocess_func_sig_via_mcp(
                session=session,
                new_path=str(new_path),
                old_path=str(old_path),
                image_base=0x180000000,
                new_binary_dir=temp_dir,
                platform="windows",
                debug=True,
            )

        return result, session

    async def test_preprocess_func_sig_via_mcp_allows_vfunc_sig_match_count_within_limit(
        self,
    ) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            old_path = Path(temp_dir) / "INetworkMessages_GetLoggingChannel.windows.yaml"
            new_path = Path(temp_dir) / "INetworkMessages_GetLoggingChannel.new.windows.yaml"

            _write_yaml(
                old_path,
                {
                    "func_name": "INetworkMessages_GetLoggingChannel",
                    "vfunc_sig": "FF 90 20 01 00 00",
                    "vfunc_sig_max_match": 10,
                    "vtable_name": "INetworkMessages",
                    "vfunc_offset": "0x120",
                    "vfunc_index": 36,
                    "func_va": "0x180111111",
                },
            )
            _write_yaml(
                Path(temp_dir) / "INetworkMessages_vtable.windows.yaml",
                {
                    "vtable_entries": {
                        36: "0x180222222",
                    }
                },
            )

            session = AsyncMock()

            async def _fake_call_tool(*, name: str, arguments: dict[str, object]):
                if name == "find_bytes":
                    self.assertEqual(
                        ["FF 90 20 01 00 00"],
                        arguments["patterns"],
                    )
                    return _FakeCallToolResult(
                        [
                            {
                                "matches": ["0x180012340", "0x180056780"],
                                "n": 2,
                            }
                        ]
                    )
                if name == "py_eval":
                    return _py_eval_payload(
                        {
                            "func_va": "0x180222222",
                            "func_size": "0x40",
                        }
                    )
                raise AssertionError(f"unexpected MCP tool: {name}")

            session.call_tool.side_effect = _fake_call_tool

            result = await ida_analyze_util.preprocess_func_sig_via_mcp(
                session=session,
                new_path=str(new_path),
                old_path=str(old_path),
                image_base=0x180000000,
                new_binary_dir=temp_dir,
                platform="windows",
                debug=True,
            )

        self.assertIsNotNone(result)
        assert result is not None
        self.assertEqual(10, result["vfunc_sig_max_match"])
        self.assertEqual("FF 90 20 01 00 00", result["vfunc_sig"])
        self.assertEqual(36, result["vfunc_index"])

    async def test_preprocess_func_sig_via_mcp_rejects_invalid_vfunc_sig_max_match(
        self,
    ) -> None:
        invalid_values = [True, 1.5, 0, "abc"]

        for invalid_value in invalid_values:
            with self.subTest(vfunc_sig_max_match=invalid_value):
                result, session = await self._preprocess_with_vfunc_sig_max_match(
                    invalid_value,
                )

                self.assertIsNone(result)
                session.call_tool.assert_not_awaited()


if __name__ == "__main__":
    unittest.main()
