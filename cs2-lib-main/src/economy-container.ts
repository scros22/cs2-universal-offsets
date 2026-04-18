/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Ian Lucas. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/

import { type EnumValues } from "./utils.ts";

export const CS2Rarity = {
    Common: "common",
    Uncommon: "uncommon",
    Rare: "rare",
    Mythical: "mythical",
    Legendary: "legendary",
    Ancient: "ancient",
    Immortal: "immortal"
} as const;

export type CS2RarityKeys = keyof typeof CS2Rarity;
export type CS2RarityValues = EnumValues<typeof CS2Rarity>;

export const CS2RarityColor = {
    Default: "#ded6cc",
    Common: "#b0c3d9",
    Uncommon: "#5e98d9",
    Rare: "#4b69ff",
    Mythical: "#8847ff",
    Legendary: "#d32ce6",
    Ancient: "#eb4b4b",
    Immortal: "#e4ae39"
} as const;

export type CS2RarityColorValues = EnumValues<typeof CS2RarityColor>;

export const CS2_RARITIES: ("common" | "uncommon" | "rare" | "mythical" | "legendary" | "ancient" | "immortal")[] =
    Object.values(CS2Rarity);
export const CS2_RARITY_ORDER: readonly [
    ...("common" | "uncommon" | "rare" | "mythical" | "legendary" | "ancient" | "immortal")[],
    "special"
] = [...CS2_RARITIES, "special"] as const;

export const CS2RarityColorName: {
    readonly "#ded6cc": "common";
    readonly "#b0c3d9": "common";
    readonly "#5e98d9": "uncommon";
    readonly "#4b69ff": "rare";
    readonly "#8847ff": "mythical";
    readonly "#d32ce6": "legendary";
    readonly "#eb4b4b": "ancient";
    readonly "#e4ae39": "immortal";
} = {
    [CS2RarityColor.Default]: CS2Rarity.Common,
    [CS2RarityColor.Common]: CS2Rarity.Common,
    [CS2RarityColor.Uncommon]: CS2Rarity.Uncommon,
    [CS2RarityColor.Rare]: CS2Rarity.Rare,
    [CS2RarityColor.Mythical]: CS2Rarity.Mythical,
    [CS2RarityColor.Legendary]: CS2Rarity.Legendary,
    [CS2RarityColor.Ancient]: CS2Rarity.Ancient,
    [CS2RarityColor.Immortal]: CS2Rarity.Immortal
} as const;

export const CS2RaritySoundName: {
    readonly "#ded6cc": "common";
    readonly "#b0c3d9": "common";
    readonly "#5e98d9": "uncommon";
    readonly "#4b69ff": "rare";
    readonly "#8847ff": "mythical";
    readonly "#d32ce6": "legendary";
    readonly "#eb4b4b": "ancient";
    readonly "#e4ae39": "ancient";
} = {
    [CS2RarityColor.Default]: CS2Rarity.Common,
    [CS2RarityColor.Common]: CS2Rarity.Common,
    [CS2RarityColor.Uncommon]: CS2Rarity.Uncommon,
    [CS2RarityColor.Rare]: CS2Rarity.Rare,
    [CS2RarityColor.Mythical]: CS2Rarity.Mythical,
    [CS2RarityColor.Legendary]: CS2Rarity.Legendary,
    [CS2RarityColor.Ancient]: CS2Rarity.Ancient,
    [CS2RarityColor.Immortal]: CS2Rarity.Ancient
} as const;

export type CS2RaritySoundNameValues = EnumValues<typeof CS2RaritySoundName>;

export const CS2RarityColorOrder: {
    readonly "#ded6cc": 1;
    readonly "#b0c3d9": 1;
    readonly "#5e98d9": 2;
    readonly "#4b69ff": 3;
    readonly "#8847ff": 4;
    readonly "#d32ce6": 5;
    readonly "#eb4b4b": 6;
    readonly "#e4ae39": 7;
} = {
    [CS2RarityColor.Default]: 1,
    [CS2RarityColor.Common]: 1,
    [CS2RarityColor.Uncommon]: 2,
    [CS2RarityColor.Rare]: 3,
    [CS2RarityColor.Mythical]: 4,
    [CS2RarityColor.Legendary]: 5,
    [CS2RarityColor.Ancient]: 6,
    [CS2RarityColor.Immortal]: 7
} as const;

export const CS2_RARITY_COLOR_DEFAULT = 0;
export const CS2_BASE_ODD = 0.8;
export const CS2_STATTRAK_ODD = 0.1;

export function randomFloat(min: number, max: number): number {
    return Math.random() * (max - min) + min;
}

export function randomInt(min: number, max: number): number {
    min = Math.ceil(min);
    max = Math.floor(max);
    return Math.floor(Math.random() * (max - min + 1)) + min;
}

export function randomBoolean(): boolean {
    return Math.random() >= 0.5;
}
