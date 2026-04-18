import unittest

from ida_analyze_util import build_remote_text_export_py_eval


class TestBuildRemoteTextExportPyEval(unittest.TestCase):
    def test_build_remote_text_export_py_eval_rejects_relative_output_path(self) -> None:
        with self.assertRaises(ValueError):
            build_remote_text_export_py_eval(
                output_path="relative/detail.yaml",
                producer_code="payload_text = 'ok'",
                content_var="payload_text",
            )

    def test_build_remote_text_export_py_eval_contains_atomic_write_and_small_ack(self) -> None:
        script = build_remote_text_export_py_eval(
            output_path="/tmp/vcall-detail.yaml",
            producer_code="payload_text = 'ok'",
            content_var="payload_text",
            format_name="yaml",
        )
        self.assertIn("os.path.isabs(output_path)", script)
        self.assertIn("tmp_path = output_path + '.tmp'", script)
        self.assertIn("os.replace(tmp_path, output_path)", script)
        self.assertIn("'bytes_written'", script)
        self.assertIn("'format': format_name", script)


if __name__ == "__main__":
    unittest.main()
