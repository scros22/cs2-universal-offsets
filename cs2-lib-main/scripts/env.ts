/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Ian Lucas. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/

import * as dotenv from "dotenv";
import { join } from "path";
dotenv.config({
    quiet: true
});

export const CWD_PATH = process.cwd();
export const CS2_CSGO_PATH = process.env.CS2_CSGO_PATH ?? join(CWD_PATH, "scripts/workdir/decompiled");
export const { INPUT_FORCE, STORAGE_ZONE, STORAGE_ACCESS_KEY, INPUT_TEXTURES } = process.env;
