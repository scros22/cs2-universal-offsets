import importlib
import json
import types
import unittest
from unittest.mock import AsyncMock, patch


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


def _import_register_event_listener_module():
    return importlib.import_module(
        "ida_preprocessor_scripts._register_event_listener_abstract"
    )


class TestBuildRegisterEventListenerPyEval(unittest.TestCase):
    def test_build_register_event_listener_py_eval_windows_embeds_hexrays_and_slot_recovery(
        self,
    ) -> None:
        register_event_listener = _import_register_event_listener_module()
        code = register_event_listener._build_register_event_listener_py_eval(
            platform="windows",
            source_func_va="0x180010000",
            anchor_event_name="CLoopModeGame::OnClientPollNetworking",
            search_window_after_anchor=24,
            search_window_before_call=64,
        )

        self.assertIn("ida_hexrays", code)
        self.assertIn("source_func_va", code)
        self.assertIn("0x180010000", code)
        self.assertIn("anchor_event_name", code)
        self.assertIn("_recover_register_value", code)
        self.assertIn("anchor callee not found", code)
        self.assertIn("temp_callback_slot", code)
        compile(code, "<register_event_listener_windows>", "exec")

    def test_build_register_event_listener_py_eval_linux_recovers_reused_temp_base_register(
        self,
    ) -> None:
        register_event_listener = _import_register_event_listener_module()
        source_func_va = 0x15F5200
        register_func_va = 0x15265A0
        poll_callback_va = 0x15BC910
        advance_callback_va = 0x15BCB50
        poll_string_va = 0xB6C820
        advance_string_va = 0xB3C758
        temp_base = 0x50
        temp_callback_slot = temp_base + 8

        o_reg = 1
        o_displ = 2
        o_near = 3
        instructions = {
            0x15F5292: ("lea", ("r14", "[rbp+var_50]"), (o_reg, o_displ), (0, temp_base)),
            0x15F529D: ("mov", ("rsi", "r14"), (o_reg, o_reg), (0, 0)),
            0x15F52AC: ("lea", ("rdx", "CLoopModeGame_OnClientPollNetworking"), (o_reg, o_near), (0, poll_callback_va)),
            0x15F52BE: ("mov", ("[rbp+var_48]", "rdx"), (o_displ, o_reg), (temp_callback_slot, 0)),
            0x15F52C2: ("lea", ("rdx", "aCloopmodegameO"), (o_reg, o_near), (0, poll_string_va)),
            0x15F52C9: ("push", ("rdx", ""), (o_reg, 0), (0, 0)),
            0x15F52CF: ("call", ("RegisterEventListener_Abstract", ""), (o_near, 0), (register_func_va, 0)),
            0x15F52E4: ("mov", ("rsi", "r14"), (o_reg, o_reg), (0, 0)),
            0x15F52EB: ("lea", ("rdx", "sub_15BCB50"), (o_reg, o_near), (0, advance_callback_va)),
            0x15F5302: ("mov", ("[rbp+var_48]", "rdx"), (o_displ, o_reg), (temp_callback_slot, 0)),
            0x15F5306: ("lea", ("rdx", "aCloopmodegameO_0"), (o_reg, o_near), (0, advance_string_va)),
            0x15F5310: ("mov", ("[rsp+0D0h+var_D0]", "rdx"), (o_displ, o_reg), (0, 0)),
            0x15F5319: ("call", ("RegisterEventListener_Abstract", ""), (o_near, 0), (register_func_va, 0)),
        }
        heads = sorted(instructions)

        class FakeFunc:
            def __init__(self, start_ea: int, end_ea: int) -> None:
                self.start_ea = start_ea
                self.end_ea = end_ea

        class FakeXref:
            def __init__(self, frm: int) -> None:
                self.frm = frm

        class FakeString:
            def __init__(self, ea: int, text: str) -> None:
                self.ea = ea
                self.text = text

            def __str__(self) -> str:
                return self.text

        class FakeLine:
            def __init__(self, line: str) -> None:
                self.line = line

        class FakeExpr:
            def __init__(self, op: int, ea: int = 0, obj_ea: int = 0) -> None:
                self.op = op
                self.ea = ea
                self.obj_ea = obj_ea
                self.x = None

        class FakeCallExpr:
            def __init__(self, ea: int, callee_ea: int) -> None:
                self.op = 10
                self.ea = ea
                self.x = FakeExpr(13, obj_ea=callee_ea)

        class FakeCFunc:
            body = [
                FakeCallExpr(0x15F52CF, register_func_va),
                FakeCallExpr(0x15F5319, register_func_va),
            ]

            def get_pseudocode(self) -> list[FakeLine]:
                return [
                    FakeLine('"CLoopModeGame::OnClientPollNetworking"'),
                    FakeLine('"CLoopModeGame::OnClientAdvanceTick"'),
                ]

        class FakeVisitor:
            def __init__(self, _flags: int) -> None:
                pass

            def apply_to(self, item, _parent) -> int:
                for expr in item:
                    self.visit_expr(expr)
                return 0

        def fake_get_func(ea: int):
            if ea == source_func_va:
                return FakeFunc(source_func_va, 0x15F5400)
            if ea in (register_func_va, poll_callback_va, advance_callback_va):
                return FakeFunc(ea, ea + 0x20)
            return None

        def fake_prev_head(start_ea: int, min_ea: int) -> int:
            for head in reversed(heads):
                if min_ea <= head < start_ea:
                    return head
            return -1

        def fake_next_head(start_ea: int, max_ea: int) -> int:
            for head in heads:
                if start_ea < head < max_ea:
                    return head
            return -1

        def fake_xrefs_to(ea: int, _flags: int) -> list[FakeXref]:
            if ea == poll_string_va:
                return [FakeXref(0x15F52C2)]
            if ea == register_func_va:
                return [FakeXref(0x15F52CF), FakeXref(0x15F5319)]
            return []

        fake_idaapi = types.SimpleNamespace(
            o_imm=4,
            o_mem=5,
            o_near=o_near,
            o_far=6,
            o_displ=o_displ,
            o_reg=o_reg,
            BADADDR=-1,
            get_func=fake_get_func,
        )
        fake_idc = types.SimpleNamespace(
            STRTYPE_C=0,
            prev_head=fake_prev_head,
            next_head=fake_next_head,
            print_insn_mnem=lambda ea: instructions.get(ea, ("", (), (), ()))[0],
            print_operand=lambda ea, index: instructions.get(ea, ("", ("", ""), (), ()))[1][index],
            get_operand_type=lambda ea, index: instructions.get(ea, ("", (), (-1, -1), ()))[2][index],
            get_operand_value=lambda ea, index: instructions.get(ea, ("", (), (), (0, 0)))[3][index],
            get_strlit_contents=lambda ea, _length, _type: {
                poll_string_va: b"CLoopModeGame::OnClientPollNetworking",
                advance_string_va: b"CLoopModeGame::OnClientAdvanceTick",
            }.get(ea),
            is_code=lambda flags: bool(flags),
        )
        fake_idautils = types.SimpleNamespace(
            Strings=lambda: [
                FakeString(poll_string_va, "CLoopModeGame::OnClientPollNetworking"),
                FakeString(advance_string_va, "CLoopModeGame::OnClientAdvanceTick"),
            ],
            XrefsTo=fake_xrefs_to,
        )
        fake_hexrays = types.SimpleNamespace(
            ctree_visitor_t=FakeVisitor,
            CV_FAST=1,
            cot_call=10,
            cot_cast=11,
            cot_ref=12,
            cot_obj=13,
            decompile=lambda _ea: FakeCFunc(),
        )

        code = register_event_listener._build_register_event_listener_py_eval(
            platform="linux",
            source_func_va=hex(source_func_va),
            anchor_event_name="CLoopModeGame::OnClientPollNetworking",
            search_window_after_anchor=64,
            search_window_before_call=64,
        )
        modules = {
            "ida_hexrays": fake_hexrays,
            "idaapi": fake_idaapi,
            "ida_bytes": types.SimpleNamespace(get_full_flags=lambda ea: ea in instructions),
            "idautils": fake_idautils,
            "idc": fake_idc,
        }
        namespace = {}

        with patch.dict("sys.modules", modules):
            exec(code, namespace)

        payload = json.loads(namespace["result"])
        self.assertTrue(payload["ok"], payload)
        items_by_event = {item["event_name"]: item for item in payload["items"]}
        self.assertEqual(
            hex(poll_callback_va),
            items_by_event["CLoopModeGame::OnClientPollNetworking"]["callback_va"],
        )
        self.assertEqual(
            hex(advance_callback_va),
            items_by_event["CLoopModeGame::OnClientAdvanceTick"]["callback_va"],
        )


class TestCollectRegisterEventListenerCandidates(unittest.IsolatedAsyncioTestCase):
    async def test_collect_candidates_returns_register_function_and_items(self) -> None:
        register_event_listener = _import_register_event_listener_module()
        session = AsyncMock()
        session.call_tool.return_value = _py_eval_payload(
            {
                "ok": True,
                "register_func_va": "0x180055000",
                "items": [
                    {
                        "event_name": "CLoopModeGame::OnClientPollNetworking",
                        "callback_va": "0x180066000",
                        "call_ea": "0x180012345",
                        "temp_base": "0x28",
                        "temp_callback_slot": "0x30",
                    }
                ],
            }
        )

        result = (
            await register_event_listener._collect_register_event_listener_candidates(
                session=session,
                platform="windows",
                source_func_va="0x180010000",
                anchor_event_name="CLoopModeGame::OnClientPollNetworking",
                search_window_after_anchor=24,
                search_window_before_call=64,
                debug=True,
            )
        )

        py_eval_code = session.call_tool.await_args.kwargs["arguments"]["code"]
        self.assertIn("source_func_va", py_eval_code)
        self.assertIn("0x180010000", py_eval_code)
        self.assertIn("CLoopModeGame::OnClientPollNetworking", py_eval_code)
        self.assertEqual("0x180055000", result["register_func_va"])
        self.assertEqual(
            "CLoopModeGame::OnClientPollNetworking",
            result["items"][0]["event_name"],
        )

    async def test_collect_candidates_returns_none_when_hexrays_is_unavailable(
        self,
    ) -> None:
        register_event_listener = _import_register_event_listener_module()
        session = AsyncMock()
        session.call_tool.return_value = _py_eval_payload(
            {"ok": False, "error": "ida_hexrays unavailable"}
        )

        result = (
            await register_event_listener._collect_register_event_listener_candidates(
                session=session,
                platform="linux",
                source_func_va="0x180010000",
                anchor_event_name="CLoopModeGame::OnClientPollNetworking",
                search_window_after_anchor=24,
                search_window_before_call=64,
                debug=True,
            )
        )

        self.assertIsNone(result)

    async def test_collect_candidates_returns_none_when_items_is_not_list(
        self,
    ) -> None:
        register_event_listener = _import_register_event_listener_module()
        session = AsyncMock()
        session.call_tool.return_value = _py_eval_payload(
            {
                "ok": True,
                "register_func_va": "0x180055000",
                "items": {"event_name": "CLoopModeGame::OnClientPollNetworking"},
            }
        )

        result = (
            await register_event_listener._collect_register_event_listener_candidates(
                session=session,
                platform="windows",
                source_func_va="0x180010000",
                anchor_event_name="CLoopModeGame::OnClientPollNetworking",
                search_window_after_anchor=24,
                search_window_before_call=64,
                debug=True,
            )
        )

        self.assertIsNone(result)

    async def test_collect_candidates_returns_none_when_item_is_missing_required_key(
        self,
    ) -> None:
        register_event_listener = _import_register_event_listener_module()
        session = AsyncMock()
        session.call_tool.return_value = _py_eval_payload(
            {
                "ok": True,
                "register_func_va": "0x180055000",
                "items": [
                    {
                        "event_name": "CLoopModeGame::OnClientPollNetworking",
                        "callback_va": "0x180066000",
                        "call_ea": "0x180012345",
                        "temp_base": "0x28",
                    }
                ],
            }
        )

        result = (
            await register_event_listener._collect_register_event_listener_candidates(
                session=session,
                platform="windows",
                source_func_va="0x180010000",
                anchor_event_name="CLoopModeGame::OnClientPollNetworking",
                search_window_after_anchor=24,
                search_window_before_call=64,
                debug=True,
            )
        )

        self.assertIsNone(result)

    async def test_collect_candidates_returns_none_when_item_field_type_is_invalid(
        self,
    ) -> None:
        register_event_listener = _import_register_event_listener_module()
        session = AsyncMock()
        session.call_tool.return_value = _py_eval_payload(
            {
                "ok": True,
                "register_func_va": "0x180055000",
                "items": [
                    {
                        "event_name": "CLoopModeGame::OnClientPollNetworking",
                        "callback_va": 0x180066000,
                        "call_ea": "0x180012345",
                        "temp_base": "0x28",
                        "temp_callback_slot": "0x30",
                    }
                ],
            }
        )

        result = (
            await register_event_listener._collect_register_event_listener_candidates(
                session=session,
                platform="windows",
                source_func_va="0x180010000",
                anchor_event_name="CLoopModeGame::OnClientPollNetworking",
                search_window_after_anchor=24,
                search_window_before_call=64,
                debug=True,
            )
        )

        self.assertIsNone(result)


class TestPreprocessRegisterEventListenerAbstractSkill(
    unittest.IsolatedAsyncioTestCase
):
    async def test_preprocess_skill_writes_register_function_and_target_callbacks(
        self,
    ) -> None:
        register_event_listener = _import_register_event_listener_module()
        target_specs = [
            {
                "target_name": "CLoopModeGame_OnClientPollNetworking",
                "event_name": "CLoopModeGame::OnClientPollNetworking",
                "rename_to": "CLoopModeGame_OnClientPollNetworking",
            },
            {
                "target_name": "CLoopModeGame_OnClientAdvanceTick",
                "event_name": "CLoopModeGame::OnClientAdvanceTick",
                "rename_to": "CLoopModeGame_OnClientAdvanceTick",
            },
        ]

        requested_fields = [
            ("RegisterEventListener_Abstract", ["func_name", "func_va"]),
            ("CLoopModeGame_OnClientPollNetworking", ["func_name", "func_va"]),
            ("CLoopModeGame_OnClientAdvanceTick", ["func_name", "func_va"]),
        ]
        func_info_by_va = {
            "0x180055000": {"func_va": "0x180055000", "func_size": "0x40"},
            "0x180066000": {"func_va": "0x180066000", "func_size": "0x50"},
            "0x180077000": {"func_va": "0x180077000", "func_size": "0x60"},
        }

        async def _query_func_info_by_va(_session, func_va, debug=False):
            _ = debug
            return func_info_by_va.get(func_va)

        with patch.object(
            register_event_listener,
            "_read_yaml",
            return_value={"func_va": "0x180010000"},
        ), patch.object(
            register_event_listener,
            "_collect_register_event_listener_candidates",
            AsyncMock(
                return_value={
                    "register_func_va": "0x180055000",
                    "items": [
                        {
                            "event_name": "CLoopModeGame::OnClientPollNetworking",
                            "callback_va": "0x180066000",
                            "call_ea": "0x180012345",
                            "temp_base": "0x28",
                            "temp_callback_slot": "0x30",
                        },
                        {
                            "event_name": "CLoopModeGame::OnClientAdvanceTick",
                            "callback_va": "0x180077000",
                            "call_ea": "0x180012390",
                            "temp_base": "0x28",
                            "temp_callback_slot": "0x30",
                        },
                        {
                            "event_name": "CLoopModeGame::OnUnusedNullsub",
                            "callback_va": "0x180088000",
                            "call_ea": "0x1800123D0",
                            "temp_base": "0x28",
                            "temp_callback_slot": "0x30",
                        },
                    ],
                }
            ),
        ), patch.object(
            register_event_listener,
            "_query_func_info",
            AsyncMock(side_effect=_query_func_info_by_va),
        ), patch.object(register_event_listener, "write_func_yaml") as mock_write:
            result = (
                await register_event_listener.preprocess_register_event_listener_abstract_skill(
                    session=AsyncMock(),
                    expected_outputs=[
                        "/tmp/RegisterEventListener_Abstract.windows.yaml",
                        "/tmp/CLoopModeGame_OnClientPollNetworking.windows.yaml",
                        "/tmp/CLoopModeGame_OnClientAdvanceTick.windows.yaml",
                    ],
                    new_binary_dir="/tmp",
                    platform="windows",
                    image_base=0x180000000,
                    source_yaml_stem="CLoopModeGame_RegisterEventMapInternal",
                    register_func_target_name="RegisterEventListener_Abstract",
                    anchor_event_name="CLoopModeGame::OnClientPollNetworking",
                    target_specs=target_specs,
                    generate_yaml_desired_fields=requested_fields,
                    debug=True,
                )
            )

        self.assertTrue(result)
        self.assertEqual(3, mock_write.call_count)
        expected_writes = {
            "/tmp/RegisterEventListener_Abstract.windows.yaml": {
                "func_name": "RegisterEventListener_Abstract",
                "func_va": "0x180055000",
            },
            "/tmp/CLoopModeGame_OnClientPollNetworking.windows.yaml": {
                "func_name": "CLoopModeGame_OnClientPollNetworking",
                "func_va": "0x180066000",
            },
            "/tmp/CLoopModeGame_OnClientAdvanceTick.windows.yaml": {
                "func_name": "CLoopModeGame_OnClientAdvanceTick",
                "func_va": "0x180077000",
            },
        }
        actual_writes = {
            call_args.args[0]: call_args.args[1]
            for call_args in mock_write.call_args_list
        }
        self.assertEqual(expected_writes, actual_writes)
        self.assertNotIn(
            "/tmp/CLoopModeGame_OnUnusedNullsub.windows.yaml",
            actual_writes,
        )

    async def test_preprocess_skill_writes_requested_signature_fields(
        self,
    ) -> None:
        register_event_listener = _import_register_event_listener_module()
        target_specs = [
            {
                "target_name": "CLoopModeGame_OnClientPollNetworking",
                "event_name": "CLoopModeGame::OnClientPollNetworking",
            }
        ]
        requested_fields = [
            (
                "RegisterEventListener_Abstract",
                ["func_name", "func_va", "func_sig", "func_rva", "func_size"],
            ),
            (
                "CLoopModeGame_OnClientPollNetworking",
                ["func_name", "func_va", "func_sig", "func_rva", "func_size"],
            ),
        ]
        func_info_by_va = {
            "0x180055000": {"func_va": "0x180055000", "func_size": "0x999"},
            "0x180066000": {"func_va": "0x180066000", "func_size": "0x888"},
        }
        sig_info_by_va = {
            "0x180055000": {
                "func_sig": "48 8B ?? 55",
                "func_rva": "0x55000",
                "func_size": "0x40",
            },
            "0x180066000": {
                "func_sig": "40 53 ?? 48",
                "func_rva": "0x66000",
                "func_size": "0x50",
            },
        }

        async def _query_func_info_by_va(_session, func_va, debug=False):
            _ = debug
            return func_info_by_va.get(func_va)

        async def _gen_func_sig_by_va(session, func_va, image_base, debug=False):
            _ = (session, image_base, debug)
            return sig_info_by_va.get(func_va)

        mock_collect = AsyncMock(
            return_value={
                "register_func_va": "0x180055000",
                "items": [
                    {
                        "event_name": "CLoopModeGame::OnClientPollNetworking",
                        "callback_va": "0x180066000",
                        "call_ea": "0x180012345",
                        "temp_base": "0x28",
                        "temp_callback_slot": "0x30",
                    }
                ],
            }
        )
        mock_query = AsyncMock(side_effect=_query_func_info_by_va)
        mock_sig = AsyncMock(side_effect=_gen_func_sig_by_va)

        with patch.object(
            register_event_listener,
            "_read_yaml",
            return_value={"func_va": "0x180010000"},
        ), patch.object(
            register_event_listener,
            "_collect_register_event_listener_candidates",
            mock_collect,
        ), patch.object(
            register_event_listener,
            "_query_func_info",
            mock_query,
        ), patch.object(
            register_event_listener,
            "preprocess_gen_func_sig_via_mcp",
            mock_sig,
        ), patch.object(
            register_event_listener,
            "_rename_func_best_effort",
            AsyncMock(),
        ), patch.object(register_event_listener, "write_func_yaml") as mock_write:
            result = (
                await register_event_listener.preprocess_register_event_listener_abstract_skill(
                    session=AsyncMock(),
                    expected_outputs=[
                        "/tmp/RegisterEventListener_Abstract.windows.yaml",
                        "/tmp/CLoopModeGame_OnClientPollNetworking.windows.yaml",
                    ],
                    new_binary_dir="/tmp",
                    platform="windows",
                    image_base=0x180000000,
                    source_yaml_stem="CLoopModeGame_RegisterEventMapInternal",
                    register_func_target_name="RegisterEventListener_Abstract",
                    anchor_event_name="CLoopModeGame::OnClientPollNetworking",
                    target_specs=target_specs,
                    generate_yaml_desired_fields=requested_fields,
                    debug=True,
                )
            )

        self.assertTrue(result)
        self.assertEqual("0x180010000", mock_collect.await_args.kwargs["source_func_va"])
        self.assertEqual(
            ["0x180055000", "0x180066000"],
            [call_args.kwargs["func_va"] for call_args in mock_sig.await_args_list],
        )
        expected_writes = {
            "/tmp/RegisterEventListener_Abstract.windows.yaml": {
                "func_name": "RegisterEventListener_Abstract",
                "func_va": "0x180055000",
                "func_sig": "48 8B ?? 55",
                "func_rva": "0x55000",
                "func_size": "0x40",
            },
            "/tmp/CLoopModeGame_OnClientPollNetworking.windows.yaml": {
                "func_name": "CLoopModeGame_OnClientPollNetworking",
                "func_va": "0x180066000",
                "func_sig": "40 53 ?? 48",
                "func_rva": "0x66000",
                "func_size": "0x50",
            },
        }
        actual_writes = {
            call_args.args[0]: call_args.args[1]
            for call_args in mock_write.call_args_list
        }
        self.assertEqual(expected_writes, actual_writes)

    async def test_preprocess_skill_returns_false_when_declared_event_is_missing(
        self,
    ) -> None:
        register_event_listener = _import_register_event_listener_module()
        target_specs = [
            {
                "target_name": "CLoopModeGame_OnClientPollNetworking",
                "event_name": "CLoopModeGame::OnClientPollNetworking",
            },
            {
                "target_name": "CLoopModeGame_OnClientAdvanceTick",
                "event_name": "CLoopModeGame::OnClientAdvanceTick",
            },
        ]
        requested_fields = [
            ("RegisterEventListener_Abstract", ["func_name", "func_va"]),
            ("CLoopModeGame_OnClientPollNetworking", ["func_name", "func_va"]),
            ("CLoopModeGame_OnClientAdvanceTick", ["func_name", "func_va"]),
        ]

        with patch.object(
            register_event_listener,
            "_read_yaml",
            return_value={"func_va": "0x180010000"},
        ), patch.object(
            register_event_listener,
            "_collect_register_event_listener_candidates",
            AsyncMock(
                return_value={
                    "register_func_va": "0x180055000",
                    "items": [
                        {
                            "event_name": "CLoopModeGame::OnClientPollNetworking",
                            "callback_va": "0x180066000",
                            "call_ea": "0x180012345",
                            "temp_base": "0x28",
                            "temp_callback_slot": "0x30",
                        }
                    ],
                }
            ),
        ), patch.object(
            register_event_listener,
            "_query_func_info",
            AsyncMock(
                return_value={"func_va": "0x180055000", "func_size": "0x40"}
            ),
        ), patch.object(register_event_listener, "write_func_yaml") as mock_write:
            result = (
                await register_event_listener.preprocess_register_event_listener_abstract_skill(
                    session=AsyncMock(),
                    expected_outputs=[
                        "/tmp/RegisterEventListener_Abstract.windows.yaml",
                        "/tmp/CLoopModeGame_OnClientPollNetworking.windows.yaml",
                        "/tmp/CLoopModeGame_OnClientAdvanceTick.windows.yaml",
                    ],
                    new_binary_dir="/tmp",
                    platform="windows",
                    image_base=0x180000000,
                    source_yaml_stem="CLoopModeGame_RegisterEventMapInternal",
                    register_func_target_name="RegisterEventListener_Abstract",
                    anchor_event_name="CLoopModeGame::OnClientPollNetworking",
                    target_specs=target_specs,
                    generate_yaml_desired_fields=requested_fields,
                    debug=True,
                )
            )

        self.assertFalse(result)
        mock_write.assert_not_called()

    async def test_preprocess_skill_returns_false_when_extra_event_exists_in_strict_mode(
        self,
    ) -> None:
        register_event_listener = _import_register_event_listener_module()
        target_specs = [
            {
                "target_name": "CLoopModeGame_OnClientPollNetworking",
                "event_name": "CLoopModeGame::OnClientPollNetworking",
            }
        ]
        requested_fields = [
            ("RegisterEventListener_Abstract", ["func_name", "func_va"]),
            ("CLoopModeGame_OnClientPollNetworking", ["func_name", "func_va"]),
        ]

        with patch.object(
            register_event_listener,
            "_read_yaml",
            return_value={"func_va": "0x180010000"},
        ), patch.object(
            register_event_listener,
            "_collect_register_event_listener_candidates",
            AsyncMock(
                return_value={
                    "register_func_va": "0x180055000",
                    "items": [
                        {
                            "event_name": "CLoopModeGame::OnClientPollNetworking",
                            "callback_va": "0x180066000",
                            "call_ea": "0x180012345",
                            "temp_base": "0x28",
                            "temp_callback_slot": "0x30",
                        },
                        {
                            "event_name": "CLoopModeGame::OnUnusedNullsub",
                            "callback_va": "0x180077000",
                            "call_ea": "0x180012390",
                            "temp_base": "0x28",
                            "temp_callback_slot": "0x30",
                        },
                    ],
                }
            ),
        ), patch.object(
            register_event_listener,
            "_query_func_info",
            AsyncMock(),
        ) as mock_query, patch.object(
            register_event_listener,
            "_rename_func_best_effort",
            AsyncMock(),
        ) as mock_rename, patch.object(
            register_event_listener,
            "write_func_yaml",
        ) as mock_write:
            result = (
                await register_event_listener.preprocess_register_event_listener_abstract_skill(
                    session=AsyncMock(),
                    expected_outputs=[
                        "/tmp/RegisterEventListener_Abstract.windows.yaml",
                        "/tmp/CLoopModeGame_OnClientPollNetworking.windows.yaml",
                    ],
                    new_binary_dir="/tmp",
                    platform="windows",
                    image_base=0x180000000,
                    source_yaml_stem="CLoopModeGame_RegisterEventMapInternal",
                    register_func_target_name="RegisterEventListener_Abstract",
                    anchor_event_name="CLoopModeGame::OnClientPollNetworking",
                    target_specs=target_specs,
                    generate_yaml_desired_fields=requested_fields,
                    allow_extra_events=False,
                    debug=True,
                )
            )

        self.assertFalse(result)
        mock_query.assert_not_awaited()
        mock_rename.assert_not_awaited()
        mock_write.assert_not_called()

    async def test_preprocess_skill_returns_false_without_any_write_when_callback_payload_build_fails(
        self,
    ) -> None:
        register_event_listener = _import_register_event_listener_module()
        target_specs = [
            {
                "target_name": "CLoopModeGame_OnClientPollNetworking",
                "event_name": "CLoopModeGame::OnClientPollNetworking",
            },
            {
                "target_name": "CLoopModeGame_OnClientAdvanceTick",
                "event_name": "CLoopModeGame::OnClientAdvanceTick",
            },
        ]
        requested_fields = [
            ("RegisterEventListener_Abstract", ["func_name", "func_va"]),
            ("CLoopModeGame_OnClientPollNetworking", ["func_name", "func_va"]),
            ("CLoopModeGame_OnClientAdvanceTick", ["func_name", "func_va"]),
        ]
        func_info_by_va = {
            "0x180055000": {"func_va": "0x180055000", "func_size": "0x40"},
            "0x180066000": {"func_va": "0x180066000", "func_size": "0x50"},
            "0x180077000": None,
        }

        async def _query_func_info_by_va(_session, func_va, debug=False):
            _ = debug
            return func_info_by_va.get(func_va)

        with patch.object(
            register_event_listener,
            "_read_yaml",
            return_value={"func_va": "0x180010000"},
        ), patch.object(
            register_event_listener,
            "_collect_register_event_listener_candidates",
            AsyncMock(
                return_value={
                    "register_func_va": "0x180055000",
                    "items": [
                        {
                            "event_name": "CLoopModeGame::OnClientPollNetworking",
                            "callback_va": "0x180066000",
                            "call_ea": "0x180012345",
                            "temp_base": "0x28",
                            "temp_callback_slot": "0x30",
                        },
                        {
                            "event_name": "CLoopModeGame::OnClientAdvanceTick",
                            "callback_va": "0x180077000",
                            "call_ea": "0x180012390",
                            "temp_base": "0x28",
                            "temp_callback_slot": "0x30",
                        },
                    ],
                }
            ),
        ), patch.object(
            register_event_listener,
            "_query_func_info",
            AsyncMock(side_effect=_query_func_info_by_va),
        ), patch.object(
            register_event_listener,
            "_rename_func_best_effort",
            AsyncMock(),
        ) as mock_rename, patch.object(
            register_event_listener,
            "write_func_yaml",
        ) as mock_write:
            result = (
                await register_event_listener.preprocess_register_event_listener_abstract_skill(
                    session=AsyncMock(),
                    expected_outputs=[
                        "/tmp/RegisterEventListener_Abstract.windows.yaml",
                        "/tmp/CLoopModeGame_OnClientPollNetworking.windows.yaml",
                        "/tmp/CLoopModeGame_OnClientAdvanceTick.windows.yaml",
                    ],
                    new_binary_dir="/tmp",
                    platform="windows",
                    image_base=0x180000000,
                    source_yaml_stem="CLoopModeGame_RegisterEventMapInternal",
                    register_func_target_name="RegisterEventListener_Abstract",
                    anchor_event_name="CLoopModeGame::OnClientPollNetworking",
                    target_specs=target_specs,
                    generate_yaml_desired_fields=requested_fields,
                    debug=True,
                )
            )

        self.assertFalse(result)
        mock_rename.assert_not_awaited()
        mock_write.assert_not_called()
