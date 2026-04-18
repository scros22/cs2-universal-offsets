import json
import unittest
from unittest.mock import AsyncMock, patch

from ida_preprocessor_scripts import _registerconcommand as registerconcommand


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


class TestBuildRegisterConCommandPyEval(unittest.TestCase):
    def test_build_registerconcommand_py_eval_linux_embeds_exact_match_and_linux_registers(
        self,
    ) -> None:
        code = registerconcommand._build_registerconcommand_py_eval(
            platform="linux",
            command_name="bot_add",
            help_string=(
                "bot_add <t|ct> <type> <difficulty> <name> - "
                "Adds a bot matching the given criteria."
            ),
            search_window_before_call=48,
            search_window_after_xref=24,
        )

        self.assertIn("bot_add", code)
        self.assertIn("Adds a bot matching the given criteria.", code)
        self.assertIn("reg_names_linux", code)
        self.assertIn("rsi", code)
        self.assertIn("esi", code)
        self.assertIn("handler_va", code)
        self.assertIn("RegisterConCommand", code)
        self.assertIn("_is_registerconcommand_call", code)
        self.assertIn("idautils.XrefsTo", code)
        self.assertIn("_recover_register_value", code)
        self.assertIn("def _collect_candidates(params):", code)
        self.assertIn("return candidates", code)
        compile(code, "<registerconcommand_py_eval_linux>", "exec")

    def test_build_registerconcommand_py_eval_windows_embeds_slot_recovery_logic(
        self,
    ) -> None:
        code = registerconcommand._build_registerconcommand_py_eval(
            platform="windows",
            command_name="bot_add",
            help_string="bot_add <t|ct> <type> <difficulty> <name> - Adds a bot matching the given criteria.",
            search_window_before_call=48,
            search_window_after_xref=24,
        )

        self.assertIn("handler_slot_addr", code)
        self.assertIn("slot_value_addr", code)
        self.assertIn("lea", code)
        self.assertIn("command_name = params", code)
        self.assertIn("_recover_stack_slot", code)
        self.assertIn("_recover_slot_value", code)
        self.assertIn("reg_names_windows", code)
        self.assertIn("traceback.format_exc()", code)
        self.assertIn("'ok': False", code)
        self.assertIn("_collect_candidates(params)", code)
        compile(code, "<registerconcommand_py_eval_windows>", "exec")


class TestCollectRegisterConCommandCandidates(unittest.IsolatedAsyncioTestCase):
    async def test_collect_registerconcommand_candidates_uses_py_eval_and_returns_candidates(
        self,
    ) -> None:
        session = AsyncMock()
        session.call_tool.return_value = _py_eval_payload(
            {
                "candidates": [
                    {
                        "command_name": "bot_add",
                        "help_string": "bot_add <t|ct> <type> <difficulty> <name> - Adds a bot matching the given criteria.",
                        "handler_va": "0x180055000",
                    }
                ]
            }
        )

        candidates = await registerconcommand._collect_registerconcommand_candidates(
            session=session,
            platform="linux",
            command_name="bot_add",
            help_string="bot_add <t|ct> <type> <difficulty> <name> - Adds a bot matching the given criteria.",
            search_window_before_call=48,
            search_window_after_xref=24,
            debug=True,
        )

        self.assertEqual(
            [
                {
                    "command_name": "bot_add",
                    "help_string": "bot_add <t|ct> <type> <difficulty> <name> - Adds a bot matching the given criteria.",
                    "handler_va": "0x180055000",
                }
            ],
            candidates,
        )
        code = session.call_tool.await_args.kwargs["arguments"]["code"]
        self.assertIn("bot_add", code)
        self.assertIn("search_window_before_call = 48", code)
        self.assertIn("search_window_after_xref = 24", code)

    async def test_collect_registerconcommand_candidates_returns_empty_on_invalid_payload(
        self,
    ) -> None:
        session = AsyncMock()
        session.call_tool.return_value = _py_eval_payload({"unexpected": []})

        candidates = await registerconcommand._collect_registerconcommand_candidates(
            session=session,
            platform="windows",
            command_name="bot_add",
            help_string=None,
            search_window_before_call=48,
            search_window_after_xref=24,
            debug=True,
        )

        self.assertEqual([], candidates)

    async def test_collect_registerconcommand_candidates_prints_traceback_when_py_eval_returns_error(
        self,
    ) -> None:
        session = AsyncMock()
        session.call_tool.return_value = _py_eval_payload(
            {
                "ok": False,
                "traceback": (
                    "Traceback (most recent call last):\n"
                    "  File \"<string>\", line 1, in <module>\n"
                    "RuntimeError: boom"
                ),
            }
        )

        with patch("builtins.print") as mock_print:
            candidates = (
                await registerconcommand._collect_registerconcommand_candidates(
                    session=session,
                    platform="windows",
                    command_name="bot_add",
                    help_string=None,
                    search_window_before_call=48,
                    search_window_after_xref=24,
                    debug=True,
                )
            )

        self.assertEqual([], candidates)
        printed = "\n".join(
            call.args[0] for call in mock_print.call_args_list if call.args
        )
        self.assertIn("RegisterConCommand py_eval traceback follows", printed)
        self.assertIn("RuntimeError: boom", printed)

    async def test_collect_registerconcommand_candidates_returns_empty_on_invalid_candidate_structure(
        self,
    ) -> None:
        session = AsyncMock()
        session.call_tool.return_value = _py_eval_payload({"candidates": [{}]})

        candidates = await registerconcommand._collect_registerconcommand_candidates(
            session=session,
            platform="windows",
            command_name="bot_add",
            help_string=None,
            search_window_before_call=48,
            search_window_after_xref=24,
            debug=True,
        )

        self.assertEqual([], candidates)

    async def test_collect_registerconcommand_candidates_returns_empty_on_invalid_candidate_values(
        self,
    ) -> None:
        session = AsyncMock()
        session.call_tool.return_value = _py_eval_payload(
            {
                "candidates": [
                    {
                        "command_name": 1,
                        "help_string": {},
                        "handler_va": [],
                    }
                ]
            }
        )

        candidates = await registerconcommand._collect_registerconcommand_candidates(
            session=session,
            platform="windows",
            command_name="bot_add",
            help_string=None,
            search_window_before_call=48,
            search_window_after_xref=24,
            debug=True,
        )

        self.assertEqual([], candidates)


class TestQueryFuncInfo(unittest.IsolatedAsyncioTestCase):
    async def test_query_func_info_returns_dict_from_py_eval_payload(self) -> None:
        session = AsyncMock()
        session.call_tool = AsyncMock(
            return_value=_py_eval_payload(
                {
                    "func_va": "0x180055000",
                    "func_size": "0x90",
                }
            )
        )

        result = await registerconcommand._query_func_info(
            session,
            "0x180055000",
            debug=True,
        )

        self.assertEqual(
            result,
            {
                "func_va": "0x180055000",
                "func_size": "0x90",
            },
        )


class TestPreprocessRegisterConCommandSkill(unittest.IsolatedAsyncioTestCase):
    async def test_preprocess_registerconcommand_skill_writes_requested_func_payload(
        self,
    ) -> None:
        session = AsyncMock()
        requested_fields = [
            (
                "BotAdd_CommandHandler",
                ["func_name", "func_sig", "func_va", "func_rva", "func_size"],
            )
        ]

        with patch.object(
            registerconcommand,
            "_collect_registerconcommand_candidates",
            AsyncMock(
                return_value=[
                    {
                        "command_name": "bot_add",
                        "help_string": "bot_add <t|ct> <type> <difficulty> <name> - Adds a bot matching the given criteria.",
                        "handler_va": "0x180055000",
                    }
                ]
            ),
        ), patch.object(
            registerconcommand,
            "_query_func_info",
            AsyncMock(return_value={"func_va": "0x180055000", "func_size": "0x90"}),
        ), patch.object(
            registerconcommand,
            "preprocess_gen_func_sig_via_mcp",
            AsyncMock(
                return_value={
                    "func_va": "0x180055000",
                    "func_rva": "0x55000",
                    "func_size": "0x90",
                    "func_sig": "48 89 5C 24 ?? 57",
                }
            ),
        ), patch.object(registerconcommand, "write_func_yaml") as mock_write:
            result = await registerconcommand.preprocess_registerconcommand_skill(
                session=session,
                expected_outputs=["/tmp/BotAdd_CommandHandler.windows.yaml"],
                new_binary_dir="/tmp",
                platform="windows",
                image_base=0x180000000,
                target_name="BotAdd_CommandHandler",
                generate_yaml_desired_fields=requested_fields,
                command_name="bot_add",
                help_string="bot_add <t|ct> <type> <difficulty> <name> - Adds a bot matching the given criteria.",
                debug=True,
            )

        self.assertTrue(result)
        mock_write.assert_called_once_with(
            "/tmp/BotAdd_CommandHandler.windows.yaml",
            {
                "func_name": "BotAdd_CommandHandler",
                "func_va": "0x180055000",
                "func_rva": "0x55000",
                "func_size": "0x90",
                "func_sig": "48 89 5C 24 ?? 57",
            },
        )

    async def test_preprocess_registerconcommand_skill_requires_exact_command_name_match(
        self,
    ) -> None:
        with patch.object(
            registerconcommand,
            "_collect_registerconcommand_candidates",
            AsyncMock(
                return_value=[
                    {
                        "command_name": "bot_add_cheat",
                        "help_string": "bot_add <t|ct> <type> <difficulty> <name> - Adds a bot matching the given criteria.",
                        "handler_va": "0x180055000",
                    }
                ]
            ),
        ):
            result = await registerconcommand.preprocess_registerconcommand_skill(
                session=AsyncMock(),
                expected_outputs=["/tmp/BotAdd_CommandHandler.windows.yaml"],
                new_binary_dir="/tmp",
                platform="windows",
                image_base=0x180000000,
                target_name="BotAdd_CommandHandler",
                generate_yaml_desired_fields=[
                    ("BotAdd_CommandHandler", ["func_name", "func_va"])
                ],
                command_name="bot_add",
                help_string=None,
                debug=True,
            )

        self.assertFalse(result)

    async def test_preprocess_registerconcommand_skill_rejects_multiple_handlers(
        self,
    ) -> None:
        with patch.object(
            registerconcommand,
            "_collect_registerconcommand_candidates",
            AsyncMock(
                return_value=[
                    {
                        "command_name": "bot_add",
                        "help_string": "a",
                        "handler_va": "0x180010000",
                    },
                    {
                        "command_name": "bot_add",
                        "help_string": "a",
                        "handler_va": "0x180020000",
                    },
                ]
            ),
        ):
            result = await registerconcommand.preprocess_registerconcommand_skill(
                session=AsyncMock(),
                expected_outputs=["/tmp/BotAdd_CommandHandler.windows.yaml"],
                new_binary_dir="/tmp",
                platform="windows",
                image_base=0x180000000,
                target_name="BotAdd_CommandHandler",
                generate_yaml_desired_fields=[
                    ("BotAdd_CommandHandler", ["func_name", "func_va"])
                ],
                command_name="bot_add",
                help_string=None,
                debug=True,
            )

        self.assertFalse(result)

    async def test_preprocess_registerconcommand_skill_requires_exact_help_string_match(
        self,
    ) -> None:
        with patch.object(
            registerconcommand,
            "_collect_registerconcommand_candidates",
            AsyncMock(
                return_value=[
                    {
                        "command_name": "bot_add",
                        "help_string": "bot_add something else",
                        "handler_va": "0x180055000",
                    }
                ]
            ),
        ):
            result = await registerconcommand.preprocess_registerconcommand_skill(
                session=AsyncMock(),
                expected_outputs=["/tmp/BotAdd_CommandHandler.windows.yaml"],
                new_binary_dir="/tmp",
                platform="windows",
                image_base=0x180000000,
                target_name="BotAdd_CommandHandler",
                generate_yaml_desired_fields=[
                    ("BotAdd_CommandHandler", ["func_name", "func_va"])
                ],
                command_name="bot_add",
                help_string="bot_add expected help",
                debug=True,
            )

        self.assertFalse(result)

    async def test_preprocess_registerconcommand_skill_rejects_when_match_keys_missing(
        self,
    ) -> None:
        result = await registerconcommand.preprocess_registerconcommand_skill(
            session=AsyncMock(),
            expected_outputs=["/tmp/BotAdd_CommandHandler.windows.yaml"],
            new_binary_dir="/tmp",
            platform="windows",
            image_base=0x180000000,
            target_name="BotAdd_CommandHandler",
            generate_yaml_desired_fields=[("BotAdd_CommandHandler", ["func_name"])],
            command_name=None,
            help_string=None,
            debug=True,
        )

        self.assertFalse(result)

    async def test_preprocess_registerconcommand_skill_rejects_non_single_expected_match_count(
        self,
    ) -> None:
        result = await registerconcommand.preprocess_registerconcommand_skill(
            session=AsyncMock(),
            expected_outputs=["/tmp/BotAdd_CommandHandler.windows.yaml"],
            new_binary_dir="/tmp",
            platform="windows",
            image_base=0x180000000,
            target_name="BotAdd_CommandHandler",
            generate_yaml_desired_fields=[("BotAdd_CommandHandler", ["func_name"])],
            command_name="bot_add",
            help_string=None,
            expected_match_count=2,
            debug=True,
        )

        self.assertFalse(result)

    async def test_preprocess_registerconcommand_skill_returns_false_when_requested_field_missing(
        self,
    ) -> None:
        with patch.object(
            registerconcommand,
            "_collect_registerconcommand_candidates",
            AsyncMock(
                return_value=[
                    {
                        "command_name": "bot_add",
                        "help_string": "a",
                        "handler_va": "0x180055000",
                    }
                ]
            ),
        ), patch.object(
            registerconcommand,
            "_query_func_info",
            AsyncMock(return_value={"func_va": "0x180055000", "func_size": "0x90"}),
        ), patch.object(registerconcommand, "write_func_yaml") as mock_write:
            result = await registerconcommand.preprocess_registerconcommand_skill(
                session=AsyncMock(),
                expected_outputs=["/tmp/BotAdd_CommandHandler.windows.yaml"],
                new_binary_dir="/tmp",
                platform="windows",
                image_base=0x180000000,
                target_name="BotAdd_CommandHandler",
                generate_yaml_desired_fields=[
                    ("BotAdd_CommandHandler", ["func_name", "func_missing"])
                ],
                command_name="bot_add",
                help_string=None,
                debug=True,
            )

        self.assertFalse(result)
        mock_write.assert_not_called()
