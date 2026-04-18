/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Ian Lucas. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/

import { ChildProcessWithoutNullStreams } from "child_process";
import { createHash } from "crypto";
import { createReadStream, existsSync, readFileSync } from "fs";
import { access, readFile, rm, writeFile } from "fs/promises";
import { decode as htmlEntitiesDecode } from "html-entities";
import { basename, resolve } from "path";
import { pipeline } from "stream/promises";
import { fileURLToPath } from "url";
import { ensure } from "../src/utils";

export const log = console.log;
export const warning = console.warn;

export async function writeJson(path: string, contents: any) {
    const file = resolve(process.cwd(), path);
    const stringified = JSON.stringify(contents);
    await writeFile(file, stringified, "utf-8");
}

export function readJson<T>(path: string, fallback?: T) {
    const file = resolve(process.cwd(), path);
    if (fallback !== undefined && !existsSync(file)) {
        return fallback;
    }
    return JSON.parse(readFileSync(file, "utf-8")) as T;
}

export async function write(path: string, contents: string) {
    const file = resolve(process.cwd(), path);
    await writeFile(file, contents, "utf-8");
}

export async function read(path: string) {
    const file = resolve(process.cwd(), path);
    return await readFile(file, "utf-8");
}

export async function sleep(ms: number) {
    return new Promise((resolve) => setTimeout(resolve, ms));
}

export async function fetchText(url: string) {
    url = htmlEntitiesDecode(url);
    log(`GET ${url}`);
    return (await fetch(url)).text();
}

export function dedupe<T>(array: T[]) {
    return [...new Set(array)];
}

export function shouldRun(url: string) {
    return basename(process.argv[1]) === basename(fileURLToPath(url));
}

export async function exists(path: string) {
    try {
        await access(path);
        return true;
    } catch {
        return false;
    }
}

export function readProcess(ps: ChildProcessWithoutNullStreams) {
    return new Promise<string>((resolve, reject) => {
        let data = "";
        ps.stdout.on("data", (chunk) => (data += chunk));
        ps.stderr.on("data", (chunk) => (data += chunk));
        ps.on("close", (code) => {
            if (code === 0) {
                resolve(data);
            } else {
                reject(data);
            }
        });
    });
}

export function prependHash<T extends string | undefined>(str: T) {
    if (str === undefined || str.startsWith("#")) {
        return str;
    }
    return `#${str}`;
}

export class PromiseQueue {
    private concurrency: number;
    private queue: { promiseFn: () => Promise<void>; resolve: (value?: unknown) => void; reject: () => void }[];
    private running: number;
    private completedCount: number;
    private idleResolvers: ((value?: unknown) => void)[];

    constructor(concurrency: number) {
        this.concurrency = concurrency;
        this.queue = [];
        this.running = 0;
        this.completedCount = 0;
        this.idleResolvers = [];
    }

    push(promiseFn: () => Promise<void>) {
        return new Promise((resolve, reject) => {
            this.queue.push({ promiseFn, resolve, reject });
            this.next();
        });
    }

    async waitForIdle() {
        if (this.queue.length === 0 && this.running === 0) return;
        return new Promise((resolve) => {
            this.idleResolvers.push(resolve);
        });
    }

    private next() {
        while (this.running < this.concurrency && this.queue.length > 0) {
            const { promiseFn, resolve, reject } = ensure(this.queue.shift());
            this.running++;
            promiseFn()
                .then(resolve)
                .catch(reject)
                .finally(() => {
                    this.running--;
                    this.completedCount++;

                    if (this.completedCount % this.concurrency === 0) {
                        const remaining = this.queue.length + this.running;
                        console.log(`${remaining} items left in queue...`);
                    }

                    this.next();
                    if (this.queue.length === 0 && this.running === 0) {
                        const resolvers = this.idleResolvers;
                        this.idleResolvers = [];
                        for (const r of resolvers) r();
                    }
                });
        }
    }
}

export async function getFileSha256(filePath: string) {
    const hash = createHash("sha256");
    const stream = createReadStream(filePath);
    await pipeline(stream, hash);
    return hash.digest("hex").toLowerCase();
}

export async function getBufferSha256(buffer: Buffer) {
    return createHash("sha256").update(buffer).digest("hex");
}

export async function getFilesSha256(filePaths: string[]) {
    const hash = createHash("sha256");
    for (const filePath of filePaths) {
        const fileBuffer = await readFile(filePath);
        hash.update(fileBuffer);
    }
    return hash.digest("hex");
}

export async function readFileOrDefault(path: string, fallback = "") {
    if (!(await exists(path))) {
        return fallback;
    }
    return await readFile(path, "utf-8");
}

export async function rmIfExists(path: string) {
    if (await exists(path)) {
        await rm(path, { recursive: true });
    }
}

const sent: string[] = [];
export function logOnce(message: string) {
    if (!sent.includes(message)) {
        sent.push(message);
        log(message);
    }
}
