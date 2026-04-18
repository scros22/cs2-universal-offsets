import unittest
from subprocess import CompletedProcess
from unittest.mock import patch

import run_cpp_tests


class TestRunFixHeaderAgent(unittest.TestCase):
    @patch.object(
        run_cpp_tests,
        "_load_codex_developer_instructions",
        return_value='developer_instructions="test prompt"',
    )
    @patch("run_cpp_tests.subprocess.run")
    def test_run_fix_header_agent_passes_codex_prompt_via_stdin_on_retry(
        self,
        mock_run,
        _mock_load_prompt,
    ) -> None:
        mock_run.side_effect = [
            CompletedProcess(args=["codex"], returncode=1, stdout="", stderr="first failure"),
            CompletedProcess(args=["codex"], returncode=0, stdout="", stderr=""),
        ]

        result = run_cpp_tests.run_fix_header_agent(
            fix_prompt="fix the vtable diff",
            agent="codex",
            debug=False,
            max_retries=2,
        )

        self.assertTrue(result)
        self.assertEqual(2, mock_run.call_count)

        first_call = mock_run.call_args_list[0]
        second_call = mock_run.call_args_list[1]

        self.assertEqual(["exec", "-"], first_call.args[0][-2:])
        self.assertEqual(
            ["exec", "resume", "--last", "-"],
            second_call.args[0][-4:],
        )
        self.assertEqual("fix the vtable diff", first_call.kwargs["input"])
        self.assertEqual("fix the vtable diff", second_call.kwargs["input"])
        self.assertTrue(first_call.kwargs["text"])
        self.assertTrue(second_call.kwargs["text"])


    @patch("run_cpp_tests.subprocess.run")
    def test_run_fix_header_agent_passes_claude_prompt_via_stdin(
        self,
        mock_run,
    ) -> None:
        mock_run.return_value = CompletedProcess(
            args=["claude"], returncode=0, stdout="", stderr=""
        )

        result = run_cpp_tests.run_fix_header_agent(
            fix_prompt="fix the vtable diff",
            agent="claude",
            debug=False,
            max_retries=1,
        )

        self.assertTrue(result)
        self.assertEqual(1, mock_run.call_count)

        call = mock_run.call_args_list[0]
        cmd = call.args[0]

        p_index = cmd.index("-p")
        self.assertEqual("-", cmd[p_index + 1])
        self.assertNotIn("fix the vtable diff", cmd)
        self.assertEqual("fix the vtable diff", call.kwargs["input"])
        self.assertTrue(call.kwargs["text"])
        self.assertIn("--session-id", cmd)
        self.assertNotIn("--resume", cmd)

    @patch("run_cpp_tests.subprocess.run")
    def test_run_fix_header_agent_passes_claude_prompt_via_stdin_on_retry(
        self,
        mock_run,
    ) -> None:
        mock_run.side_effect = [
            CompletedProcess(args=["claude"], returncode=1, stdout="", stderr="fail"),
            CompletedProcess(args=["claude"], returncode=0, stdout="", stderr=""),
        ]

        result = run_cpp_tests.run_fix_header_agent(
            fix_prompt="fix the vtable diff",
            agent="claude",
            debug=False,
            max_retries=2,
        )

        self.assertTrue(result)
        self.assertEqual(2, mock_run.call_count)

        first_call = mock_run.call_args_list[0]
        second_call = mock_run.call_args_list[1]
        first_cmd = first_call.args[0]
        second_cmd = second_call.args[0]

        for cmd in (first_cmd, second_cmd):
            p_index = cmd.index("-p")
            self.assertEqual("-", cmd[p_index + 1])
            self.assertNotIn("fix the vtable diff", cmd)

        self.assertEqual("fix the vtable diff", first_call.kwargs["input"])
        self.assertEqual("fix the vtable diff", second_call.kwargs["input"])

        self.assertIn("--session-id", first_cmd)
        self.assertNotIn("--resume", first_cmd)
        self.assertIn("--resume", second_cmd)
        self.assertNotIn("--session-id", second_cmd)

        sid_index = first_cmd.index("--session-id") + 1
        resume_index = second_cmd.index("--resume") + 1
        self.assertEqual(first_cmd[sid_index], second_cmd[resume_index])


    @patch("run_cpp_tests.subprocess.run")
    def test_run_fix_header_agent_external_session_id(
        self,
        mock_run,
    ) -> None:
        mock_run.return_value = CompletedProcess(
            args=["claude"], returncode=0, stdout="", stderr=""
        )

        result = run_cpp_tests.run_fix_header_agent(
            fix_prompt="fix it",
            agent="claude",
            debug=False,
            max_retries=1,
            session_id="custom-session-id",
        )

        self.assertTrue(result)
        cmd = mock_run.call_args_list[0].args[0]
        sid_index = cmd.index("--session-id") + 1
        self.assertEqual("custom-session-id", cmd[sid_index])

    @patch("run_cpp_tests.subprocess.run")
    def test_run_fix_header_agent_is_continuation_uses_resume(
        self,
        mock_run,
    ) -> None:
        mock_run.return_value = CompletedProcess(
            args=["claude"], returncode=0, stdout="", stderr=""
        )

        result = run_cpp_tests.run_fix_header_agent(
            fix_prompt="fix it",
            agent="claude",
            debug=False,
            max_retries=1,
            session_id="my-session",
            is_continuation=True,
        )

        self.assertTrue(result)
        cmd = mock_run.call_args_list[0].args[0]
        self.assertIn("--resume", cmd)
        self.assertNotIn("--session-id", cmd)
        resume_index = cmd.index("--resume") + 1
        self.assertEqual("my-session", cmd[resume_index])

    @patch.object(
        run_cpp_tests,
        "_load_codex_developer_instructions",
        return_value='developer_instructions="test"',
    )
    @patch("run_cpp_tests.subprocess.run")
    def test_run_fix_header_agent_codex_is_continuation_uses_resume(
        self,
        mock_run,
        _mock_load,
    ) -> None:
        mock_run.return_value = CompletedProcess(
            args=["codex"], returncode=0, stdout="", stderr=""
        )

        result = run_cpp_tests.run_fix_header_agent(
            fix_prompt="fix it",
            agent="codex",
            debug=False,
            max_retries=1,
            is_continuation=True,
        )

        self.assertTrue(result)
        cmd = mock_run.call_args_list[0].args[0]
        self.assertEqual(
            ["exec", "resume", "--last", "-"],
            cmd[-4:],
        )


class TestRunFixHeaderWithVerification(unittest.TestCase):
    def _make_args(self, **overrides):
        defaults = {
            "agent": "claude",
            "debug": False,
            "maxretry": 1,
            "maxverify": 3,
            "clang": "clang++",
            "std": "c++20",
            "gamever": "14132",
        }
        defaults.update(overrides)
        import argparse

        return argparse.Namespace(**defaults)

    def _make_test_item(self):
        return {
            "name": "TestVtable",
            "symbol": "IFoo",
            "cpp": "test.cpp",
            "target": "x86_64-pc-windows-msvc",
        }

    @patch.object(run_cpp_tests, "compile_and_compare")
    @patch.object(run_cpp_tests, "run_fix_header_agent")
    def test_passes_on_first_verify(self, mock_agent, mock_compile):
        from pathlib import Path

        mock_agent.return_value = True
        mock_compile.return_value = {
            "status": "ok",
            "command": [],
            "output": "",
            "compare_reports": [{"differences": []}],
        }

        result = run_cpp_tests.run_fix_header_with_verification(
            symbol="IFoo",
            header_paths=[Path("foo.h")],
            diff_reports=[{"differences": [{"type": "x", "message": "mismatch"}]}],
            test_item=self._make_test_item(),
            args=self._make_args(),
            config_dir=Path("."),
            bindir=Path("bin"),
            claude_allowed_tools="",
            claude_permission_mode="",
            claude_extra_args="",
            debug=False,
        )

        self.assertTrue(result)
        self.assertEqual(1, mock_agent.call_count)
        self.assertEqual(1, mock_compile.call_count)
        # First call should not be a continuation
        self.assertFalse(mock_agent.call_args.kwargs["is_continuation"])

    @patch.object(run_cpp_tests, "compile_and_compare")
    @patch.object(run_cpp_tests, "run_fix_header_agent")
    def test_retries_on_remaining_diffs(self, mock_agent, mock_compile):
        from pathlib import Path

        mock_agent.return_value = True
        mock_compile.side_effect = [
            # First verify: still has diffs
            {
                "status": "ok",
                "command": [],
                "output": "",
                "compare_reports": [
                    {"differences": [{"type": "x", "message": "still wrong"}]}
                ],
            },
            # Second verify: resolved
            {
                "status": "ok",
                "command": [],
                "output": "",
                "compare_reports": [{"differences": []}],
            },
        ]

        result = run_cpp_tests.run_fix_header_with_verification(
            symbol="IFoo",
            header_paths=[Path("foo.h")],
            diff_reports=[{"differences": [{"type": "x", "message": "mismatch"}]}],
            test_item=self._make_test_item(),
            args=self._make_args(),
            config_dir=Path("."),
            bindir=Path("bin"),
            claude_allowed_tools="",
            claude_permission_mode="",
            claude_extra_args="",
            debug=False,
        )

        self.assertTrue(result)
        self.assertEqual(2, mock_agent.call_count)
        self.assertEqual(2, mock_compile.call_count)
        # First call: not continuation; second call: is continuation
        self.assertFalse(mock_agent.call_args_list[0].kwargs["is_continuation"])
        self.assertTrue(mock_agent.call_args_list[1].kwargs["is_continuation"])
        # Both calls share the same session_id
        self.assertEqual(
            mock_agent.call_args_list[0].kwargs["session_id"],
            mock_agent.call_args_list[1].kwargs["session_id"],
        )

    @patch.object(run_cpp_tests, "compile_and_compare")
    @patch.object(run_cpp_tests, "run_fix_header_agent")
    def test_fails_after_max_verify(self, mock_agent, mock_compile):
        from pathlib import Path

        mock_agent.return_value = True
        mock_compile.return_value = {
            "status": "ok",
            "command": [],
            "output": "",
            "compare_reports": [
                {"differences": [{"type": "x", "message": "persistent"}]}
            ],
        }

        result = run_cpp_tests.run_fix_header_with_verification(
            symbol="IFoo",
            header_paths=[Path("foo.h")],
            diff_reports=[{"differences": [{"type": "x", "message": "mismatch"}]}],
            test_item=self._make_test_item(),
            args=self._make_args(maxverify=2),
            config_dir=Path("."),
            bindir=Path("bin"),
            claude_allowed_tools="",
            claude_permission_mode="",
            claude_extra_args="",
            debug=False,
        )

        self.assertFalse(result)
        self.assertEqual(2, mock_agent.call_count)
        self.assertEqual(2, mock_compile.call_count)

    @patch.object(run_cpp_tests, "compile_and_compare")
    @patch.object(run_cpp_tests, "run_fix_header_agent")
    def test_fails_on_agent_failure(self, mock_agent, mock_compile):
        from pathlib import Path

        mock_agent.return_value = False

        result = run_cpp_tests.run_fix_header_with_verification(
            symbol="IFoo",
            header_paths=[Path("foo.h")],
            diff_reports=[{"differences": [{"type": "x", "message": "mismatch"}]}],
            test_item=self._make_test_item(),
            args=self._make_args(),
            config_dir=Path("."),
            bindir=Path("bin"),
            claude_allowed_tools="",
            claude_permission_mode="",
            claude_extra_args="",
            debug=False,
        )

        self.assertFalse(result)
        self.assertEqual(1, mock_agent.call_count)
        mock_compile.assert_not_called()

    @patch.object(run_cpp_tests, "compile_and_compare")
    @patch.object(run_cpp_tests, "run_fix_header_agent")
    def test_fails_on_recompile_failure(self, mock_agent, mock_compile):
        from pathlib import Path

        mock_agent.return_value = True
        mock_compile.return_value = {
            "status": "compile_failed",
            "command": [],
            "output": "error: syntax error",
        }

        result = run_cpp_tests.run_fix_header_with_verification(
            symbol="IFoo",
            header_paths=[Path("foo.h")],
            diff_reports=[{"differences": [{"type": "x", "message": "mismatch"}]}],
            test_item=self._make_test_item(),
            args=self._make_args(),
            config_dir=Path("."),
            bindir=Path("bin"),
            claude_allowed_tools="",
            claude_permission_mode="",
            claude_extra_args="",
            debug=False,
        )

        self.assertFalse(result)
        self.assertEqual(1, mock_agent.call_count)
        self.assertEqual(1, mock_compile.call_count)


if __name__ == "__main__":
    unittest.main()
