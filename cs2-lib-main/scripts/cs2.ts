/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Ian Lucas. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/

import { depotDownloader } from "@ianlucas/depot-downloader";
import { DecompilerArgs, vrfDecompiler } from "@ianlucas/vrf-decompiler";
import { mkdir, writeFile } from "fs/promises";
import { availableParallelism } from "os";
import { join } from "path";
import { assert, ensure } from "../src/utils";
import { CS2_CSGO_PATH, CWD_PATH, INPUT_FORCE, INPUT_TEXTURES } from "./env";
import { log, readFileOrDefault, readProcess, shouldRun } from "./utils";

export const SCRIPTS_DIR = join(CWD_PATH, "scripts");
export const WORKDIR_DIR = join(SCRIPTS_DIR, "workdir");
export const DECOMPILED_DIR = join(WORKDIR_DIR, "decompiled");
export const ASSETS_MANIFEST_PATH = join(SCRIPTS_DIR, "cs2.manifest");
export const DEPOT_FILELIST_PATH = join(SCRIPTS_DIR, "cs2.depot");
export const PAK_FILELIST_PATH = join(WORKDIR_DIR, "cs2_pak.depot");
export const DEPOT_CSGO_PATH = join(WORKDIR_DIR, "game/csgo");
export const CSGO_PAK_DIR_PATH = join(DEPOT_CSGO_PATH, "pak01_dir.vpk");

const DEPOT_SUCCESS_RE = /100[,.]00%/;
const DEPOT_MANIFEST_RE = /Manifest\s(\d+)/;

const APP_ID = 730;
const ASSETS_DEPOT_ID = 2347770;
const EXTRACT_DEFAULT_DIRS = ["weapons/models/"];
const EXTRACT_IMAGE_DIRS = ["panorama/", "resource/", "scripts/", "soundevents/"];
const EXTRACT_TEXTURE_DIRS = [
    "weapons/paints/",
    "materials/models/weapons/customization/paints/vmats/",
    "materials/models/weapons/customization/paints/custom/",
    "materials/models/weapons/customization/paints/gunsmith/",
    "items/assets/paintkits/"
];

export class CS2 {
    public local = !CS2_CSGO_PATH.includes("workdir");

    private async fetchLatestAssetsManifest() {
        const output = await readProcess(
            depotDownloader({
                app: APP_ID,
                depot: ASSETS_DEPOT_ID,
                dir: WORKDIR_DIR,
                manifestOnly: true
            })
        );
        return ensure(output.match(DEPOT_MANIFEST_RE)?.[1]);
    }

    private async downloadCsgoPakDir() {
        log("Downloading 'pak01_dir.vpk'...");
        const output = await readProcess(
            depotDownloader({
                app: APP_ID,
                depot: ASSETS_DEPOT_ID,
                dir: WORKDIR_DIR,
                filelist: DEPOT_FILELIST_PATH
            })
        );
        assert(DEPOT_SUCCESS_RE.test(output));
    }

    private async browseRequiredCsgoPakDirParts() {
        log("Checking required 'pak01_dir.vpk' parts...");
        const vpks = new Set<string>(["game/csgo/steam.inf"]);
        const output = await readProcess(
            vrfDecompiler({
                input: CSGO_PAK_DIR_PATH,
                vpkDir: true
            })
        );
        const dirs = [
            ...EXTRACT_DEFAULT_DIRS,
            ...EXTRACT_IMAGE_DIRS,
            ...(INPUT_TEXTURES === "true" ? EXTRACT_TEXTURE_DIRS : [])
        ];
        for (const line of output.split("\n")) {
            if (dirs.some((dir) => line.startsWith(dir))) {
                const meta = Object.fromEntries(
                    line
                        .split(" ")
                        .slice(1)
                        .map((kv) => kv.split("="))
                ) as { fnumber: string };
                vpks.add(`game/csgo/pak01_${meta.fnumber.padStart(3, "0")}.vpk`);
            }
        }
        await writeFile(PAK_FILELIST_PATH, [...vpks].join("\n"), "utf-8");
    }

    private async syncLatestAssetsManifest() {
        const current = await readFileOrDefault(ASSETS_MANIFEST_PATH);
        const latest = await this.fetchLatestAssetsManifest();
        assert(INPUT_FORCE === "true" || current !== latest, `Depot ${ASSETS_DEPOT_ID} is already up to date.`);
        await writeFile(ASSETS_MANIFEST_PATH, latest, "utf-8");
    }

    private async downloadCsgoPakDirParts() {
        await this.browseRequiredCsgoPakDirParts();
        log("Downloading packages...");
        const output = await readProcess(
            depotDownloader({
                app: APP_ID,
                depot: ASSETS_DEPOT_ID,
                dir: WORKDIR_DIR,
                filelist: PAK_FILELIST_PATH
            })
        );
        assert(DEPOT_SUCCESS_RE.test(output));
    }

    private async extractAndDecompileFiles() {
        const threads = availableParallelism();
        log(`Extracting and decompiling asset files (${threads} threads)...`);
        for (const dir of EXTRACT_IMAGE_DIRS) {
            await readProcess(
                vrfDecompiler({
                    input: CSGO_PAK_DIR_PATH,
                    output: DECOMPILED_DIR,
                    vpkDecompile: true,
                    vpkFilepath: dir,
                    threads
                })
            );
        }
        return true;
    }

    public async download() {
        await mkdir(WORKDIR_DIR, { recursive: true });
        await this.syncLatestAssetsManifest();
        await this.downloadCsgoPakDir();
        await this.downloadCsgoPakDirParts();
        await this.extractAndDecompileFiles();
        log("Game files successfully downloaded and decompiled.");
    }

    async decompile(options: DecompilerArgs) {
        return await readProcess(
            vrfDecompiler({
                input: CSGO_PAK_DIR_PATH,
                threads: availableParallelism(),
                ...options
            })
        );
    }
}

if (shouldRun(import.meta.url)) {
    await new CS2().download();
}
