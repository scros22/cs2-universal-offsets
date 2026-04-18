/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Ian Lucas. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/

import { ensure } from "./utils.ts";

export interface CS2Map {
    mapname: string;
    name: string;
}

export const CS2_ANCIENT_MAP: CS2Map = {
    mapname: "de_ancient",
    name: "Ancient"
};

export const CS2_ANUBIS_MAP: CS2Map = {
    mapname: "de_anubis",
    name: "Anubis"
};

export const CS2_INFERNO_MAP: CS2Map = {
    mapname: "de_inferno",
    name: "Inferno"
};

export const CS2_MIRAGE_MAP: CS2Map = {
    mapname: "de_mirage",
    name: "Mirage"
};

export const CS2_DUST2_MAP: CS2Map = {
    mapname: "de_dust2",
    name: "Dust 2"
};

export const CS2_NUKE_MAP: CS2Map = {
    mapname: "de_nuke",
    name: "Nuke"
};

export const CS2_OVERPASS_MAP: CS2Map = {
    mapname: "de_overpass",
    name: "Overpass"
};

export const CS2_VERTIGO_MAP: CS2Map = {
    mapname: "de_vertigo",
    name: "Vertigo"
};

export const CS2_TRAIN_MAP: CS2Map = {
    mapname: "de_train",
    name: "Train"
};

export const CS2_ACTIVE_MAP_POOL: CS2Map[] = [
    CS2_ANCIENT_MAP,
    CS2_ANUBIS_MAP,
    CS2_DUST2_MAP,
    CS2_INFERNO_MAP,
    CS2_MIRAGE_MAP,
    CS2_NUKE_MAP,
    CS2_TRAIN_MAP
];

export const CS2_ALL_MAPS: CS2Map[] = [
    CS2_ANCIENT_MAP,
    CS2_ANUBIS_MAP,
    CS2_DUST2_MAP,
    CS2_INFERNO_MAP,
    CS2_MIRAGE_MAP,
    CS2_NUKE_MAP,
    CS2_TRAIN_MAP,
    CS2_OVERPASS_MAP,
    CS2_VERTIGO_MAP
];

export function getMapnameName(mapname: string): string {
    return ensure(CS2_ALL_MAPS.find((map) => map.mapname.includes(mapname))?.name);
}
