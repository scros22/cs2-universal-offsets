import importlib.util
import unittest
from pathlib import Path
from unittest.mock import AsyncMock, patch

import ida_skill_preprocessor


FLATTENED_SERIALIZERS_SCRIPT_PATH = Path(
    "ida_preprocessor_scripts/"
    "find-CFlattenedSerializers_CreateFieldChangedEventQueue-impl.py"
)
SET_IS_FOR_SERVER_SCRIPT_PATH = Path(
    "ida_preprocessor_scripts/"
    "find-CNetworkMessages_SetIsForServer.py"
)
I_SET_IS_FOR_SERVER_SCRIPT_PATH = Path(
    "ida_preprocessor_scripts/"
    "find-INetworkMessages_SetIsForServer.py"
)
I_GET_LOGGING_CHANNEL_WINDOWS_SCRIPT_PATH = Path(
    "ida_preprocessor_scripts/"
    "find-INetworkMessages_GetLoggingChannel-windows.py"
)
I_GET_LOGGING_CHANNEL_LINUX_SCRIPT_PATH = Path(
    "ida_preprocessor_scripts/"
    "find-INetworkMessages_GetLoggingChannel-linux.py"
)
NETWORK_GROUP_STATS_SCRIPT_PATH = Path(
    "ida_preprocessor_scripts/"
    "find-CNetworkMessages_GetNetworkGroupCount-AND-"
    "CNetworkMessages_GetNetworkGroupName-AND-"
    "CNetworkMessages_GetNetworkGroupColor.py"
)
REALLOCATING_FACTORY_SCRIPT_PATH = Path(
    "ida_preprocessor_scripts/"
    "find-CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem_vtable.py"
)
REALLOCATING_FACTORY_DEALLOCATE_SCRIPT_PATH = Path(
    "ida_preprocessor_scripts/"
    "find-CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem_Deallocate-impl.py"
)
FIND_NETWORK_GROUP_SCRIPT_PATH = Path(
    "ida_preprocessor_scripts/"
    "find-CNetworkMessages_FindNetworkGroup.py"
)
CNETWORK_SERVER_SERVICE_INIT_SCRIPT_PATH = Path(
    "ida_preprocessor_scripts/"
    "find-CNetworkServerService_Init.py"
)
BOT_ADD_COMMAND_HANDLER_SCRIPT_PATH = Path(
    "ida_preprocessor_scripts/"
    "find-BotAdd_CommandHandler.py"
)
SHOW_HUD_HINT_SCRIPT_PATH = Path(
    "ida_preprocessor_scripts/"
    "find-ShowHudHint.py"
)
ON_EVENT_MAP_CALLBACKS_CLIENT_SCRIPT_PATH = Path(
    "ida_preprocessor_scripts/"
    "find-CLoopModeGame_OnEventMapCallbacks-client.py"
)

class _FakeStreamableHttpClient:
    async def __aenter__(self):
        return ("read-stream", "write-stream", None)

    async def __aexit__(self, exc_type, exc, tb):
        return False


class _FakeAsyncClient:
    def __init__(self, *args, **kwargs):
        pass

    async def __aenter__(self):
        return self

    async def __aexit__(self, exc_type, exc, tb):
        return False


class _FakeClientSession:
    def __init__(self, read_stream, write_stream):
        self.read_stream = read_stream
        self.write_stream = write_stream

    async def __aenter__(self):
        return self

    async def __aexit__(self, exc_type, exc, tb):
        return False

    async def initialize(self):
        return None

    async def call_tool(self, name, arguments):
        return {"name": name, "arguments": arguments}


def _load_module(script_path: Path, module_name: str):
    spec = importlib.util.spec_from_file_location(
        module_name,
        script_path,
    )
    module = importlib.util.module_from_spec(spec)
    assert spec.loader is not None
    spec.loader.exec_module(module)
    return module


class TestFindBotAddCommandHandler(unittest.IsolatedAsyncioTestCase):
    async def test_preprocess_skill_forwards_registerconcommand_contract(self) -> None:
        module = _load_module(
            BOT_ADD_COMMAND_HANDLER_SCRIPT_PATH,
            "find_BotAdd_CommandHandler",
        )
        mock_preprocess_registerconcommand_skill = AsyncMock(return_value=True)
        expected_generate_yaml_desired_fields = [
            (
                "BotAdd_CommandHandler",
                [
                    "func_name",
                    "func_sig",
                    "func_va",
                    "func_rva",
                    "func_size",
                ],
            )
        ]

        with patch.object(
            module,
            "preprocess_registerconcommand_skill",
            mock_preprocess_registerconcommand_skill,
        ):
            result = await module.preprocess_skill(
                session="session",
                skill_name="skill",
                expected_outputs=["out.yaml"],
                old_yaml_map={"k": "v"},
                new_binary_dir="bin_dir",
                platform="linux",
                image_base=0x400000,
                debug=True,
            )

        self.assertTrue(result)
        mock_preprocess_registerconcommand_skill.assert_awaited_once_with(
            session="session",
            expected_outputs=["out.yaml"],
            new_binary_dir="bin_dir",
            platform="linux",
            image_base=0x400000,
            target_name="BotAdd_CommandHandler",
            generate_yaml_desired_fields=expected_generate_yaml_desired_fields,
            command_name="bot_add",
            help_string="bot_add <t|ct> <type> <difficulty> <name> - Adds a bot matching the given criteria.",
            search_window_before_call=96,
            search_window_after_xref=96,
            debug=True,
        )


class TestFindShowHudHint(unittest.IsolatedAsyncioTestCase):
    async def test_preprocess_skill_forwards_define_inputfunc_contract(self) -> None:
        module = _load_module(
            SHOW_HUD_HINT_SCRIPT_PATH,
            "find_ShowHudHint",
        )
        mock_helper = AsyncMock(return_value=True)
        expected_generate_yaml_desired_fields = [
            (
                "ShowHudHint",
                ["func_name", "func_va", "func_rva", "func_size", "func_sig"],
            )
        ]

        with patch.object(
            module,
            "preprocess_define_inputfunc_skill",
            mock_helper,
            create=True,
        ):
            result = await module.preprocess_skill(
                session="session",
                skill_name="skill",
                expected_outputs=["out.yaml"],
                old_yaml_map={"k": "v"},
                new_binary_dir="bin_dir",
                platform="windows",
                image_base=0x180000000,
                debug=True,
            )

        self.assertTrue(result)
        mock_helper.assert_awaited_once_with(
            session="session",
            expected_outputs=["out.yaml"],
            platform="windows",
            image_base=0x180000000,
            target_name="ShowHudHint",
            input_name="ShowHudHint",
            generate_yaml_desired_fields=expected_generate_yaml_desired_fields,
            handler_ptr_offset=0x10,
            allowed_segment_names=(".data",),
            rename_to="ShowHudHint",
            debug=True,
        )


class TestFindCLoopModeGameOnEventMapCallbacksClient(
    unittest.IsolatedAsyncioTestCase
):
    async def test_preprocess_skill_forwards_register_event_listener_contract(
        self,
    ) -> None:
        module = _load_module(
            ON_EVENT_MAP_CALLBACKS_CLIENT_SCRIPT_PATH,
            "find_CLoopModeGame_OnEventMapCallbacks_client",
        )
        mock_helper = AsyncMock(return_value=True)

        with patch.object(
            module,
            "preprocess_register_event_listener_abstract_skill",
            mock_helper,
            create=True,
        ):
            result = await module.preprocess_skill(
                session="session",
                skill_name="skill",
                expected_outputs=["out.yaml"],
                old_yaml_map={"k": "v"},
                new_binary_dir="bin_dir",
                platform="windows",
                image_base=0x180000000,
                debug=True,
            )

        self.assertTrue(result)
        mock_helper.assert_awaited_once_with(
            session="session",
            expected_outputs=["out.yaml"],
            new_binary_dir="bin_dir",
            platform="windows",
            image_base=0x180000000,
            source_yaml_stem=module.SOURCE_YAML_STEM,
            register_func_target_name=module.REGISTER_FUNC_TARGET_NAME,
            anchor_event_name=module.ANCHOR_EVENT_NAME,
            target_specs=module.TARGET_SPECS,
            generate_yaml_desired_fields=module.GENERATE_YAML_DESIRED_FIELDS,
            search_window_after_anchor=module.SEARCH_WINDOW_AFTER_ANCHOR,
            search_window_before_call=module.SEARCH_WINDOW_BEFORE_CALL,
            debug=True,
        )


class TestFindCFlattenedSerializersCreateFieldChangedEventQueueImpl(
    unittest.IsolatedAsyncioTestCase
):
    async def test_preprocess_skill_forwards_expected_inherit_vfuncs(self) -> None:
        module = _load_module(
            FLATTENED_SERIALIZERS_SCRIPT_PATH,
            "find_CFlattenedSerializers_CreateFieldChangedEventQueue_impl",
        )
        mock_preprocess_common_skill = AsyncMock(return_value=True)
        expected_inherit_vfuncs = [
            (
                "CFlattenedSerializers_CreateFieldChangedEventQueue",
                "CFlattenedSerializers",
                "../server/CFlattenedSerializers_CreateFieldChangedEventQueue",
                True,
            )
        ]
        expected_generate_yaml_desired_fields = [
            (
                "CFlattenedSerializers_CreateFieldChangedEventQueue",
                [
                    "func_name",
                    "func_va",
                    "func_rva",
                    "func_size",
                    "func_sig",
                    "vtable_name",
                    "vfunc_offset",
                    "vfunc_index",
                ],
            )
        ]

        with patch.object(
            module,
            "preprocess_common_skill",
            mock_preprocess_common_skill,
        ):
            result = await module.preprocess_skill(
                session="session",
                skill_name="skill",
                expected_outputs=["out.yaml"],
                old_yaml_map={"k": "v"},
                new_binary_dir="bin_dir",
                platform="windows",
                image_base=0x180000000,
                debug=True,
            )

        self.assertTrue(result)
        mock_preprocess_common_skill.assert_awaited_once_with(
            session="session",
            expected_outputs=["out.yaml"],
            old_yaml_map={"k": "v"},
            new_binary_dir="bin_dir",
            platform="windows",
            image_base=0x180000000,
            inherit_vfuncs=expected_inherit_vfuncs,
            generate_yaml_desired_fields=expected_generate_yaml_desired_fields,
            debug=True,
        )


class TestFindCNetworkMessagesSetIsForServerImpl(unittest.IsolatedAsyncioTestCase):
    async def test_preprocess_skill_forwards_expected_inherit_vfuncs(self) -> None:
        module = _load_module(
            SET_IS_FOR_SERVER_SCRIPT_PATH,
            "find_CNetworkMessages_SetIsForServer_impl",
        )
        mock_preprocess_common_skill = AsyncMock(return_value=True)
        expected_inherit_vfuncs = [
            (
                "CNetworkMessages_SetIsForServer",
                "CNetworkMessages",
                "../engine/INetworkMessages_SetIsForServer",
                True,
            )
        ]
        expected_generate_yaml_desired_fields = [
            (
                "CNetworkMessages_SetIsForServer",
                [
                    "func_name",
                    "func_va",
                    "func_rva",
                    "func_size",
                    "func_sig",
                    "vtable_name",
                    "vfunc_offset",
                    "vfunc_index",
                ],
            )
        ]

        with patch.object(
            module,
            "preprocess_common_skill",
            mock_preprocess_common_skill,
        ):
            result = await module.preprocess_skill(
                session="session",
                skill_name="skill",
                expected_outputs=["out.yaml"],
                old_yaml_map={"k": "v"},
                new_binary_dir="bin_dir",
                platform="windows",
                image_base=0x180000000,
                debug=True,
            )

        self.assertTrue(result)
        mock_preprocess_common_skill.assert_awaited_once_with(
            session="session",
            expected_outputs=["out.yaml"],
            old_yaml_map={"k": "v"},
            new_binary_dir="bin_dir",
            platform="windows",
            image_base=0x180000000,
            inherit_vfuncs=expected_inherit_vfuncs,
            generate_yaml_desired_fields=expected_generate_yaml_desired_fields,
            debug=True,
        )


class TestFindINetworkMessagesSetIsForServer(unittest.IsolatedAsyncioTestCase):
    async def test_preprocess_skill_forwards_llm_and_vtable_wiring(self) -> None:
        module = _load_module(
            I_SET_IS_FOR_SERVER_SCRIPT_PATH,
            "find_INetworkMessages_SetIsForServer",
        )
        mock_preprocess_common_skill = AsyncMock(return_value=True)
        expected_llm_decompile_specs = [
            (
                "INetworkMessages_SetIsForServer",
                "prompt/call_llm_decompile.md",
                "references/engine/CNetworkServerService_Init.{platform}.yaml",
            )
        ]
        expected_func_vtable_relations = [
            ("INetworkMessages_SetIsForServer", "INetworkMessages")
        ]
        expected_generate_yaml_desired_fields = [
            (
                "INetworkMessages_SetIsForServer",
                [
                    "func_name",
                    "vfunc_sig",
                    "vfunc_offset",
                    "vfunc_index",
                    "vtable_name",
                ],
            )
        ]
        llm_config = {
            "model": "gpt-4.1-mini",
            "api_key": "test-api-key",
            "base_url": "https://example.invalid/v1",
        }

        with patch.object(
            module,
            "preprocess_common_skill",
            mock_preprocess_common_skill,
        ):
            result = await module.preprocess_skill(
                session="session",
                skill_name="skill",
                expected_outputs=["out.yaml"],
                old_yaml_map={"k": "v"},
                new_binary_dir="bin_dir",
                platform="windows",
                image_base=0x180000000,
                llm_config=llm_config,
                debug=True,
            )

        self.assertTrue(result)
        mock_preprocess_common_skill.assert_awaited_once_with(
            session="session",
            expected_outputs=["out.yaml"],
            old_yaml_map={"k": "v"},
            new_binary_dir="bin_dir",
            platform="windows",
            image_base=0x180000000,
            func_names=["INetworkMessages_SetIsForServer"],
            func_vtable_relations=expected_func_vtable_relations,
            llm_decompile_specs=expected_llm_decompile_specs,
            llm_config=llm_config,
            generate_yaml_desired_fields=expected_generate_yaml_desired_fields,
            debug=True,
        )


class TestFindCNetworkMessagesGetNetworkGroupStats(
    unittest.IsolatedAsyncioTestCase
):
    async def test_preprocess_skill_forwards_llm_and_vtable_wiring(self) -> None:
        module = _load_module(
            NETWORK_GROUP_STATS_SCRIPT_PATH,
            "find_CNetworkMessages_GetNetworkGroupStats",
        )
        mock_preprocess_common_skill = AsyncMock(return_value=True)
        expected_llm_decompile_specs = [
            (
                "CNetworkMessages_GetNetworkGroupCount",
                "prompt/call_llm_decompile.md",
                "references/networksystem/CNetworkSystem_SendNetworkStats.{platform}.yaml",
            ),
            (
                "CNetworkMessages_GetNetworkGroupName",
                "prompt/call_llm_decompile.md",
                "references/networksystem/CNetworkSystem_SendNetworkStats.{platform}.yaml",
            ),
            (
                "CNetworkMessages_GetNetworkGroupColor",
                "prompt/call_llm_decompile.md",
                "references/networksystem/CNetworkSystem_SendNetworkStats.{platform}.yaml",
            ),
        ]
        expected_func_vtable_relations = [
            ("CNetworkMessages_GetNetworkGroupCount", "CNetworkMessages"),
            ("CNetworkMessages_GetNetworkGroupName", "CNetworkMessages"),
            ("CNetworkMessages_GetNetworkGroupColor", "CNetworkMessages"),
        ]
        expected_generate_yaml_desired_fields = [
            (
                "CNetworkMessages_GetNetworkGroupCount",
                [
                    "func_name",
                    "vfunc_sig",
                    "vfunc_offset",
                    "vfunc_index",
                    "vtable_name",
                ],
            ),
            (
                "CNetworkMessages_GetNetworkGroupName",
                [
                    "func_name",
                    "vfunc_sig",
                    "vfunc_offset",
                    "vfunc_index",
                    "vtable_name",
                ],
            ),
            (
                "CNetworkMessages_GetNetworkGroupColor",
                [
                    "func_name",
                    "vfunc_sig",
                    "vfunc_offset",
                    "vfunc_index",
                    "vtable_name",
                ],
            ),
        ]
        llm_config = {
            "model": "gpt-4.1-mini",
            "api_key": "test-api-key",
            "base_url": "https://example.invalid/v1",
        }

        with patch.object(
            module,
            "preprocess_common_skill",
            mock_preprocess_common_skill,
        ):
            result = await module.preprocess_skill(
                session="session",
                skill_name="skill",
                expected_outputs=["out.yaml"],
                old_yaml_map={"k": "v"},
                new_binary_dir="bin_dir",
                platform="windows",
                image_base=0x180000000,
                llm_config=llm_config,
                debug=True,
            )

        self.assertTrue(result)
        mock_preprocess_common_skill.assert_awaited_once_with(
            session="session",
            expected_outputs=["out.yaml"],
            old_yaml_map={"k": "v"},
            new_binary_dir="bin_dir",
            platform="windows",
            image_base=0x180000000,
            func_names=[
                "CNetworkMessages_GetNetworkGroupCount",
                "CNetworkMessages_GetNetworkGroupName",
                "CNetworkMessages_GetNetworkGroupColor",
            ],
            func_vtable_relations=expected_func_vtable_relations,
            llm_decompile_specs=expected_llm_decompile_specs,
            llm_config=llm_config,
            generate_yaml_desired_fields=expected_generate_yaml_desired_fields,
            debug=True,
        )


class TestFindCGameSystemReallocatingFactoryCSpawnGroupMgrGameSystemVtable(
    unittest.IsolatedAsyncioTestCase
):
    async def test_preprocess_skill_forwards_expected_vtable_and_aliases(self) -> None:
        module = _load_module(
            REALLOCATING_FACTORY_SCRIPT_PATH,
            "find_CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem_vtable",
        )
        mock_preprocess_common_skill = AsyncMock(return_value=True)
        expected_vtable_class_names = [
            "CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem"
        ]
        expected_mangled_class_names = {
            "CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem": [
                "??_7?$CGameSystemReallocatingFactory@VCSpawnGroupMgrGameSystem@@V1@@@6B@",
                "_ZTV30CGameSystemReallocatingFactoryI24CSpawnGroupMgrGameSystemS0_E",
            ]
        }
        expected_generate_yaml_desired_fields = [
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
        ]

        with patch.object(
            module,
            "preprocess_common_skill",
            mock_preprocess_common_skill,
        ):
            result = await module.preprocess_skill(
                session="session",
                skill_name="skill",
                expected_outputs=["out.yaml"],
                old_yaml_map={"k": "v"},
                new_binary_dir="bin_dir",
                platform="windows",
                image_base=0x180000000,
                debug=True,
            )

        self.assertTrue(result)
        mock_preprocess_common_skill.assert_awaited_once_with(
            session="session",
            expected_outputs=["out.yaml"],
            vtable_class_names=expected_vtable_class_names,
            mangled_class_names=expected_mangled_class_names,
            generate_yaml_desired_fields=expected_generate_yaml_desired_fields,
            platform="windows",
            image_base=0x180000000,
            debug=True,
        )


class TestFindCGameSystemReallocatingFactoryCSpawnGroupMgrGameSystemDeallocateImpl(
    unittest.IsolatedAsyncioTestCase
):
    async def test_preprocess_skill_forwards_expected_inherit_vfuncs(self) -> None:
        module = _load_module(
            REALLOCATING_FACTORY_DEALLOCATE_SCRIPT_PATH,
            "find_CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem_Deallocate_impl",
        )
        mock_preprocess_common_skill = AsyncMock(return_value=True)
        expected_inherit_vfuncs = [
            (
                "CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem_Deallocate",
                "CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem",
                "../client/IGameSystemFactory_Deallocate",
                True,
            )
        ]
        expected_generate_yaml_desired_fields = [
            (
                "CGameSystemReallocatingFactory_CSpawnGroupMgrGameSystem_Deallocate",
                [
                    "func_name",
                    "func_va",
                    "func_rva",
                    "func_size",
                    "func_sig",
                    "vtable_name",
                    "vfunc_offset",
                    "vfunc_index",
                ],
            )
        ]

        with patch.object(
            module,
            "preprocess_common_skill",
            mock_preprocess_common_skill,
        ):
            result = await module.preprocess_skill(
                session="session",
                skill_name="skill",
                expected_outputs=["out.yaml"],
                old_yaml_map={"k": "v"},
                new_binary_dir="bin_dir",
                platform="windows",
                image_base=0x180000000,
                debug=True,
            )

        self.assertTrue(result)
        mock_preprocess_common_skill.assert_awaited_once_with(
            session="session",
            expected_outputs=["out.yaml"],
            old_yaml_map={"k": "v"},
            new_binary_dir="bin_dir",
            platform="windows",
            image_base=0x180000000,
            inherit_vfuncs=expected_inherit_vfuncs,
            generate_yaml_desired_fields=expected_generate_yaml_desired_fields,
            debug=True,
        )


class TestPreprocessSingleSkillViaMcp(unittest.IsolatedAsyncioTestCase):
    async def test_forwards_llm_config_when_script_accepts_it(self) -> None:
        received = {}

        async def fake_preprocess_skill(
            session, skill_name, expected_outputs, old_yaml_map,
            new_binary_dir, platform, image_base, llm_config, debug=False,
        ):
            received["args"] = {
                "session": session,
                "skill_name": skill_name,
                "expected_outputs": expected_outputs,
                "old_yaml_map": old_yaml_map,
                "new_binary_dir": new_binary_dir,
                "platform": platform,
                "image_base": image_base,
                "llm_config": llm_config,
                "debug": debug,
            }
            return True

        with patch.object(
            ida_skill_preprocessor,
            "_get_preprocess_entry",
            return_value=fake_preprocess_skill,
        ), patch.object(
            ida_skill_preprocessor.httpx,
            "AsyncClient",
            _FakeAsyncClient,
        ), patch.object(
            ida_skill_preprocessor,
            "streamable_http_client",
            return_value=_FakeStreamableHttpClient(),
        ), patch.object(
            ida_skill_preprocessor,
            "ClientSession",
            _FakeClientSession,
        ), patch.object(
            ida_skill_preprocessor,
            "parse_mcp_result",
            return_value={"result": "0x180000000"},
        ):
            result = await ida_skill_preprocessor.preprocess_single_skill_via_mcp(
                host="127.0.0.1",
                port=13337,
                skill_name="find-CNetworkMessages_FindNetworkGroup",
                expected_outputs=["out.yaml"],
                old_yaml_map={"out.yaml": "old.yaml"},
                new_binary_dir="bin_dir",
                platform="windows",
                llm_model="gpt-4.1-mini",
                llm_apikey="test-api-key",
                llm_baseurl="https://example.invalid/v1",
                debug=True,
            )

        self.assertTrue(result)
        self.assertEqual(
            {
                "model": "gpt-4.1-mini",
                "api_key": "test-api-key",
                "base_url": "https://example.invalid/v1",
            },
            received["args"]["llm_config"],
        )
        self.assertEqual(0x180000000, received["args"]["image_base"])
        self.assertTrue(received["args"]["debug"])

    async def test_skips_llm_config_when_script_does_not_accept_it(self) -> None:
        received = {}

        async def fake_preprocess_skill(
            session, skill_name, expected_outputs, old_yaml_map,
            new_binary_dir, platform, image_base, debug=False,
        ):
            received["args"] = {
                "session": session,
                "skill_name": skill_name,
                "expected_outputs": expected_outputs,
                "old_yaml_map": old_yaml_map,
                "new_binary_dir": new_binary_dir,
                "platform": platform,
                "image_base": image_base,
                "debug": debug,
            }
            return True

        with patch.object(
            ida_skill_preprocessor,
            "_get_preprocess_entry",
            return_value=fake_preprocess_skill,
        ), patch.object(
            ida_skill_preprocessor.httpx,
            "AsyncClient",
            _FakeAsyncClient,
        ), patch.object(
            ida_skill_preprocessor,
            "streamable_http_client",
            return_value=_FakeStreamableHttpClient(),
        ), patch.object(
            ida_skill_preprocessor,
            "ClientSession",
            _FakeClientSession,
        ), patch.object(
            ida_skill_preprocessor,
            "parse_mcp_result",
            return_value={"result": "0x180000000"},
        ):
            result = await ida_skill_preprocessor.preprocess_single_skill_via_mcp(
                host="127.0.0.1",
                port=13337,
                skill_name="find-CNetworkMessages_FindNetworkGroup",
                expected_outputs=["out.yaml"],
                old_yaml_map={"out.yaml": "old.yaml"},
                new_binary_dir="bin_dir",
                platform="windows",
                llm_model="gpt-4.1-mini",
                llm_apikey="test-api-key",
                llm_baseurl="https://example.invalid/v1",
                debug=True,
            )

        self.assertTrue(result)
        self.assertEqual(0x180000000, received["args"]["image_base"])
        self.assertTrue(received["args"]["debug"])


class TestFindCNetworkMessagesFindNetworkGroup(unittest.IsolatedAsyncioTestCase):
    async def test_preprocess_skill_forwards_llm_and_vtable_wiring(self) -> None:
        module = _load_module(
            FIND_NETWORK_GROUP_SCRIPT_PATH,
            "find_CNetworkMessages_FindNetworkGroup",
        )
        mock_preprocess_common_skill = AsyncMock(return_value=True)
        expected_inherit_vfuncs = [
            (
                "CNetworkMessages_FindNetworkGroup",
                "CNetworkMessages",
                "../engine/INetworkMessages_FindNetworkGroup",
                True,
            )
        ]
        expected_func_vtable_relations = [
            ("CNetworkMessages_FindNetworkGroup", "CNetworkMessages")
        ]
        expected_generate_yaml_desired_fields = [
            (
                "CNetworkMessages_FindNetworkGroup",
                [
                    "func_name",
                    "func_va",
                    "func_rva",
                    "func_size",
                    "func_sig",
                    "vtable_name",
                    "vfunc_offset",
                    "vfunc_index",
                ],
            )
        ]
        llm_config = {
            "model": "gpt-4.1-mini",
            "api_key": "test-api-key",
            "base_url": "https://example.invalid/v1",
        }

        with patch.object(
            module,
            "preprocess_common_skill",
            mock_preprocess_common_skill,
        ):
            result = await module.preprocess_skill(
                session="session",
                skill_name="skill",
                expected_outputs=["out.yaml"],
                old_yaml_map={"k": "v"},
                new_binary_dir="bin_dir",
                platform="windows",
                image_base=0x180000000,
                llm_config=llm_config,
                debug=True,
            )

        self.assertTrue(result)
        mock_preprocess_common_skill.assert_awaited_once_with(
            session="session",
            expected_outputs=["out.yaml"],
            old_yaml_map={"k": "v"},
            new_binary_dir="bin_dir",
            platform="windows",
            image_base=0x180000000,
            func_names=["CNetworkMessages_FindNetworkGroup"],
            func_vtable_relations=expected_func_vtable_relations,
            inherit_vfuncs=expected_inherit_vfuncs,
            generate_yaml_desired_fields=expected_generate_yaml_desired_fields,
            llm_config=llm_config,
            debug=True,
        )


class TestFindINetworkMessagesFindNetworkGroup(unittest.IsolatedAsyncioTestCase):
    async def test_preprocess_skill_forwards_llm_and_vtable_wiring(self) -> None:
        module = _load_module(
            "ida_preprocessor_scripts/find-INetworkMessages_FindNetworkGroup.py",
            "find_INetworkMessages_FindNetworkGroup",
        )
        mock_preprocess_common_skill = AsyncMock(return_value=True)
        expected_llm_decompile_specs = [
            (
                "INetworkMessages_FindNetworkGroup",
                "prompt/call_llm_decompile.md",
                "references/engine/CNetworkGameClient_RecordEntityBandwidth.{platform}.yaml",
            )
        ]
        expected_func_vtable_relations = [
            ("INetworkMessages_FindNetworkGroup", "INetworkMessages")
        ]
        expected_generate_yaml_desired_fields = [
            (
                "INetworkMessages_FindNetworkGroup",
                [
                    "func_name",
                    "vfunc_sig",
                    "vfunc_offset",
                    "vfunc_index",
                    "vtable_name",
                ],
            )
        ]
        llm_config = {
            "model": "gpt-4.1-mini",
            "api_key": "test-api-key",
            "base_url": "https://example.invalid/v1",
        }

        with patch.object(
            module,
            "preprocess_common_skill",
            mock_preprocess_common_skill,
        ):
            result = await module.preprocess_skill(
                session="session",
                skill_name="skill",
                expected_outputs=["out.yaml"],
                old_yaml_map={"k": "v"},
                new_binary_dir="bin_dir",
                platform="windows",
                image_base=0x180000000,
                llm_config=llm_config,
                debug=True,
            )

        self.assertTrue(result)
        mock_preprocess_common_skill.assert_awaited_once_with(
            session="session",
            expected_outputs=["out.yaml"],
            old_yaml_map={"k": "v"},
            new_binary_dir="bin_dir",
            platform="windows",
            image_base=0x180000000,
            func_names=["INetworkMessages_FindNetworkGroup"],
            func_vtable_relations=expected_func_vtable_relations,
            llm_decompile_specs=expected_llm_decompile_specs,
            generate_yaml_desired_fields=expected_generate_yaml_desired_fields,
            llm_config=llm_config,
            debug=True,
        )


class TestFindINetworkMessagesSetNetworkSerializationContextDataAndCFlattenedSerializersCreateFieldChangedEventQueue(
    unittest.IsolatedAsyncioTestCase
):
    async def test_preprocess_skill_forwards_split_field_contracts(self) -> None:
        module = _load_module(
            "ida_preprocessor_scripts/find-INetworkMessages_SetNetworkSerializationContextData-AND-CFlattenedSerializers_CreateFieldChangedEventQueue.py",
            "find_INetworkMessages_SetNetworkSerializationContextData_AND_CFlattenedSerializers_CreateFieldChangedEventQueue",
        )
        mock_preprocess_common_skill = AsyncMock(return_value=True)
        expected_llm_decompile_specs = [
            (
                "INetworkMessages_SetNetworkSerializationContextData",
                "prompt/call_llm_decompile.md",
                "references/server/CEntitySystem_Activate.{platform}.yaml",
            ),
            (
                "CFlattenedSerializers_CreateFieldChangedEventQueue",
                "prompt/call_llm_decompile.md",
                "references/server/CEntitySystem_Activate.{platform}.yaml",
            ),
        ]
        expected_func_vtable_relations = [
            ("INetworkMessages_SetNetworkSerializationContextData", "INetworkMessages"),
            ("CFlattenedSerializers_CreateFieldChangedEventQueue", "CFlattenedSerializers"),
        ]
        expected_generate_yaml_desired_fields = [
            (
                "INetworkMessages_SetNetworkSerializationContextData",
                [
                    "func_name",
                    "vfunc_sig",
                    "vfunc_offset",
                    "vfunc_index",
                    "vtable_name",
                ],
            ),
            (
                "CFlattenedSerializers_CreateFieldChangedEventQueue",
                [
                    "func_name",
                    "vfunc_sig",
                    "vfunc_offset",
                    "vfunc_index",
                    "vtable_name",
                ],
            ),
        ]
        llm_config = {
            "model": "gpt-4.1-mini",
            "api_key": "test-api-key",
            "base_url": "https://example.invalid/v1",
        }

        with patch.object(
            module,
            "preprocess_common_skill",
            mock_preprocess_common_skill,
        ):
            result = await module.preprocess_skill(
                session="session",
                skill_name="skill",
                expected_outputs=["out-a.yaml", "out-b.yaml"],
                old_yaml_map={"k": "v"},
                new_binary_dir="bin_dir",
                platform="linux",
                image_base=0x180000000,
                llm_config=llm_config,
                debug=True,
            )

        self.assertTrue(result)
        mock_preprocess_common_skill.assert_awaited_once_with(
            session="session",
            expected_outputs=["out-a.yaml", "out-b.yaml"],
            old_yaml_map={"k": "v"},
            new_binary_dir="bin_dir",
            platform="linux",
            image_base=0x180000000,
            func_names=[
                "INetworkMessages_SetNetworkSerializationContextData",
                "CFlattenedSerializers_CreateFieldChangedEventQueue",
            ],
            func_vtable_relations=expected_func_vtable_relations,
            llm_decompile_specs=expected_llm_decompile_specs,
            llm_config=llm_config,
            generate_yaml_desired_fields=expected_generate_yaml_desired_fields,
            debug=True,
        )


class TestFindCBaseEntityCollisionRulesChanged(unittest.IsolatedAsyncioTestCase):
    async def test_preprocess_skill_forwards_generate_yaml_desired_fields(self) -> None:
        module = _load_module(
            "ida_preprocessor_scripts/find-CBaseEntity_CollisionRulesChanged.py",
            "find_CBaseEntity_CollisionRulesChanged",
        )
        mock_preprocess_common_skill = AsyncMock(return_value=True)
        expected_generate_yaml_desired_fields = [
            (
                "CBaseEntity_CollisionRulesChanged",
                ["func_name", "func_va", "func_rva", "func_size", "func_sig"],
            )
        ]

        with patch.object(
            module,
            "preprocess_common_skill",
            mock_preprocess_common_skill,
        ):
            result = await module.preprocess_skill(
                session="session",
                skill_name="skill",
                expected_outputs=["out.yaml"],
                old_yaml_map={"k": "v"},
                new_binary_dir="bin_dir",
                platform="windows",
                image_base=0x180000000,
                debug=True,
            )

        self.assertTrue(result)
        mock_preprocess_common_skill.assert_awaited_once_with(
            session="session",
            expected_outputs=["out.yaml"],
            old_yaml_map={"k": "v"},
            new_binary_dir="bin_dir",
            platform="windows",
            image_base=0x180000000,
            func_names=["CBaseEntity_CollisionRulesChanged"],
            generate_yaml_desired_fields=expected_generate_yaml_desired_fields,
            debug=True,
        )


class TestFindINetworkMessagesGetLoggingChannelWindows(
    unittest.IsolatedAsyncioTestCase
):
    async def test_preprocess_skill_forwards_vfunc_sig_max_match_directive(
        self,
    ) -> None:
        module = _load_module(
            I_GET_LOGGING_CHANNEL_WINDOWS_SCRIPT_PATH,
            "find_INetworkMessages_GetLoggingChannel_windows",
        )
        mock_preprocess_common_skill = AsyncMock(return_value=True)
        llm_config = {"model": "gpt-4.1-mini", "api_key": "test-api-key"}
        expected_llm_decompile_specs = [
            (
                "INetworkMessages_GetLoggingChannel",
                "prompt/call_llm_decompile.md",
                (
                    "references/server/"
                    "CNetworkUtlVectorEmbedded_TryLateResolve_m_vecRenderAttributes."
                    "{platform}.yaml"
                ),
            )
        ]
        expected_func_vtable_relations = [
            ("INetworkMessages_GetLoggingChannel", "INetworkMessages")
        ]
        expected_generate_yaml_desired_fields = [
            (
                "INetworkMessages_GetLoggingChannel",
                [
                    "func_name",
                    "vfunc_sig",
                    "vfunc_sig_max_match:10",
                    "vfunc_offset",
                    "vfunc_index",
                    "vtable_name",
                ],
            )
        ]

        with patch.object(
            module,
            "preprocess_common_skill",
            mock_preprocess_common_skill,
        ):
            result = await module.preprocess_skill(
                session="session",
                skill_name="skill",
                expected_outputs=["out.yaml"],
                old_yaml_map={"k": "v"},
                new_binary_dir="bin_dir",
                platform="windows",
                image_base=0x180000000,
                llm_config=llm_config,
                debug=True,
            )

        self.assertTrue(result)
        mock_preprocess_common_skill.assert_awaited_once_with(
            session="session",
            expected_outputs=["out.yaml"],
            old_yaml_map={"k": "v"},
            new_binary_dir="bin_dir",
            platform="windows",
            image_base=0x180000000,
            func_names=["INetworkMessages_GetLoggingChannel"],
            func_vtable_relations=expected_func_vtable_relations,
            llm_decompile_specs=expected_llm_decompile_specs,
            llm_config=llm_config,
            generate_yaml_desired_fields=expected_generate_yaml_desired_fields,
            debug=True,
        )


class TestFindINetworkMessagesGetLoggingChannelLinux(
    unittest.IsolatedAsyncioTestCase
):
    async def test_preprocess_skill_forwards_linux_llm_decompile_spec(
        self,
    ) -> None:
        module = _load_module(
            I_GET_LOGGING_CHANNEL_LINUX_SCRIPT_PATH,
            "find_INetworkMessages_GetLoggingChannel_linux",
        )
        mock_preprocess_common_skill = AsyncMock(return_value=True)
        llm_config = {"model": "gpt-4.1-mini", "api_key": "test-api-key"}
        expected_llm_decompile_specs = [
            (
                "INetworkMessages_GetLoggingChannel",
                "prompt/call_llm_decompile.md",
                (
                    "references/server/"
                    "CNetworkUtlVectorEmbedded_NetworkStateChanged_m_vecRenderAttributes."
                    "{platform}.yaml"
                ),
            )
        ]
        expected_func_vtable_relations = [
            ("INetworkMessages_GetLoggingChannel", "INetworkMessages")
        ]
        expected_generate_yaml_desired_fields = [
            (
                "INetworkMessages_GetLoggingChannel",
                [
                    "func_name",
                    "vfunc_sig",
                    "vfunc_sig_max_match:10",
                    "vfunc_offset",
                    "vfunc_index",
                    "vtable_name",
                ],
            )
        ]

        with patch.object(
            module,
            "preprocess_common_skill",
            mock_preprocess_common_skill,
        ):
            result = await module.preprocess_skill(
                session="session",
                skill_name="skill",
                expected_outputs=["out.yaml"],
                old_yaml_map={"k": "v"},
                new_binary_dir="bin_dir",
                platform="linux",
                image_base=0x180000000,
                llm_config=llm_config,
                debug=True,
            )

        self.assertTrue(result)
        mock_preprocess_common_skill.assert_awaited_once_with(
            session="session",
            expected_outputs=["out.yaml"],
            old_yaml_map={"k": "v"},
            new_binary_dir="bin_dir",
            platform="linux",
            image_base=0x180000000,
            func_names=["INetworkMessages_GetLoggingChannel"],
            func_vtable_relations=expected_func_vtable_relations,
            llm_decompile_specs=expected_llm_decompile_specs,
            llm_config=llm_config,
            generate_yaml_desired_fields=expected_generate_yaml_desired_fields,
            debug=True,
        )


class TestFindCNetworkServerServiceInit(unittest.IsolatedAsyncioTestCase):
    async def test_script_forwards_six_tuple_func_xrefs(self) -> None:
        module = _load_module(
            CNETWORK_SERVER_SERVICE_INIT_SCRIPT_PATH,
            "find_CNetworkServerService_Init",
        )
        mock_preprocess_common_skill = AsyncMock(return_value=True)
        expected_func_xrefs = [
            (
                "CNetworkServerService_Init",
                [
                    "ServerToClient",
                    "Entities",
                    "Local Player",
                    "Other Players",
                ],
                [],
                [],
                [],
                [],
            )
        ]
        expected_func_vtable_relations = [
            ("CNetworkServerService_Init", "CNetworkServerService")
        ]
        expected_generate_yaml_desired_fields = [
            (
                "CNetworkServerService_Init",
                [
                    "func_name",
                    "func_va",
                    "func_rva",
                    "func_size",
                    "func_sig",
                    "vtable_name",
                    "vfunc_offset",
                    "vfunc_index",
                ],
            )
        ]

        with patch.object(
            module,
            "preprocess_common_skill",
            mock_preprocess_common_skill,
        ):
            result = await module.preprocess_skill(
                session="session",
                skill_name="skill",
                expected_outputs=["out.yaml"],
                old_yaml_map={"k": "v"},
                new_binary_dir="bin_dir",
                platform="windows",
                image_base=0x180000000,
                debug=True,
            )

        self.assertTrue(result)
        mock_preprocess_common_skill.assert_awaited_once_with(
            session="session",
            expected_outputs=["out.yaml"],
            old_yaml_map={"k": "v"},
            new_binary_dir="bin_dir",
            platform="windows",
            image_base=0x180000000,
            func_names=["CNetworkServerService_Init"],
            func_xrefs=expected_func_xrefs,
            func_vtable_relations=expected_func_vtable_relations,
            generate_yaml_desired_fields=expected_generate_yaml_desired_fields,
            debug=True,
        )


if __name__ == "__main__":
    unittest.main()
