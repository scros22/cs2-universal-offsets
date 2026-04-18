import { defineConfig, type RolldownOptions } from "rolldown";
import { dts } from "rolldown-plugin-dts";
import del from "rollup-plugin-delete";

const options: RolldownOptions = {
    input: ["src/index.ts", "src/translations/index.ts"],
    plugins: [del({ targets: ["dist"] }), dts()],
    treeshake: true,
    output: {
        dir: "dist",
        format: "esm",
        minify: true,
        entryFileNames: "[name].mjs",
        chunkFileNames: "_chunks/[name]-[hash].mjs",
        preserveModules: true,
        preserveModulesRoot: "src"
    }
};

export default defineConfig(options) as RolldownOptions;
