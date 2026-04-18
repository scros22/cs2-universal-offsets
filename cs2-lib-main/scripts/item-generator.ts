/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Ian Lucas. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/

import * as BunnyStorageSDK from "@bunny.net/storage-sdk";
import { createReadStream } from "fs";
import { copyFile, mkdir, readdir, readFile } from "fs/promises";
import { join } from "path";
import sharp from "sharp";
import { Readable } from "stream";
import { stripHtml } from "string-strip-html";
import { format } from "util";
import { CS2_DEFAULT_MAX_WEAR, CS2_DEFAULT_MIN_WEAR } from "../src/economy-constants.ts";
import { CS2RarityColorValues } from "../src/economy-container.ts";
import {
    CS2ContainerType,
    CS2Item,
    CS2ItemTeam,
    CS2ItemTeamValues,
    CS2ItemTranslation,
    CS2ItemTranslationByLanguage,
    CS2ItemType,
    CS2ItemTypeValues,
    CS2StickerMarkup
} from "../src/economy-types.ts";
import { CS2KeyValues } from "../src/keyvalues.ts";
import { CS2KeyValues3 } from "../src/keyvalues3.ts";
import { assert, ensure, fail, isNotUndefined } from "../src/utils.ts";
import { CS2, DECOMPILED_DIR, SCRIPTS_DIR, WORKDIR_DIR } from "./cs2.ts";
import { CS2_CSGO_PATH, INPUT_TEXTURES, STORAGE_ACCESS_KEY, STORAGE_ZONE } from "./env.ts";
import { HARDCODED_SPECIALS } from "./item-generator-specials.ts";
import { useItemsTemplate, useStickerMarkupTemplate, useTranslationTemplate } from "./item-generator-templates.ts";
import {
    CompositeMaterialData,
    CS2ExportItem,
    CS2ExtendedItem,
    CS2GameItems,
    CS2Language,
    MaterialData,
    ModelData
} from "./item-generator-types.ts";
import {
    exists,
    getBufferSha256,
    getFileSha256,
    getFilesSha256,
    log,
    logOnce,
    prependHash,
    PromiseQueue,
    readJson,
    rmIfExists,
    shouldRun,
    warning,
    write,
    writeJson
} from "./utils.ts";

const AGENTS_SOUNDEVENTS_PATH = join(CS2_CSGO_PATH, "soundevents/vo/agents");
const GAME_IMAGES_DIR = join(CS2_CSGO_PATH, "panorama/images");
const GAME_ITEMS_PATH = join(CS2_CSGO_PATH, "scripts/items/items_game.txt");
const GAME_RESOURCE_DIR = join(CS2_CSGO_PATH, "resource");
const OUTPUT_DIR = join(WORKDIR_DIR, "output");

const ITEM_IDS_JSON_PATH = "scripts/data/items-ids.json";
const ITEMS_JSON_PATH = "scripts/data/items.json";
const ITEMS_TS_PATH = "src/items.ts";
const STICKER_MARKUP_TS_PATH = "src/sticker-markup.ts";
const TRANSLATIONS_TS_PATH = "src/translations/%s.ts";
const ENGLISH_JSON_PATH = "scripts/data/english.json";
const CONTAINER_SPECIALS_JSON_PATH = "scripts/data/container-specials.json";

const FORMATTED_STRING_RE = /%s(\d+)/g;
const LANGUAGE_FILE_RE = /csgo_([^\._]+)\.txt$/;
const LOOT_ITEM_RE = /^\[([^\]]+)\](.*)$/;
const SKIN_PHASE_RE = /_phase(\d)/;
const WEAPON_CATEGORY_RE = /(c4|[^\d]+)/;

const BASE_WEAPON_EQUIPMENT = ["weapon_taser"];
const FREE_MUSIC_KITS = ["1", "70"];
const HEAVY_WEAPONS = ["weapon_m249", "weapon_mag7", "weapon_negev", "weapon_nova", "weapon_sawedoff", "weapon_xm1014"];
const MELEE_OR_GLOVES_TYPES: CS2ItemTypeValues[] = [CS2ItemType.Melee, CS2ItemType.Gloves];
const PAINT_IMAGE_SUFFIXES = ["light", "medium", "heavy"] as const;
// prettier-ignore
const UNCATEGORIZED_STICKERS = ["community_mix01", "community02", "danger_zone", "standard", "stickers2", "tournament_assets"];
const REMOVE_KEYCHAIN_TOOL_INDEX = "65";

const OUTPUT_IMAGE_QUALITY = 95;
const DECOMPILER_DATA_SEPARATOR = `--- Data for block "DATA" ---`;

export class ItemHelper extends Map<number, CS2Item> {
    constructor() {
        super(readJson<any[]>(ITEMS_JSON_PATH, []).map((item) => [item.id, item]));
    }
}

export class ItemIdentityHelper {
    allIdentifiers = readJson<string[]>(ITEM_IDS_JSON_PATH, []);
    uniqueIdentifiers: string[] = [];

    get(identifier: string) {
        assert(!this.uniqueIdentifiers.includes(identifier));
        this.uniqueIdentifiers.push(identifier);
        const index = this.allIdentifiers.indexOf(identifier);
        if (index === -1) {
            this.allIdentifiers.push(identifier);
            return this.allIdentifiers.length - 1;
        }
        return index;
    }
}

export class ContainerSpecialsHelper {
    private data = readJson<Record<string, string[]>>(CONTAINER_SPECIALS_JSON_PATH, {});
    private specials: Record<string, number[] | undefined> = {};

    populate(items: (readonly [string, CS2ExtendedItem])[]) {
        const lookup: Record<string, number> = {};
        for (const [name, item] of items) {
            if (MELEE_OR_GLOVES_TYPES.includes(item.type)) {
                lookup[item.base ? `${name} | ★ (Vanilla)` : name] = item.id;
            }
        }
        for (const [containerName, specials] of Object.entries(this.data)) {
            this.specials[containerName] = specials.map((name) =>
                ensure(lookup[name], `Failed to find ${name} in lookup.`)
            );
        }
    }

    getSpecials(containerName: string) {
        return this.specials[containerName];
    }
}

export class ItemGenerator {
    gameItemsAsText: string = null!;
    gameItems: CS2GameItems["items_game"] = null!;

    private csgoTranslationByLanguage: Record<string, CS2Language["lang"]["Tokens"]> = null!;
    private itemTranslationByLanguage: CS2ItemTranslationByLanguage = null!;
    private itemNames = new Map<number, string>();
    private itemSetImage: Record<string, string | undefined> = null!;
    private itemSetItemKey: Record<string, string | undefined> = null!;
    private itemsRaritiesColorHex: typeof this.raritiesColorHex = null!;
    private paintKitsRaritiesColorHex: typeof this.raritiesColorHex = null!;
    private raritiesColorHex: Record<string, string | undefined> = null!;
    private staticAssets: Record<string, string | undefined> = null!;

    private containerSpecialsHelper = new ContainerSpecialsHelper();
    private itemIdentityHelper = new ItemIdentityHelper();
    private itemHelper = new ItemHelper();

    private cs2 = new CS2();

    private baseItems: CS2ExtendedItem[] = [];
    private containerItems = new Map<string, number>();
    private items = new Map<number, CS2ExtendedItem>();

    private stickerMarkup: CS2StickerMarkup = {};

    private paintKits: {
        className: string;
        compositeMaterialPath?: string;
        descToken?: string;
        index: number;
        isLegacy: boolean;
        nameToken: string;
        rarityColorHex: string;
        wearMax: number;
        wearMin: number;
    }[] = null!;

    private graffitiTints: {
        hexColor: string;
        id: number;
        name: string;
        nameToken: string;
    }[] = null!;

    private keychainBaseId: number = null!;

    async run() {
        await this.start();
        await this.readCsgoLanguageFiles();
        await this.readItemsGameFile();
        await this.parseBaseWeapons();
        await this.parseBaseMelees();
        await this.parseBaseGloves();
        await this.parseUtilities();
        await this.parsePaintKits();
        await this.parseMusicKits();
        await this.parseKeychains();
        await this.parseStickers();
        await this.parseGraffiti();
        await this.parsePatches();
        await this.parseAgents();
        await this.parseCollectibles();
        await this.parseTools();
        await this.parseContainers();
        await this.uploadAssets();
        await this.end();
    }

    async start() {
        await rmIfExists(OUTPUT_DIR);
        this.staticAssets = {};
        const folders = ["images", "models", "textures"];
        for (const folder of folders) {
            const directory = join(SCRIPTS_DIR, folder);
            const outputDirectory = join(OUTPUT_DIR, folder);
            await mkdir(outputDirectory, { recursive: true });
            if (folder === "textures") {
                continue;
            }
            const filenames = await readdir(directory);
            for (const filename of filenames) {
                const path = join(directory, filename);
                if (filename.endsWith(".png")) {
                    const key = `/images/${filename}`;
                    const value = await this.copyAndOptimizeImage(path, "/images/{sha256}.webp");
                    this.staticAssets[key] = value;
                } else if (filename.endsWith(".glb")) {
                    const sha256 = await getFileSha256(path);
                    const key = `/models/${filename}`;
                    const value = `/models/${sha256}.glb`;
                    await copyFile(path, join(OUTPUT_DIR, value));
                    this.staticAssets[key] = value;
                } else {
                    await copyFile(path, join(outputDirectory, filename));
                }
            }
        }
    }

    async readCsgoLanguageFiles(include?: string[]) {
        this.itemTranslationByLanguage = {};
        this.csgoTranslationByLanguage = Object.fromEntries(
            await Promise.all(
                (await readdir(GAME_RESOURCE_DIR))
                    .map((file) => {
                        const matches = file.match(LANGUAGE_FILE_RE);
                        return matches !== null ? ([file, matches[1]] as const) : undefined;
                    })
                    .filter(isNotUndefined)
                    .filter(([_, language]) => include === undefined || include.includes(language))
                    .map(async ([file, language]) => {
                        this.itemTranslationByLanguage[language] = {};
                        return [
                            language,
                            Object.entries(
                                CS2KeyValues.parse<CS2Language>(await readFile(join(GAME_RESOURCE_DIR, file), "utf-8"))
                                    .lang.Tokens
                            ).reduce((tokens, [key, value]) => {
                                key = key.toLowerCase();
                                assert(tokens[key] === undefined);
                                tokens[key] = value;
                                return tokens;
                            }, {} as any)
                        ];
                    })
            )
        );
        const { length } = Object.keys(this.csgoTranslationByLanguage);
        assert(length > 0);
        assert(this.csgoTranslationByLanguage.english !== undefined);
        warning(`Loaded ${length} language(s).`);
    }

    async readItemsGameFile() {
        this.gameItemsAsText = await readFile(GAME_ITEMS_PATH, "utf-8");
        this.gameItems = CS2KeyValues.parse<CS2GameItems>(this.gameItemsAsText).items_game;
        this.raritiesColorHex = Object.fromEntries(
            Object.entries(this.gameItems.rarities).map(([rarityKey, { color }]) => {
                return [rarityKey, ensure(this.gameItems.colors[ensure(color)]?.hex_color)] as const;
            })
        );
        assert(this.raritiesColorHex.default !== undefined);
        assert(this.raritiesColorHex.common !== undefined);
        assert(this.raritiesColorHex.rare !== undefined);
        this.paintKitsRaritiesColorHex = Object.fromEntries(
            Object.entries(this.gameItems.paint_kits_rarity).map(([paintKitKey, rarityKey]) => {
                return [paintKitKey, this.raritiesColorHex[rarityKey]] as const;
            })
        );
        const rarityKeys = Object.keys(this.raritiesColorHex);
        this.itemsRaritiesColorHex = Object.fromEntries(
            // Mapping rarities for items inside loot lists. Looks like this is
            // the actual rarity of the item, then we fallback to paint, or
            // rarity defined in the item itself.
            Object.entries(this.gameItems.client_loot_lists)
                .map(([clientLootListKey, clientLootList]) => {
                    const rarityKey = rarityKeys.find((rarityKey) => clientLootListKey.includes(`_${rarityKey}`));
                    return rarityKey !== undefined
                        ? Object.keys(clientLootList)
                              .map((itemOrClientLootListKey) =>
                                  itemOrClientLootListKey.includes("customplayer_") ||
                                  LOOT_ITEM_RE.test(itemOrClientLootListKey)
                                      ? ([itemOrClientLootListKey, this.raritiesColorHex[rarityKey]] as const)
                                      : undefined
                              )
                              .filter(isNotUndefined)
                        : undefined;
                })
                .filter(isNotUndefined)
                .flat()
        );
        this.paintKits = Object.entries(this.gameItems.paint_kits)
            .map(
                ([
                    paintKitIndex,
                    {
                        composite_material_path,
                        description_string,
                        description_tag,
                        name,
                        use_legacy_model,
                        wear_remap_max,
                        wear_remap_min
                    }
                ]) => {
                    assert(name);
                    if (name === "default" || description_tag === undefined) {
                        return undefined;
                    }
                    return {
                        className: name,
                        compositeMaterialPath: composite_material_path,
                        descToken: prependHash(description_string),
                        index: Number(paintKitIndex),
                        isLegacy: use_legacy_model === "1",
                        nameToken: prependHash(description_tag),
                        rarityColorHex: this.getRarityColorHex([name]),
                        wearMax: wear_remap_max !== undefined ? Number(wear_remap_max) : CS2_DEFAULT_MAX_WEAR,
                        wearMin: wear_remap_min !== undefined ? Number(wear_remap_min) : CS2_DEFAULT_MIN_WEAR
                    };
                }
            )
            .filter(isNotUndefined);
        this.graffitiTints = Object.values(this.gameItems.graffiti_tints).map(({ id, hex_color }) => ({
            id: Number(id),
            name: this.requireTranslation(`#Attrib_SprayTintValue_${id}`),
            nameToken: `#Attrib_SprayTintValue_${id}`,
            hexColor: hex_color
        }));
        this.itemSetImage = {};
        this.itemSetItemKey = Object.fromEntries(
            (
                await Promise.all(
                    Object.entries(this.gameItems.item_sets).map(async ([itemSetKey, { items }]) => {
                        return await Promise.all(
                            Object.keys(items).map(async (itemKey) => {
                                if (this.itemSetImage[itemSetKey] === undefined) {
                                    this.itemSetImage[itemSetKey] = await this.getCollectionImage(itemSetKey);
                                }
                                return [itemKey, itemSetKey] as const;
                            })
                        );
                    })
                )
            ).flat()
        );
    }

    private async parseBaseWeapons() {
        warning("Parsing base weapons...");
        for (const [itemDef, { baseitem, flexible_loadout_slot, name, prefab, image_inventory }] of Object.entries(
            this.gameItems.items
        )) {
            if (baseitem !== "1" || flexible_loadout_slot === undefined) {
                continue;
            }
            const category = flexible_loadout_slot.match(WEAPON_CATEGORY_RE)?.[1];
            if (category === undefined || (category === "equipment" && !BASE_WEAPON_EQUIPMENT.includes(name))) {
                continue;
            }
            const { used_by_classes, item_name, item_description, model_player } = this.getPrefab(prefab);
            const teams = this.getTeams(used_by_classes);
            const id = this.itemIdentityHelper.get(`weapon_${this.getTeamsString(used_by_classes)}_${itemDef}`);
            const stickerMarkup = await this.findStickerMarkup(itemDef, model_player);
            this.addTranslation(id, "name", item_name);
            this.addTranslation(id, "desc", item_description);
            this.addItem({
                base: true,
                category: this.getBaseWeaponCategory(name, category),
                className: name,
                def: Number(itemDef),
                descToken: item_description,
                free: true,
                id,
                image:
                    image_inventory !== undefined
                        ? await this.getImage(image_inventory)
                        : await this.getBaseImage(id, name),
                index: undefined,
                legacyStickerSlots: stickerMarkup?.filter((m) => m.legacy).length,
                model: name.replace("weapon_", ""),
                modelBinary: this.staticAssets[`/models/${itemDef}.glb`],
                nameToken: item_name,
                rarity: this.getRarityColorHex(["default"]),
                stickerSlots: stickerMarkup?.filter((m) => !m.legacy).length,
                teams,
                type: CS2ItemType.Weapon
            });
        }
    }

    private async parseBaseMelees() {
        warning("Parsing base melee...");
        for (const [
            itemDef,
            { item_name, image_inventory, item_description, name, used_by_classes, prefab, baseitem }
        ] of Object.entries(this.gameItems.items)) {
            if (
                item_name === undefined ||
                image_inventory === undefined ||
                used_by_classes === undefined ||
                (prefab === "melee" && baseitem !== "1") ||
                !prefab?.includes("melee") ||
                prefab?.includes("noncustomizable") ||
                !this.hasTranslation(item_name)
            ) {
                continue;
            }
            const thePrefab = this.getPrefab(prefab);
            const teams = this.getTeams(used_by_classes);
            const id = this.itemIdentityHelper.get(`melee_${this.getTeamsString(used_by_classes)}_${itemDef}`);
            this.addTranslation(id, "name", item_name);
            this.addTranslation(id, "desc", item_description);
            this.addItem({
                base: true,
                className: name,
                def: Number(itemDef),
                descToken: item_description,
                free: baseitem === "1" ? true : undefined,
                id,
                image: await this.getImage(image_inventory),
                index: baseitem === "1" ? undefined : 0,
                model: name.replace("weapon_", ""),
                nameToken: item_name,
                rarity: this.getRarityColorHex([thePrefab.item_rarity], "default"),
                teams,
                type: CS2ItemType.Melee
            });
        }
    }

    private async parseBaseGloves() {
        warning("Parsing base gloves...");
        for (const [
            itemDef,
            { item_name, baseitem, name, prefab, image_inventory, item_description, used_by_classes }
        ] of Object.entries(this.gameItems.items)) {
            if (item_name === undefined || !prefab?.includes("hands")) {
                continue;
            }
            const teams = this.getTeams(used_by_classes, CS2ItemTeam.Both);
            const id = this.itemIdentityHelper.get(`glove_${this.getTeamsString(used_by_classes, "3_2")}_${itemDef}`);
            this.addTranslation(id, "name", item_name);
            this.addTranslation(id, "desc", item_description);
            this.addItem({
                base: true,
                className: name,
                def: Number(itemDef),
                descToken: item_description,
                free: baseitem === "1" ? true : undefined,
                id,
                image:
                    image_inventory !== undefined
                        ? await this.getImage(image_inventory)
                        : this.requireStaticAsset(`/images/${name}.png`),
                index: baseitem === "1" ? undefined : 0,
                model: name,
                nameToken: item_name,
                rarity: this.getRarityColorHex([baseitem === "1" ? "default" : "ancient"]),
                teams,
                type: CS2ItemType.Gloves
            });
        }
    }

    private async parseUtilities() {
        warning("Parsing utilities...");
        for (const [itemDef, { flexible_loadout_slot, name, prefab, image_inventory }] of Object.entries(
            this.gameItems.items
        )) {
            if (!flexible_loadout_slot?.startsWith("grenade")) {
                continue;
            }
            const { item_name, item_description } = this.getPrefab(prefab);
            const id = this.itemIdentityHelper.get(`utility_${itemDef}`);
            this.addTranslation(id, "name", item_name);
            this.addTranslation(id, "desc", item_description);
            this.addItem({
                base: true,
                className: name,
                def: Number(itemDef),
                descToken: item_description,
                free: true,
                id,
                image:
                    image_inventory !== undefined
                        ? await this.getImage(image_inventory)
                        : await this.getBaseImage(id, name),
                index: undefined,
                model: name.replace("weapon_", ""),
                nameToken: item_name,
                rarity: this.getRarityColorHex(["default"]),
                teams: CS2ItemTeam.Both,
                type: CS2ItemType.Utility
            });
        }
    }

    private async parsePaintKits() {
        warning("Parsing paint kits...");
        for (const paintKit of this.paintKits) {
            for (const baseItem of this.baseItems) {
                if (!(await this.isPaintImageValid(baseItem.className, paintKit.className))) {
                    continue;
                }
                const itemKey = `[${paintKit.className}]${baseItem.className}`;
                if (baseItem.type === CS2ItemType.Weapon && !this.gameItemsAsText.includes(itemKey)) {
                    continue;
                }
                const id = this.itemIdentityHelper.get(`paint_${baseItem.def}_${paintKit.index}`);
                this.addContainerItem(itemKey, id);
                this.addTranslation(id, "name", baseItem.nameToken, " | ", paintKit.nameToken);
                this.addTranslation(id, "desc", paintKit.descToken);
                this.addItem({
                    ...baseItem,
                    ...this.getPaintCollection(id, itemKey),
                    altName: this.getPaintAltName(paintKit.className),
                    base: undefined,
                    baseId: baseItem.id,
                    free: undefined,
                    id,
                    image: await this.getPaintImage(id, baseItem.className, paintKit.className),
                    index: Number(paintKit.index),
                    legacy: (baseItem.type === "weapon" && paintKit.isLegacy) || undefined,
                    legacyStickerSlots: undefined,
                    modelBinary: undefined,
                    rarity: this.getRarityColorHex(
                        MELEE_OR_GLOVES_TYPES.includes(baseItem.type)
                            ? [baseItem.rarity, paintKit.rarityColorHex]
                            : [itemKey, paintKit.rarityColorHex]
                    ),
                    stickerSlots: undefined,
                    textureImage:
                        (await this.getPaintTexture(id, paintKit.className, paintKit.compositeMaterialPath)) ??
                        this.itemHelper.get(id)?.textureImage,
                    wearMax: paintKit.wearMax,
                    wearMin: paintKit.wearMin
                });
            }
        }
    }

    private async parseMusicKits() {
        warning("Parsing music kits...");
        const baseId = this.createStub("musickit", "#CSGO_musickit_desc");
        for (const [index, { name, loc_name, loc_description, image_inventory }] of Object.entries(
            this.gameItems.music_definitions
        )) {
            if (index === "2") {
                // Duplicated CS:GO Music Kit.
                continue;
            }
            const itemKey = `[${name}]musickit`;
            const id = this.itemIdentityHelper.get(`musickit_${index}`);
            const base = FREE_MUSIC_KITS.includes(index) ? true : undefined;
            this.addContainerItem(itemKey, id);
            this.addTranslation(id, "name", "#CSGO_Type_MusicKit", " | ", loc_name);
            this.addTranslation(id, "desc", loc_description);
            this.addItem({
                base,
                baseId,
                def: 1314,
                free: base,
                id,
                image: await this.getImage(image_inventory),
                index: Number(index),
                rarity: this.getRarityColorHex(["rare"]),
                type: CS2ItemType.MusicKit
            });
        }
    }

    private async parseKeychains() {
        warning("Parsing keychains...");
        this.keychainBaseId = this.createStub("keychain", "#CSGO_Tool_Keychain_Desc");
        for (const [index, { name, loc_name, loc_description, item_rarity, image_inventory }] of Object.entries(
            this.gameItems.keychain_definitions
        )) {
            if (!this.hasTranslation(loc_name)) {
                continue;
            }
            if (!(await this.isImageValid(image_inventory))) {
                log(`Inventory image not found for ${image_inventory} (index: ${index})`);
                continue;
            }
            const id = this.itemIdentityHelper.get(`keychain_${index}`);
            const itemKey = `[${name}]keychain`;
            this.addContainerItem(itemKey, id);
            this.addTranslation(id, "name", "#CSGO_Tool_Keychain", " | ", loc_name);
            this.tryAddTranslation(id, "desc", loc_description);
            this.addItem({
                baseId: this.keychainBaseId,
                free: index === "37" ? true : undefined,
                def: 1355,
                id,
                image: await this.getImage(image_inventory),
                index: Number(index),
                rarity: this.getRarityColorHex([itemKey, item_rarity]),
                type: CS2ItemType.Keychain
            });
        }
    }

    private async parseStickers() {
        warning("Parsing stickers...");
        const baseId = this.createStub("sticker", "#CSGO_Tool_Sticker_Desc");
        for (const [
            index,
            { name, description_string, item_name, sticker_material, tournament_event_id, item_rarity }
        ] of Object.entries(this.gameItems.sticker_kits)) {
            if (
                name === "default" ||
                item_name.includes("SprayKit") ||
                name.includes("spray_") ||
                name.includes("patch_") ||
                sticker_material.includes("_graffiti") ||
                !this.hasTranslation(item_name)
            ) {
                continue;
            }
            const [category, categoryToken] = this.getStickerCategory({ sticker_material, tournament_event_id });
            const id = this.itemIdentityHelper.get(`sticker_${index}`);
            const itemKey = `[${name}]sticker`;
            const rarity = this.getRarityColorHex([itemKey, item_rarity]);
            this.addContainerItem(itemKey, id);
            this.addTranslation(id, "name", "#CSGO_Tool_Sticker", " | ", item_name);
            this.addTranslation(id, "category", categoryToken !== undefined ? categoryToken : category);
            this.tryAddTranslation(id, "desc", description_string);
            if (tournament_event_id !== undefined) {
                this.addFormattedTranslation(
                    id,
                    "tournamentDesc",
                    "#CSGO_Event_Desc",
                    `#CSGO_Tournament_Event_Name_${tournament_event_id}`
                );
            }
            this.addItem({
                baseId,
                def: 1209,
                id,
                image: await this.getImage(`econ/stickers/${sticker_material}`),
                index: Number(index),
                rarity,
                type: CS2ItemType.Sticker
            });
            // Sticker Slab
            const keychainImage = `econ/stickers/${sticker_material}_1355_37`;
            if (!(await this.isImageValid(keychainImage))) {
                log(`Failed to find image for sticker slab at ${keychainImage} (index: ${index})`);
                continue;
            }
            const keychainId = this.itemIdentityHelper.get(`keychain_37_${index}`);
            this.addTranslation(keychainId, "name", "#keychain_kc_sticker_display_case", " | ", item_name);
            this.tryAddTranslation(keychainId, "desc", "#keychain_kc_sticker_display_case_desc");
            this.addItem({
                baseId: this.keychainBaseId,
                def: 1355,
                id: keychainId,
                image: await this.getImage(keychainImage),
                index: 37,
                rarity,
                stickerId: id,
                stickerIndex: Number(index),
                type: CS2ItemType.Keychain
            });
        }
    }

    private async parseGraffiti() {
        warning("Parsing graffiti...");
        const baseId = this.createStub("graffiti", "#CSGO_Tool_SprayPaint_Desc");
        for (const [
            index,
            { name, item_name, description_string, sticker_material, item_rarity, tournament_event_id }
        ] of Object.entries(this.gameItems.sticker_kits)) {
            if (
                !this.hasTranslation(item_name) ||
                (!item_name?.includes("#SprayKit") &&
                    item_name?.indexOf("spray_") !== 0 &&
                    !description_string?.includes("#SprayKit") &&
                    !sticker_material?.includes("_graffiti"))
            ) {
                continue;
            }
            const itemKey = `[${name}]spray`;
            if (sticker_material.startsWith("default")) {
                for (const { hexColor, nameToken: tintNameToken, id: tintId } of this.graffitiTints) {
                    const id = this.itemIdentityHelper.get(`spray_${index}_${tintId}`);
                    this.addContainerItem(itemKey, id);
                    this.addTranslation(id, "name", "#CSGO_Type_Spray", " | ", item_name, " (", tintNameToken, ")");
                    this.addTranslation(id, "desc", description_string);
                    this.addItem({
                        baseId,
                        id,
                        image: await this.getDefaultGraffitiImage(id, sticker_material, hexColor),
                        index: Number(index),
                        rarity: this.getRarityColorHex([item_rarity]),
                        tint: tintId,
                        type: CS2ItemType.Graffiti
                    });
                }
                continue;
            }
            const id = this.itemIdentityHelper.get(`spray_${index}`);
            this.addContainerItem(itemKey, id);
            this.addTranslation(id, "name", "#CSGO_Type_Spray", " | ", item_name);
            this.addTranslation(id, "desc", description_string);
            if (tournament_event_id !== undefined) {
                this.addFormattedTranslation(
                    id,
                    "tournamentDesc",
                    "#CSGO_Event_Desc",
                    `#CSGO_Tournament_Event_Name_${tournament_event_id}`
                );
            }
            this.addItem({
                baseId,
                def: 1348,
                id,
                image: await this.getImage(`econ/stickers/${sticker_material}`),
                index: Number(index),
                rarity: this.getRarityColorHex([itemKey, item_rarity]),
                type: CS2ItemType.Graffiti
            });
        }
    }

    private async parsePatches() {
        warning("Parsing patches...");
        const baseId = this.createStub("patch", "#CSGO_Tool_Patch_Desc");
        for (const [
            index,
            { name, item_name, patch_material, description_string, tournament_event_id, item_rarity }
        ] of Object.entries(this.gameItems.sticker_kits)) {
            if (item_name.indexOf("#PatchKit") !== 0 && patch_material === undefined) {
                continue;
            }
            const id = this.itemIdentityHelper.get(`patch_${index}`);
            const itemKey = `[${name}]patch`;
            this.addContainerItem(itemKey, id);
            this.addTranslation(id, "name", "#CSGO_Tool_Patch", " | ", item_name);
            this.addTranslation(id, "desc", description_string);
            if (tournament_event_id !== undefined) {
                this.addFormattedTranslation(
                    id,
                    "tournamentDesc",
                    "#CSGO_Event_Desc",
                    `#CSGO_Tournament_Event_Name_${tournament_event_id}`
                );
            }
            this.addItem({
                baseId,
                def: 4609,
                id,
                image: await this.getImage(`econ/patches/${patch_material}`),
                index: Number(index),
                rarity: this.getRarityColorHex([itemKey, item_rarity]),
                type: CS2ItemType.Patch
            });
        }
    }

    private async parseAgents() {
        warning("Parsing agents...");
        for (const [
            index,
            {
                name,
                item_name,
                vo_prefix,
                used_by_classes,
                image_inventory,
                model_player,
                item_rarity,
                prefab,
                item_description
            }
        ] of Object.entries(this.gameItems.items)) {
            if (
                item_name === undefined ||
                used_by_classes === undefined ||
                image_inventory === undefined ||
                model_player === undefined ||
                prefab !== "customplayertradable"
            ) {
                continue;
            }
            const teams = this.getTeams(used_by_classes);
            const id = this.itemIdentityHelper.get(`agent_${this.getTeamsString(used_by_classes)}_${index}`);
            const model = model_player.replace("characters/models/", "").replace(".vmdl", "");
            const voPrefix = this.getAgentVoPrefix(model_player, vo_prefix);
            this.addTranslation(id, "name", "#Type_CustomPlayer", " | ", item_name);
            this.addTranslation(id, "desc", item_description);
            this.addItem({
                def: Number(index),
                id,
                image: await this.getImage(image_inventory),
                index: undefined,
                model,
                rarity: this.getRarityColorHex([name, item_rarity]),
                teams,
                type: CS2ItemType.Agent,
                voFallback: await this.getAgentVoFallback(voPrefix),
                voFemale: this.getAgentVoFemale(voPrefix),
                voPrefix
            });
        }
    }

    private async parseCollectibles() {
        warning("Parsing collectibles...");
        for (const [
            index,
            { name, image_inventory, item_name, tool, attributes, item_rarity, item_description }
        ] of Object.entries(this.gameItems.items)) {
            if (
                image_inventory === undefined ||
                item_name === undefined ||
                (!image_inventory.includes("/status_icons/") && !image_inventory.includes("/premier_seasons/")) ||
                tool?.use_string === "#ConsumeItem" ||
                attributes?.["set supply crate series"]?.attribute_class === "supply_crate_series" ||
                item_name.indexOf("#CSGO_TournamentPass") === 0 ||
                !attributes?.["pedestal display model"]
            ) {
                continue;
            }
            if (!(await this.isImageValid(image_inventory))) {
                log(`Inventory image not found for ${image_inventory} (index: ${index})`);
                continue;
            }
            const id = this.itemIdentityHelper.get(`pin_${index}`);
            this.addContainerItem(name, id);
            this.addTranslation(id, "name", "#CSGO_Type_Collectible", " | ", item_name);
            this.tryAddTranslation(id, "desc", item_description ?? `${item_name}_Desc`);
            if (attributes?.["tournament event id"] !== undefined) {
                this.addFormattedTranslation(
                    id,
                    "tournamentDesc",
                    "#CSGO_Event_Desc",
                    `#CSGO_Tournament_Event_Name_${attributes?.["tournament event id"].value}`
                );
            }
            this.addItem({
                altName: name,
                def: Number(index),
                id,
                image: await this.getImage(image_inventory),
                index: undefined,
                rarity: this.getRarityColorHex([item_rarity, "ancient"]),
                teams: undefined,
                type: CS2ItemType.Collectible
            });
        }
    }

    private async parseTools() {
        warning("Parsing tools...");
        for (const [index, { name, baseitem, item_name, image_inventory, prefab, item_description }] of Object.entries(
            this.gameItems.items
        )) {
            if (
                prefab !== "recipe" &&
                (item_name === undefined ||
                    image_inventory === undefined ||
                    !image_inventory.includes("econ/tools/") ||
                    !prefab?.includes("csgo_tool"))
            ) {
                continue;
            }
            const id = this.itemIdentityHelper.get(`tool_${index}`);
            const thePrefab = this.gameItems.prefabs[prefab];
            const image = ensure(image_inventory || thePrefab?.image_inventory);
            this.addContainerItem(name, id);
            this.addTranslation(id, "name", "#CSGO_Type_Tool", " | ", item_name);
            this.addTranslation(id, "desc", item_description);
            this.addItem({
                def: Number(index),
                free: baseitem === "1" && index !== REMOVE_KEYCHAIN_TOOL_INDEX ? true : undefined,
                id,
                image: await this.getImage(image),
                index: undefined,
                rarity: this.getRarityColorHex(["common"]),
                teams: undefined,
                type: CS2ItemType.Tool
            });
        }
    }

    private async parseContainers() {
        warning("Parsing containers...");
        this.containerSpecialsHelper.populate(
            Array.from(this.itemNames.entries()).map(([id, name]) => [name, ensure(this.items.get(id))])
        );
        const keyItems = new Map<string, number>();
        for (const [
            containerIndex,
            {
                associated_items,
                attributes,
                image_inventory,
                image_unusual_item,
                item_description,
                item_name,
                loot_list_name,
                name,
                prefab,
                tags,
                tool
            }
        ] of Object.entries(this.gameItems.items)) {
            const hasSupplyCrateSeries =
                attributes?.["set supply crate series"]?.attribute_class === "supply_crate_series";
            if (
                item_name === undefined ||
                image_inventory === undefined ||
                (!image_inventory.includes("econ/weapon_cases") && !hasSupplyCrateSeries) ||
                tool?.type === "gift" ||
                (prefab !== "weapon_case" && !hasSupplyCrateSeries && loot_list_name === undefined)
            ) {
                continue;
            }
            if (!(await this.isImageValid(image_inventory))) {
                log(`Inventory image not found for ${image_inventory} (index: ${containerIndex})`);
                continue;
            }
            const revolvingLootListKey = attributes?.["set supply crate series"]?.value;
            assert(revolvingLootListKey !== undefined || loot_list_name !== undefined);
            const clientLootListKey =
                revolvingLootListKey !== undefined
                    ? this.gameItems.revolving_loot_lists[revolvingLootListKey]
                    : loot_list_name;
            if (clientLootListKey === undefined) {
                continue;
            }
            let contentsType: CS2ItemTypeValues | undefined;
            const contents: number[] = [];
            for (const itemKey of this.getClientLootListItems(clientLootListKey)) {
                const id = ensure(this.containerItems.get(itemKey));
                const item = ensure(this.items.get(id));
                contentsType = item.type;
                if (item.tint !== undefined) {
                    assert(item.index);
                    for (const other of this.items.values()) {
                        if (other.tint !== undefined && other.index === item.index) {
                            contents.push(other.id);
                        }
                    }
                } else {
                    contents.push(id);
                }
            }
            if (contents.length > 0) {
                const thePrefab = this.tryGetPrefab(prefab);
                // Asserts if the container requires a key.
                assert(
                    associated_items !== undefined ||
                        prefab === "sticker_capsule" ||
                        prefab === "weapon_case_souvenirpkg" ||
                        thePrefab?.prefab === "weapon_case_souvenirpkg" ||
                        tags?.StickerCapsule ||
                        name.includes("crate_signature") ||
                        name.includes("crate_pins") ||
                        name.includes("crate_musickit") ||
                        name.includes("crate_patch") ||
                        name.includes("crate_sprays") ||
                        name.includes("selfopeningitem") ||
                        prefab?.includes("selfopening")
                );
                const keys = await Promise.all(
                    Object.keys(associated_items ?? {}).map(async (keyItemDef) => {
                        if (keyItems.has(keyItemDef)) {
                            return ensure(keyItems.get(keyItemDef));
                        }
                        const { item_name, item_description, image_inventory } = this.gameItems.items[keyItemDef];
                        assert(image_inventory);
                        const id = this.itemIdentityHelper.get(`key_${keyItemDef}`);
                        const nameToken = item_name ?? "#CSGO_base_crate_key";
                        keyItems.set(keyItemDef, id);
                        this.addTranslation(id, "name", "#CSGO_Tool_WeaponCase_KeyTag", " | ", nameToken);
                        this.tryAddTranslation(id, "desc", item_description);
                        this.addItem({
                            def: Number(keyItemDef),
                            id,
                            image: await this.getImage(image_inventory),
                            rarity: this.getRarityColorHex(["common"]),
                            teams: undefined,
                            type: CS2ItemType.Key
                        });
                        return id;
                    })
                );
                const containerName = this.requireTranslation(item_name);
                const id = this.itemIdentityHelper.get(`case_${containerIndex}`);
                const specials = this.containerSpecialsHelper.getSpecials(containerName) ?? HARDCODED_SPECIALS[id];
                const containsMusicKit = containerName.includes("Music Kit");
                const containsStatTrak = containerName.includes("StatTrak");
                this.addTranslation(id, "name", "#CSGO_Type_WeaponCase", " | ", item_name);
                this.tryAddTranslation(id, "desc", item_description);
                this.addItem({
                    ...this.getCollection(id, tags?.ItemSet?.tag_value),
                    containerType: this.getContainerType(containerName, contentsType),
                    contents,
                    def: Number(containerIndex),
                    id,
                    image: await this.getImage(image_inventory),
                    keys: keys.length > 0 ? keys : undefined,
                    rarity: this.getRarityColorHex(["common"]),
                    specials: specials ?? this.itemHelper.get(id)?.specials,
                    specialsImage: await this.getSpecialsImage(id, image_unusual_item),
                    statTrakless: containsMusicKit && !containsStatTrak ? true : undefined,
                    statTrakOnly: containsMusicKit && containsStatTrak ? true : undefined,
                    teams: undefined,
                    type: CS2ItemType.Container
                });
            }
        }
    }

    private async uploadAssets() {
        if (STORAGE_ZONE === undefined || STORAGE_ACCESS_KEY === undefined) {
            return log("Asset upload skipped.");
        }
        const sz = BunnyStorageSDK.zone.connect_with_accesskey(
            BunnyStorageSDK.regions.StorageRegion.NewYork,
            STORAGE_ZONE,
            STORAGE_ACCESS_KEY
        );
        const folders = ["images", "textures", "models"];
        const queue = new PromiseQueue(40);
        for (const folder of folders) {
            const fileChecksums = await this.fetchStorageFileChecksums(sz, `/${folder}`);
            const assetsPath = join(OUTPUT_DIR, folder);
            for (const filename of await readdir(assetsPath)) {
                const assetPath = join(assetsPath, filename);
                const cdnPath = `/${folder}/${filename}`;
                if (fileChecksums[cdnPath] === undefined) {
                    queue.push(async () => {
                        await BunnyStorageSDK.file.upload(sz, cdnPath, Readable.toWeb(createReadStream(assetPath)));
                    });
                }
            }
        }
        await queue.waitForIdle();
    }

    private async end() {
        const items: CS2ExportItem[] = Array.from(this.items.values()).map((item) => ({
            ...item,
            className: undefined,
            descToken: undefined,
            nameToken: undefined
        }));

        await writeJson(ITEMS_JSON_PATH, items);
        warning(`Successfully generated '${ITEMS_JSON_PATH}'.`);

        await writeJson(ITEM_IDS_JSON_PATH, this.itemIdentityHelper.allIdentifiers);
        warning(`Successfully generated '${ITEM_IDS_JSON_PATH}'.`);

        for (const [language, translations] of Object.entries(this.itemTranslationByLanguage)) {
            const tsPath = format(TRANSLATIONS_TS_PATH, language);
            await write(tsPath, useTranslationTemplate(language, translations));
            warning(`Successfully generated '${tsPath}'.`);

            if (language === "english") {
                await writeJson(ENGLISH_JSON_PATH, translations);
                warning(`Successfully generated '${ENGLISH_JSON_PATH}'.`);
            }
        }

        await write(ITEMS_TS_PATH, useItemsTemplate(items));
        warning(`Successfully generated '${ITEMS_TS_PATH}'.`);

        if (Object.keys(this.stickerMarkup).length > 0) {
            await write(STICKER_MARKUP_TS_PATH, useStickerMarkupTemplate(this.stickerMarkup));
            warning(`Successfully generated '${STICKER_MARKUP_TS_PATH}'.`);
        }

        warning("Script completed successfully.");
    }

    private async fetchStorageFileChecksums(
        sz: BunnyStorageSDK.StorageZone,
        path: string
    ): Promise<Record<string, string | undefined>> {
        return Object.fromEntries(
            (await BunnyStorageSDK.file.list(sz, path)).map((file) => {
                return [`${file.path.replace(`/${STORAGE_ZONE}`, "")}${file.objectName}`, file.checksum?.toLowerCase()];
            })
        );
    }

    private getRarityColorHex(keywords: (string | undefined)[], defaultsTo?: string) {
        let colorHex =
            defaultsTo !== undefined
                ? defaultsTo.charAt(0) === "#"
                    ? defaultsTo
                    : this.raritiesColorHex[defaultsTo]
                : undefined;
        for (const keyword of keywords) {
            if (keyword === undefined) {
                continue;
            }
            if (keyword.charAt(0) === "#") {
                colorHex = keyword;
                break;
            }
            colorHex =
                this.itemsRaritiesColorHex[keyword] ??
                this.paintKitsRaritiesColorHex[keyword] ??
                this.raritiesColorHex[keyword];
            if (colorHex !== undefined) {
                break;
            }
        }
        if (colorHex === undefined) {
            colorHex = this.raritiesColorHex.default;
        }
        return ensure(colorHex) as CS2RarityColorValues;
    }

    private resolveToken(token?: string) {
        return (token?.charAt(0) === "#" ? token.substring(1) : token)?.toLowerCase();
    }

    private isTranslationKey(token?: string) {
        if (token === undefined || token.length === 0) {
            return false;
        }
        const resolved = this.resolveToken(token);
        return resolved !== undefined && this.csgoTranslationByLanguage.english[resolved] !== undefined;
    }

    private findTranslation(token?: string, language = "english") {
        token = this.resolveToken(token);
        if (token === undefined) {
            return undefined;
        }
        const value = this.csgoTranslationByLanguage[language][token];
        return value !== undefined ? stripHtml(value).result : undefined;
    }

    private requireTranslation(token?: string, language = "english") {
        return ensure(
            this.findTranslation(token, language),
            `Failed to find translation for '${token}' (${language}).`
        );
    }

    private hasTranslation(token?: string) {
        token = this.resolveToken(token);
        return token !== undefined && this.csgoTranslationByLanguage.english[token] !== undefined;
    }

    private addTranslation(id: number, property: keyof CS2ItemTranslation, ...tokens: (string | undefined)[]) {
        for (const [language, items] of Object.entries(this.itemTranslationByLanguage)) {
            const itemLanguage = (items[id] ??= {} as CS2ItemTranslation);
            const string = tokens
                .map((token) => {
                    assert(token !== undefined);
                    return this.isTranslationKey(token)
                        ? (this.findTranslation(token, language) ?? this.requireTranslation(token))
                        : token;
                })
                .join("")
                .trim();
            if (property === "name") {
                assert(string.length > 0);
                if (language === "english") {
                    this.itemNames.set(id, string);
                }
            }
            if (string.length > 0) {
                itemLanguage[property] = string;
            }
        }
    }

    private tryAddTranslation(id: number, property: keyof CS2ItemTranslation, token: string | undefined) {
        if (!this.isTranslationKey(token)) {
            return undefined;
        }
        return this.addTranslation(id, property, token);
    }

    private addFormattedTranslation(id: number, property: keyof CS2ItemTranslation, key?: string, ...values: string[]) {
        for (const [language, items] of Object.entries(this.itemTranslationByLanguage)) {
            (items[id] ??= {} as CS2ItemTranslation)[property] = (
                this.findTranslation(key, language) ?? this.requireTranslation(key, "english")
            ).replace(FORMATTED_STRING_RE, (_, index) => {
                const key = values[parseInt(index, 10) - 1];
                return this.findTranslation(key, language) ?? this.requireTranslation(key, "english");
            });
        }
    }

    private getPrefab(prefab?: string) {
        return ensure(this.gameItems.prefabs[ensure(prefab)]);
    }

    private tryGetPrefab(prefab?: string) {
        return prefab !== undefined ? this.gameItems.prefabs[prefab] : undefined;
    }

    private getTeams(teams?: Record<string, string>, fallback?: CS2ItemTeamValues) {
        if (teams === undefined) {
            return ensure(fallback);
        }
        const keys = Object.keys(teams);
        const ct = keys.includes("counter-terrorists");
        const t = keys.includes("terrorists");
        switch (true) {
            case ct && t:
                return CS2ItemTeam.Both;
            case ct:
                return CS2ItemTeam.CT;
            case t:
                return CS2ItemTeam.T;
            default:
                return fail();
        }
    }

    private getTeamsString(teams?: Record<string, string>, fallback?: string) {
        // We changed the way we determine the team of an item, we use this
        // logic only for getting the item id.
        return teams === undefined
            ? ensure(fallback)
            : Object.keys(teams)
                  .map((team) => {
                      switch (team) {
                          case "counter-terrorists":
                              return 3;
                          case "terrorists":
                              return 2;
                          default:
                              return fail();
                      }
                  })
                  .join("_");
    }

    private addItem(item: CS2ExtendedItem) {
        if (item.base) {
            this.baseItems.push(item);
        }
        this.items.set(item.id, item);
    }

    private getBaseWeaponCategory(name: string, category: string) {
        if (HEAVY_WEAPONS.includes(name)) {
            return "heavy";
        }
        return category;
    }

    private async getDestFilename(src: string, dest: string) {
        return dest.includes("{sha256}") ? dest.replace("{sha256}", await getFileSha256(src)) : dest;
    }

    private async copyAndOptimizeImage(src: string, dest: string) {
        const filename = await this.getDestFilename(src, dest);
        await sharp(src).webp({ quality: OUTPUT_IMAGE_QUALITY }).toFile(join(OUTPUT_DIR, filename));
        return filename;
    }

    private async copyAndOptimizeTextureImage(src: string, dest: string) {
        const filename = await this.getDestFilename(src, dest);
        await sharp(src).removeAlpha().resize(1024, 1024).webp().toFile(join(OUTPUT_DIR, filename));
        return filename;
    }

    private getImagePath(path: string) {
        return join(GAME_IMAGES_DIR, `${path}_png.png`.toLowerCase());
    }

    private getPaintImagePath(className: string | undefined, paintClassName: string | undefined, suffix = "light") {
        return join(
            GAME_IMAGES_DIR,
            `econ/default_generated/${className}_${paintClassName}_${suffix}_png.png`.toLowerCase()
        );
    }

    private requireStaticAsset(path: string) {
        return ensure(this.staticAssets[path], `Unable to find '${path}' static asset.`);
    }

    private async isImageValid(path: string) {
        return await exists(this.getImagePath(path));
    }

    private async isPaintImageValid(className?: string, paintClassName?: string) {
        return await exists(this.getPaintImagePath(className, paintClassName));
    }

    private async getBaseImage(id: number, className: string) {
        return await this.getImage(`econ/weapons/base_weapons/${className}`);
    }

    private async getImage(path: string) {
        return await this.copyAndOptimizeImage(this.getImagePath(path), `/images/{sha256}.webp`);
    }

    private async getPaintImage(id: number, className: string | undefined, paintClassName: string | undefined) {
        const paths = PAINT_IMAGE_SUFFIXES.map((suffix) => [
            this.getPaintImagePath(className, paintClassName, suffix),
            suffix
        ]);
        const sha256 = await getFilesSha256(paths.map((path) => path[0]));
        const base = `/images/${sha256}`;
        for (const [src, suffix] of paths) {
            await this.copyAndOptimizeImage(src, `${base}_${suffix}.webp`);
        }
        return await this.copyAndOptimizeImage(paths[0][0], `${base}.webp`);
    }

    private async getDefaultGraffitiImage(id: number, sticker_material: string, hexColor: string) {
        const src = this.getImagePath(`econ/stickers/${sticker_material}`);

        const input = sharp(src).ensureAlpha();
        const { width, height } = await input.metadata();
        assert(width && height);
        const color = {
            r: parseInt(hexColor.slice(1, 3), 16),
            g: parseInt(hexColor.slice(3, 5), 16),
            b: parseInt(hexColor.slice(5, 7), 16)
        };
        const grayscaleBuffer = await input.clone().removeAlpha().greyscale().raw().toBuffer();
        const pixelCount = grayscaleBuffer.length;
        const coloredPixels = Buffer.alloc(pixelCount * 3);
        for (let i = 0; i < pixelCount; i++) {
            const gray = grayscaleBuffer[i] / 255;
            coloredPixels[i * 3 + 0] = Math.round(gray * color.r);
            coloredPixels[i * 3 + 1] = Math.round(gray * color.g);
            coloredPixels[i * 3 + 2] = Math.round(gray * color.b);
        }
        const coloredImage = sharp(coloredPixels, {
            raw: {
                width,
                height,
                channels: 3
            }
        }).png();
        const alphaBuffer = await input.clone().ensureAlpha().extractChannel("alpha").toBuffer();
        const coloredWithAlpha = await coloredImage.joinChannel(alphaBuffer).png().toBuffer();
        const dest = `/images/${await getBufferSha256(coloredWithAlpha)}.webp`;
        await sharp(coloredWithAlpha).webp().toFile(join(OUTPUT_DIR, dest));
        return dest;
    }

    private async getSpecialsImage(id: number, path?: string) {
        if (path === undefined) {
            return this.requireStaticAsset("/images/default_rare_item.png");
        }
        const src = this.getImagePath(path);
        if (await exists(src)) {
            return await this.copyAndOptimizeImage(src, `/images/{sha256}_rare.webp`);
        }
        return this.requireStaticAsset("/images/default_rare_item.png");
    }

    private getPaintAltName(className: string) {
        switch (true) {
            case className.includes("_phase"):
                return `Phase ${className.match(SKIN_PHASE_RE)?.[1]}`;
            case className.includes("sapphire_marbleized"):
                return "Sapphire";
            case className.includes("ruby_marbleized"):
                return "Ruby";
            case className.includes("blackpearl_marbleized"):
                return "Black Pearl";
            case className.includes("emerald_marbleized"):
                return "Emerald";
            default:
                return undefined;
        }
    }

    private getStickerCategory({
        sticker_material,
        tournament_event_id
    }: {
        sticker_material: string;
        tournament_event_id?: string;
    }) {
        let category: string | undefined;
        let categoryToken: string | undefined;
        const [folder, subfolder] = sticker_material.split("/");
        if (folder === "alyx") {
            categoryToken = "#CSGO_crate_sticker_pack_hlalyx_capsule";
            category = this.findTranslation(categoryToken);
        }
        if (subfolder == "elemental_craft") {
            categoryToken = "#CSGO_crate_sticker_pack_stkr_craft_01_capsule";
            category = this.findTranslation(categoryToken);
        }
        if (UNCATEGORIZED_STICKERS.includes(folder)) {
            categoryToken = undefined;
            category = "Valve";
        }
        if (category === undefined) {
            categoryToken = `#CSGO_crate_sticker_pack_${folder}`;
            category = this.findTranslation(categoryToken);
        }
        if (category === undefined) {
            categoryToken = `#CSGO_crate_sticker_pack_${folder}_capsule`;
            category = this.findTranslation(categoryToken);
        }
        if (tournament_event_id !== undefined) {
            categoryToken = `#CSGO_Tournament_Event_NameShort_${tournament_event_id}`;
            category = this.findTranslation(categoryToken);
            assert(category, `unable to find the short name for tournament '${tournament_event_id}'.`);
        }
        if (category === undefined) {
            categoryToken = `#CSGO_crate_sticker_pack_${subfolder}_capsule`;
            category = this.findTranslation(categoryToken);
        }
        if (category === undefined) {
            categoryToken = `#CSGO_sticker_crate_key_${folder}`;
            category = this.findTranslation(categoryToken);
        }
        if (category === undefined) {
            categoryToken = undefined;
            category = "Valve";
        }
        return [ensure(category), categoryToken] as const;
    }

    private getAgentVoPrefix(model: string, prefix?: string) {
        switch (true) {
            case prefix === "ctm_gsg9":
                return "gsg9";
            case prefix !== undefined:
                return prefix;
            case model.includes("tm_leet"):
                return "leet";
            case model.includes("ctm_st6"):
                return "seal";
            case model.includes("ctm_swat"):
                return "swat";
            case model.includes("tm_balkan"):
                return "balkan";
            case model.includes("tm_professional"):
                return "professional";
            case model.includes("tm_phoenix"):
                return "phoenix";
            case model.includes("ctm_fbi"):
                return "fbihrt";
            case model.includes("ctm_sas"):
                return "sas";
            default:
                return fail();
        }
    }

    private getAgentVoFemale(prefix: string) {
        if (prefix.includes("_fem")) {
            return true;
        }
        if (prefix === "fbihrt_epic") {
            return true;
        }
        return undefined;
    }

    private async getAgentVoFallback(prefix: string) {
        return (await readFile(join(AGENTS_SOUNDEVENTS_PATH, `game_sounds_${prefix}.vsndevts`), "utf-8")).includes(
            "radiobot"
        )
            ? true
            : undefined;
    }

    private async getCollectionImage(name: string) {
        const src = join(GAME_IMAGES_DIR, `econ/set_icons/${name}_png.png`);
        if (!(await exists(src))) {
            log(`Image not found for collection ${name}`);
            return undefined;
        }
        return await this.copyAndOptimizeImage(src, `/images/{sha256}.webp`);
    }

    private getCollection(itemId: number, collection?: string) {
        let collectionImage: string | undefined = undefined;
        if (collection !== undefined) {
            const itemSet = this.gameItems.item_sets[collection];
            assert(itemSet, `Collection '${collection}' not found.`);
            assert(itemSet.name, `Collection name not found for '${collection}'.`);
            this.tryAddTranslation(itemId, "collectionName", itemSet.name);
            this.tryAddTranslation(itemId, "collectionDesc", itemSet.set_description);
            collectionImage = this.itemSetImage[collection];
        }
        return { collection, collectionImage };
    }

    private getPaintCollection(itemId: number, itemKey: string) {
        return this.getCollection(itemId, this.itemSetItemKey[itemKey]);
    }

    private addContainerItem(itemKey: string, id: number) {
        if (!this.containerItems.has(itemKey)) {
            this.containerItems.set(itemKey, id);
        }
    }

    private getClientLootListItems(clientLootListKey: string, items: string[] = []) {
        if (!this.gameItems.client_loot_lists[clientLootListKey]) {
            return [];
        }
        const itemOrClientLootListKeys = Object.keys(this.gameItems.client_loot_lists[clientLootListKey]);
        for (const itemOrClientLootListKey of itemOrClientLootListKeys) {
            // At this point, `containerItems` should be populated with all
            // economy items that can be retrieved from containers.
            if (this.containerItems.has(itemOrClientLootListKey)) {
                items.push(itemOrClientLootListKey);
            } else {
                // If we did not find, that means that it's probably a reference
                // to another loot list...
                this.getClientLootListItems(itemOrClientLootListKey, items);
            }
        }
        return items;
    }

    private async findStickerMarkup(itemDef?: string, modelPath?: string) {
        assert(itemDef !== undefined && modelPath !== undefined, `Failed to read ${modelPath} for sticker markup.`);
        const output = (
            await this.cs2.decompile({
                vpkFilepath: modelPath.replace(".vmdl", ".vmdl_c"),
                block: "DATA"
            })
        ).split(DECOMPILER_DATA_SEPARATOR)[1];
        const data = CS2KeyValues3.parse<ModelData>(
            CS2KeyValues3.parse<{
                m_modelInfo: {
                    m_keyValueText: string;
                };
            }>(output).m_modelInfo.m_keyValueText
        );
        const stickerMarkup = data.StickerMarkup?.map(
            ({ Index: slot, LegacyModel: legacy, Offset: offsets, Rotation: rotation, Scale: scale }) => ({
                slot,
                legacy,
                offsets,
                rotation,
                scale
            })
        );
        this.stickerMarkup[itemDef] = stickerMarkup;
        return stickerMarkup;
    }

    private async getTexturePathFromCompositeMaterial(compositeMaterialPath?: string) {
        try {
            if (compositeMaterialPath === undefined) {
                return undefined;
            }
            return ensure(
                CS2KeyValues3.parse<CompositeMaterialData>(
                    (
                        await this.cs2.decompile({
                            vpkFilepath: compositeMaterialPath,
                            block: "DATA"
                        })
                    ).split(DECOMPILER_DATA_SEPARATOR)[1]
                )
                    .m_Points[0].m_vecCompositeMaterialAssemblyProcedures[0].m_vecCompositeInputContainers.find(
                        ({ m_strAlias }) => m_strAlias === "exposed_params"
                    )
                    ?.m_vecLooseVariables.find(({ m_strName }) => m_strName === "g_tPattern")
                    ?.m_strTextureRuntimeResourcePath.split(":")[1]
            );
        } catch {
            log(`Failed to retrieve texture path from ${compositeMaterialPath}.`);
            return undefined;
        }
    }

    private async getTexturePathFromMaterial(materialPath: string) {
        return ensure(
            CS2KeyValues3.parse<MaterialData>(
                (
                    await this.cs2.decompile({
                        vpkFilepath: materialPath,
                        block: "DATA"
                    })
                ).split(DECOMPILER_DATA_SEPARATOR)[1]
            ).m_textureParams.find(({ m_name }) => m_name === "g_tPattern")
        ).m_pValue.split(":")[1];
    }

    private async getPaintTexture(id: number, materialName: string, compositeMaterialPath?: string) {
        try {
            if (INPUT_TEXTURES !== "true") {
                return undefined;
            }
            const materialFilename = `${materialName}.vmat_c`;
            const materialPath = `materials/models/weapons/customization/paints/vmats/${materialFilename}`;
            const texturePath =
                (await this.getTexturePathFromCompositeMaterial(compositeMaterialPath)) ??
                (await this.getTexturePathFromMaterial(materialPath));
            if (
                !texturePath.startsWith("items/assets/paintkits") &&
                !texturePath.startsWith("materials/models/weapons/customization/paints/custom") &&
                !texturePath.startsWith("materials/models/weapons/customization/paints/gunsmith")
            ) {
                logOnce(`Ignoring texture path '${texturePath}'`);
                return undefined;
            }
            await this.cs2.decompile({
                vpkFilepath: texturePath,
                decompile: true,
                output: DECOMPILED_DIR
            });
            return await this.copyAndOptimizeTextureImage(
                join(DECOMPILED_DIR, texturePath.replace(".vtex", ".png")),
                `/textures/{sha256}.webp`
            );
        } catch (error) {
            logOnce(`Failed to retrieve paint texture for '${materialName}'`);
            return undefined;
        }
    }

    private getContainerType(name?: string, type?: CS2ItemTypeValues) {
        switch (true) {
            case name?.includes("Souvenir"):
                return CS2ContainerType.SouvenirCase;
            case type === CS2ItemType.Weapon:
                return CS2ContainerType.WeaponCase;
            case type === CS2ItemType.Sticker:
                return CS2ContainerType.StickerCapsule;
            case type === CS2ItemType.Graffiti:
                return CS2ContainerType.GraffitiBox;
            default:
                return undefined;
        }
    }

    private createStub(name: string, descToken: string) {
        const id = this.itemIdentityHelper.get(`stub_${name}`);
        this.addTranslation(id, "name", "#Rarity_Default");
        this.addTranslation(id, "desc", descToken);
        this.addItem({
            id,
            type: CS2ItemType.Stub
        });
        return id;
    }
}

if (shouldRun(import.meta.url)) {
    await new ItemGenerator().run();
}
