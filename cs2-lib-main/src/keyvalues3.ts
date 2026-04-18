/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Ian Lucas. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/

import { fail } from "./utils.ts";

type CS2KeyValue3Value = null | string | boolean | number | CS2KeyValue3Value[] | { [key: string]: CS2KeyValue3Value };

export class CS2KeyValues3 {
    static parse<T = any>(data: string): T {
        data = data.replace(/\[[\$!][^\]]+\]/g, "").trim();
        let index = 0;

        function printDebug() {
            console.log(
                `<<<${
                    data.substring(Math.max(0, index - 64), index) +
                    ">>>" +
                    data[index] +
                    "<<<" +
                    data.substring(index + 1, Math.min(data.length, index + 63))
                }>>>`
            );
        }

        function skipHeader(): void {
            if (data.startsWith("<!--")) {
                const endIndex = data.indexOf("-->");
                if (endIndex !== -1) {
                    index = endIndex + 3;
                    skipWhitespace();
                }
            }
        }

        function peek(str: string) {
            let i = index;
            for (let j = 0; j < str.length; j++) {
                if (data[i++] !== str[j]) {
                    return false;
                }
            }
            return true;
        }

        function isWhitespace(char?: string): boolean {
            return (char?.match(/[\s\t\r\n]/) ?? undefined) !== undefined;
        }

        function isNumber(char?: string): boolean {
            return (char?.match(/[\d-]/) ?? undefined) !== undefined;
        }

        function skipWhitespace(): void {
            while (data[index] && isWhitespace(data[index])) {
                index += 1;
            }
            if (data[index] === "/" && data[index + 1] === "/") {
                while (data[index] && data[index] !== "\n") {
                    index += 1;
                }
                skipWhitespace();
            }
        }

        function parseKey(): string {
            let value = "";
            while (data[index]) {
                if (data[index]?.match(/["a-zA-Z_\d\.]/)) {
                    value += data[index];
                    index += 1;
                } else {
                    if (isWhitespace(data[index])) {
                        skipWhitespace();
                    }
                    if (data[index] === "=") {
                        if (value.length === 0) {
                            fail(`Empty key at ${index}`);
                        }
                        if (value[0] === '"' && value[value.length - 1] === '"') {
                            value = value.substring(1, value.length - 1);
                        }
                        index += 1;
                        return value;
                    }
                    printDebug();
                    fail(`Bad end of key at ${index}`);
                }
            }
            fail(`Bad end of key at ${index}`);
        }

        function parseString(): string {
            if (data[index] === '"') {
                let multiLine = peek(`"""`);
                index += multiLine ? 3 : 1;
                let value = "";
                while (data[index] && (multiLine ? !peek(`"""`) : data[index] !== '"')) {
                    while (data[index] && data[index] === "\\") {
                        index += 1;
                        const char = data[index];
                        if (char === "n") {
                            value += "\n";
                        } else {
                            value += char;
                        }
                        index += 1;
                    }
                    if (data[index] !== '"' || multiLine) {
                        value += data[index];
                        index += 1;
                    }
                }
                if (data[index] !== '"') {
                    fail("Bad end of string");
                }
                index += multiLine ? 3 : 1;
                return value;
            }
            return "";
        }

        function parseNumber(): number {
            let value = "";
            while (data[index]) {
                if (isNumber(data[index]) || data[index] === ".") {
                    value += data[index];
                    index += 1;
                } else {
                    const n = Number(value);
                    if (Number.isNaN(n)) {
                        fail(`Invalid number at ${index}`);
                    }
                    return n;
                }
            }
            fail("Bad end of number");
        }

        function parseValue(): CS2KeyValue3Value {
            if (data.slice(index).startsWith("null")) {
                index += 4;
                return null;
            }
            if (data.slice(index).startsWith("true")) {
                index += 4;
                return true;
            }
            if (data.slice(index).startsWith("false")) {
                index += 5;
                return false;
            }
            if (data.slice(index).startsWith("resource:")) {
                index += 9;
                return `resource:${parseString()}`;
            }
            if (data.slice(index).startsWith("resource_name:")) {
                index += 14;
                return `resource:${parseString()}`;
            }
            if (isNumber(data[index])) {
                return parseNumber();
            }
            if (data[index] === '"') {
                return parseString();
            }
            if (data[index] === "[") {
                index += 1;
                return parseArray();
            }
            if (data[index] === "{") {
                index += 1;
                return parsePairs();
            }
            if (data[index] === "}") {
                return "";
            }

            printDebug();
            fail(`Unexpected character at index ${index}`);
        }

        function parseArray(): CS2KeyValue3Value[] {
            const values: CS2KeyValue3Value[] = [];
            while (data[index]) {
                skipWhitespace();
                if (data[index] === "]") {
                    index += 1;
                    return values;
                }
                skipWhitespace();
                if (data[index] === ",") {
                    index += 1;
                    skipWhitespace();
                    continue;
                }
                values.push(parseValue());
            }
            return values;
        }

        function parsePairs(): { [key: string]: CS2KeyValue3Value } {
            const result: { [key: string]: CS2KeyValue3Value } = {};
            while (data[index]) {
                if (data[index] === "{") {
                    index += 1;
                    continue;
                }
                if (data[index] === "}") {
                    index += 1;
                    return result;
                }
                skipWhitespace();
                const key = parseKey();
                skipWhitespace();
                const value = parseValue();
                skipWhitespace();
                result[key] = value;
            }
            return result;
        }

        skipHeader();
        return parsePairs() as T;
    }
}
