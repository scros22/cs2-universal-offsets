/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Ian Lucas. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/

import { stringify } from "javascript-stringify";
import { CS2Item } from "../src/economy-types";
import { CS2_ITEMS } from "../src/items";
import { ensure } from "../src/utils";
import { readJson, shouldRun, write } from "./utils";

const repoBaseUrl = "https://raw.githubusercontent.com/ianlucas/cs2-lib/main";

async function fetchFromRepo<T = any>(path: string): Promise<T> {
    return await (await fetch(`${repoBaseUrl}/${path}`)).json();
}

let report = "";
let itemDiffs: string[] = [];

const replacer: Parameters<typeof stringify>[1] = (value, _, stringify) => {
    if (typeof value === "string") {
        return '"' + value.replace(/"/g, '\\"') + '"';
    }
    return stringify(value);
};

function isDifferent(a: any, b: any) {
    if (Array.isArray(a) && Array.isArray(b)) {
        return a.length !== b.length || a.some((value, index) => value !== b[index]);
    } else if (typeof a === "object" && typeof b === "object") {
        const aKeys = Object.keys(a);
        const bKeys = Object.keys(b);
        if (aKeys.length !== bKeys.length) {
            return true;
        }
        for (const key of aKeys) {
            if (isDifferent(a[key], b[key])) {
                return true;
            }
        }
        return false;
    }
    return a !== b;
}

function getPropChanges(title: string, localItem: any, repoItem: any) {
    let changes = "";
    const addedProperties = Object.keys(localItem).filter((key) => !Object.keys(repoItem).includes(key));
    const repoProperties = Object.keys(repoItem);
    const before: any = {};
    const after: any = {};
    for (const property of addedProperties) {
        after[property] = localItem[property];
    }
    for (const property of repoProperties) {
        if (isDifferent(repoItem[property], localItem[property])) {
            before[property] = repoItem[property];
            after[property] = localItem[property];
        }
    }
    if (Object.keys(before).length > 0 || Object.keys(after).length > 0) {
        changes += `### ${title}\n\n`;
        if (Object.keys(before).length > 0) {
            changes += "#### Before\n\n";
            changes += "```javascript\n";
            changes += stringify(before, replacer, 2);
            changes += "\n```\n";
        }
        if (Object.keys(after).length > 0) {
            changes += "#### After\n\n";
            changes += "```javascript\n";
            changes += stringify(after, replacer, 2);
            changes += "\n```\n";
        }
    }
    return changes;
}

async function main() {
    const repoItems = new Map(
        (await fetchFromRepo<CS2Item[]>("scripts/data/items.json")).map((item) => [item.id, item])
    );
    const localItems = new Map(CS2_ITEMS.map((item) => [item.id, item]));
    const repoEnglish = await fetchFromRepo("scripts/data/english.json");
    const localEnglish = readJson<any>("scripts/data/english.json");

    const addedKeys = Array.from(localItems.keys()).filter((key) => !repoItems.has(key));
    const removedKeys = Array.from(repoItems.keys()).filter((key) => !localItems.has(key));
    const repoKeysWithoutRemovedKeys = Array.from(repoItems.keys()).filter((key) => !removedKeys.includes(key));

    if (addedKeys.length > 0) {
        report += "# Added Items\n\n";
        for (const addedKey of addedKeys) {
            const name = ensure(localEnglish[addedKey]).name;
            report += `* ${name} (id: ${addedKey})\n`;
        }
        report += "\n";
    }

    if (removedKeys.length > 0) {
        report += "# Removed Items\n\n";
        for (const removedKey of removedKeys) {
            const name = ensure(repoEnglish[removedKey]).name;
            report += `* ${name} (id: ${removedKey})\n`;
        }
        report += "\n";
    }

    for (const key of repoKeysWithoutRemovedKeys) {
        const repoItem = ensure(repoItems.get(key));
        const localItem = ensure(localItems.get(key));
        const repoTranslation = ensure(repoEnglish[key]);
        const localTranslation = ensure(localEnglish[key]);
        const name = `${repoTranslation.name || localTranslation.name} (id: ${key})`;
        let itemChanges = getPropChanges("Item Changes", localItem, repoItem);
        let translationChanges = getPropChanges("Translation Changes", localTranslation, repoTranslation);
        if (itemChanges || translationChanges) {
            itemDiffs.push(`## ${name}\n\n${itemChanges}\n${translationChanges}`);
        }
    }

    if (itemDiffs.length > 0) {
        report += "# Item Changes\n\n";
        report += itemDiffs.join("\n\n");
    }

    if (report.trim() !== "") {
        const timestamp = new Date().toISOString().replace(/:/g, "").replace(/-/g, "").split(".")[0];
        write(`scripts/.changelogs/${timestamp}.md`, report);
    }
}

if (shouldRun(import.meta.url)) {
    main().catch(console.error);
}
