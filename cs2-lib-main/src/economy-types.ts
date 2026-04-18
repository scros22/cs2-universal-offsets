/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Ian Lucas. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/

import type { CS2RarityColorValues, CS2RaritySoundNameValues } from "./economy-container.ts";
import type { EnumValues } from "./utils.ts";

export const CS2ItemTeam = {
    T: 0,
    CT: 1,
    Both: 2
} as const;

export type CS2ItemTeamValues = EnumValues<typeof CS2ItemTeam>;

export const CS2ItemType = {
    Agent: "agent",
    Collectible: "collectible",
    Container: "case",
    Gloves: "glove",
    Graffiti: "graffiti",
    Key: "key",
    Keychain: "keychain",
    Melee: "melee",
    MusicKit: "musickit",
    Patch: "patch",
    Sticker: "sticker",
    Stub: "stub",
    Tool: "tool",
    Utility: "utility",
    Weapon: "weapon"
} as const;

export type CS2ItemTypeValues = EnumValues<typeof CS2ItemType>;

export const CS2ItemWear = {
    FactoryNew: "FN",
    MinimalWear: "MW",
    FieldTested: "FT",
    WellWorn: "WW",
    BattleScarred: "BS"
} as const;

export type CS2ItemWearValues = EnumValues<typeof CS2ItemWear>;

export const CS2ContainerType = {
    WeaponCase: 0,
    StickerCapsule: 1,
    GraffitiBox: 2,
    SouvenirCase: 3
} as const;

export type CS2ContainerTypeValues = EnumValues<typeof CS2ContainerType>;

export interface CS2Item {
    altName?: string | undefined;
    base?: boolean | undefined;
    baseId?: number | undefined;
    category?: string | undefined;
    collection?: string | undefined;
    collectionImage?: string | undefined;
    containerType?: CS2ContainerTypeValues | undefined;
    contents?: number[] | undefined;
    def?: number | undefined;
    free?: boolean | undefined;
    id: number;
    image?: string | undefined;
    index?: number | undefined;
    keys?: number[] | undefined;
    legacy?: boolean | undefined;
    legacyStickerSlots?: number | undefined;
    model?: string | undefined;
    modelBinary?: string | undefined;
    rarity?: CS2RarityColorValues | undefined;
    specials?: number[] | undefined;
    specialsImage?: string | undefined;
    statTrakless?: boolean | undefined;
    statTrakOnly?: boolean | undefined;
    stickerId?: number | undefined;
    stickerIndex?: number | undefined;
    stickerSlots?: number | undefined;
    teams?: CS2ItemTeamValues | undefined;
    textureImage?: string | undefined;
    tint?: number | undefined;
    type: CS2ItemTypeValues;
    voFallback?: boolean | undefined;
    voFemale?: boolean | undefined;
    voPrefix?: string | undefined;
    wearMax?: number | undefined;
    wearMin?: number | undefined;
}

export interface CS2ItemTranslation {
    category?: string | undefined;
    collectionDesc?: string | undefined;
    collectionName?: string | undefined;
    desc?: string | undefined;
    name: string;
    tournamentDesc?: string | undefined;
}

export type CS2ItemTranslationMap = Record<string, CS2ItemTranslation | undefined>;
export type CS2ItemTranslationByLanguage = Record<string, CS2ItemTranslationMap>;

export type CS2StickerMarkup = Record<
    string,
    { slot: number; offsets: number[]; scale: number; rotation: number; legacy?: boolean }[] | undefined
>;

export interface CS2UnlockedItem {
    attributes: {
        containerId: number;
        seed: number | undefined;
        statTrak: number | undefined;
        wear: number | undefined;
    };
    id: number;
    rarity: CS2RaritySoundNameValues;
    special: boolean;
}
