/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Ian Lucas. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/

import {
    CS2_CONTAINER_ITEMS,
    CS2_CONTRACT_TOOL_DEF,
    CS2_DISPLAY_ITEMS,
    CS2_EQUIPMENT_ITEMS,
    CS2_GRAPHIC_ART_ITEMS,
    CS2_KEYCHAINABLE_ITEMS,
    CS2_MACHINEGUN_MODELS,
    CS2_MAX_FACTORY_NEW_WEAR,
    CS2_MAX_FIELD_TESTED_WEAR,
    CS2_MAX_KEYCHAIN_SEED,
    CS2_MAX_MINIMAL_WEAR_WEAR,
    CS2_MAX_SEED,
    CS2_MAX_STATTRAK,
    CS2_MAX_STICKERS,
    CS2_MAX_WEAR,
    CS2_MAX_WELL_WORN_WEAR,
    CS2_MIDTIER_CATEGORIES,
    CS2_MIN_KEYCHAIN_SEED,
    CS2_MIN_SEED,
    CS2_MIN_STATTRAK,
    CS2_MIN_WEAR,
    CS2_MISC_CATEGORIES,
    CS2_NAMETAGGABLE_ITEMS,
    CS2_NAMETAG_RE,
    CS2_NAMETAG_TOOL_DEF,
    CS2_PAINTABLE_ITEMS,
    CS2_PATCHABLE_ITEMS,
    CS2_RIFLE_CATEGORIES,
    CS2_SEEDABLE_ITEMS,
    CS2_SNIPER_RIFLE_MODELS,
    CS2_STATTRAKABLE_ITEMS,
    CS2_STATTRAK_SWAP_TOOL_DEF,
    CS2_STICKERABLE_ITEMS,
    CS2_STORAGE_UNIT_TOOL_DEF,
    CS2_TEAMS_BOTH,
    CS2_TEAMS_CT,
    CS2_TEAMS_T,
    CS2_WEAR_FACTOR
} from "./economy-constants.ts";
import {
    CS2RarityColorName,
    CS2RarityColorOrder,
    type CS2RarityColorValues,
    CS2RaritySoundName,
    CS2_BASE_ODD,
    CS2_RARITY_COLOR_DEFAULT,
    CS2_RARITY_ORDER,
    CS2_STATTRAK_ODD,
    randomFloat,
    randomInt
} from "./economy-container.ts";
import {
    CS2ContainerType,
    type CS2ContainerTypeValues,
    type CS2Item,
    CS2ItemTeam,
    type CS2ItemTeamValues,
    type CS2ItemTranslation,
    type CS2ItemTranslationMap,
    CS2ItemType,
    type CS2ItemTypeValues,
    CS2ItemWear,
    type CS2ItemWearValues,
    type CS2UnlockedItem
} from "./economy-types.ts";
import { type CS2TeamValues } from "./teams.ts";
import { type Interface, assert, compare, ensure, safe } from "./utils.ts";

type CS2EconomyItemPredicate = Partial<CS2EconomyItem> & { team?: CS2TeamValues };

function filterItems(predicate: CS2EconomyItemPredicate): (item: CS2EconomyItem) => boolean {
    return function filter(item: CS2EconomyItem) {
        return (
            compare(predicate.type, item.type) &&
            compare(predicate.free, item.free) &&
            compare(predicate.model, item.model) &&
            compare(predicate.base, item.base) &&
            compare(predicate.category, item.category) &&
            (predicate.team === undefined || item.teams === undefined || item.teams.includes(predicate.team))
        );
    };
}

export class CS2EconomyInstance {
    baseUrl = "https://cdn.cstrike.app";
    categories: Set<string> = new Set<string>();
    items: Map<number, CS2EconomyItem> = new Map<number, CS2EconomyItem>();
    itemsAsArray: CS2EconomyItem[] = [];
    stickers: Set<CS2EconomyItem> = new Set<CS2EconomyItem>();

    use({
        assetsBaseUrl,
        items,
        language
    }: {
        assetsBaseUrl?: string;
        items: CS2Item[];
        language: CS2ItemTranslationMap;
    }): void {
        this.baseUrl = assetsBaseUrl ?? this.baseUrl;
        this.categories.clear();
        this.items.clear();
        this.stickers.clear();
        this.itemsAsArray = [];
        for (const item of items) {
            const economyItem = new CS2EconomyItem(this, item, ensure(language[item.id]));
            this.items.set(item.id, economyItem);
            if (economyItem.isSticker()) {
                this.stickers.add(economyItem);
                this.categories.add(ensure(economyItem.category));
            }
            this.itemsAsArray.push(economyItem);
        }
    }

    getById(id: number): CS2EconomyItem {
        return ensure(this.items.get(id));
    }

    get(idOrItem: number | CS2EconomyItem): CS2EconomyItem {
        return typeof idOrItem === "number" ? this.getById(idOrItem) : idOrItem;
    }

    findItem(predicate: CS2EconomyItemPredicate): CS2EconomyItem {
        return ensure(this.itemsAsArray.find(filterItems(predicate)));
    }

    filterItems(predicate: CS2EconomyItemPredicate): CS2EconomyItem[] {
        const items = this.itemsAsArray.filter(filterItems(predicate));
        assert(items.length > 0);
        return items;
    }

    validateWear(wear?: number, item?: CS2EconomyItem): boolean {
        if (wear === undefined) {
            return true;
        }
        assert(Number.isFinite(wear));
        assert(String(wear).length <= String(CS2_WEAR_FACTOR).length);
        assert(wear >= CS2_MIN_WEAR && wear <= CS2_MAX_WEAR);
        if (item !== undefined) {
            assert(item.hasWear());
            assert(item.wearMin === undefined || wear >= item.wearMin);
            assert(item.wearMax === undefined || wear <= item.wearMax);
        }
        return true;
    }

    safeValidateWear(wear?: number, item?: CS2EconomyItem): boolean {
        return safe(() => this.validateWear(wear, item));
    }

    validateSeed(seed?: number, item?: CS2EconomyItem): boolean {
        if (seed === undefined) {
            return true;
        }
        assert(Number.isFinite(seed));
        assert(Number.isInteger(seed));
        assert(item === undefined || item.hasSeed());
        assert(seed >= (item?.getMinimumSeed() ?? CS2_MIN_SEED) && seed <= (item?.getMaximumSeed() ?? CS2_MAX_SEED));
        return true;
    }

    safeValidateSeed(seed?: number, item?: CS2EconomyItem): boolean {
        return safe(() => this.validateSeed(seed, item));
    }

    trimNametag(nameTag?: string): string | undefined {
        const trimmed = nameTag?.trim();
        return trimmed === "" ? undefined : trimmed;
    }

    validateNametag(nameTag?: string, item?: CS2EconomyItem): boolean {
        if (nameTag !== undefined) {
            assert(item === undefined || item.hasNametag());
            assert(nameTag[0] !== " " && CS2_NAMETAG_RE.test(nameTag));
        }
        return true;
    }

    safeValidateNametag(nameTag?: string, item?: CS2EconomyItem): boolean {
        return safe(() => this.validateNametag(nameTag, item));
    }

    requireNametag(nameTag?: string, item?: CS2EconomyItem): boolean {
        assert(nameTag === undefined || nameTag.trim().length > 0);
        return this.validateNametag(nameTag, item);
    }

    safeRequireNametag(nameTag?: string, item?: CS2EconomyItem): boolean {
        return safe(() => this.requireNametag(nameTag, item));
    }

    validateStatTrak(statTrak?: number, item?: CS2EconomyItem): boolean {
        if (statTrak === undefined) {
            return true;
        }
        assert(item === undefined || item.hasStatTrak());
        assert(Number.isInteger(statTrak));
        assert(statTrak >= CS2_MIN_STATTRAK && statTrak <= CS2_MAX_STATTRAK);
        return true;
    }

    safeValidateStatTrak(statTrak?: number, item?: CS2EconomyItem): boolean {
        return safe(() => this.validateStatTrak(statTrak, item));
    }

    getWearFromValue(value: number): CS2ItemWearValues {
        switch (true) {
            case value <= CS2_MAX_FACTORY_NEW_WEAR:
                return CS2ItemWear.FactoryNew;
            case value <= CS2_MAX_MINIMAL_WEAR_WEAR:
                return CS2ItemWear.MinimalWear;
            case value <= CS2_MAX_FIELD_TESTED_WEAR:
                return CS2ItemWear.FieldTested;
            case value <= CS2_MAX_WELL_WORN_WEAR:
                return CS2ItemWear.WellWorn;
            default:
                return CS2ItemWear.BattleScarred;
        }
    }

    getStickerCategories(): string[] {
        return Array.from(this.categories).sort();
    }

    getStickers(): CS2EconomyItem[] {
        return Array.from(this.stickers);
    }

    validateContainerAndKey(containerItem: number | CS2EconomyItem, keyItem?: number | CS2EconomyItem): boolean {
        containerItem = this.get(containerItem);
        containerItem.expectContainer();
        keyItem = keyItem !== undefined ? this.get(keyItem) : undefined;
        if (keyItem !== undefined) {
            keyItem.expectKey();
            assert(containerItem.keys !== undefined);
            assert(containerItem.keys.includes(keyItem.id));
        } else {
            assert(containerItem.keys === undefined);
        }
        return true;
    }

    safeValidateContainerAndKey(containerItem: number | CS2EconomyItem, keyItem?: number | CS2EconomyItem): boolean {
        return safe(() => this.validateContainerAndKey(containerItem, keyItem));
    }

    validateUnlockedItem(
        item: number | CS2EconomyItem,
        { id }: ReturnType<InstanceType<typeof CS2EconomyItem>["unlockContainer"]>
    ): void {
        item = this.get(item).expectContainer();
        assert(item.rawContents?.includes(id) || item.rawSpecials?.includes(id));
    }
}

export class CS2EconomyItem implements Interface<
    Omit<CS2Item, "contents" | "specials" | "teams"> &
        CS2ItemTranslation & {
            contents: CS2EconomyItem[] | undefined;
            teams: CS2TeamValues[] | undefined;
        }
> {
    altName: string | undefined;
    base: boolean | undefined;
    baseId: number | undefined;
    category: string | undefined;
    collection: string | undefined;
    collectionDesc: string | undefined;
    collectionImage: string | undefined;
    collectionName: string | undefined;
    containerType: CS2ContainerTypeValues | undefined;
    def: number | undefined;
    desc: string | undefined;
    free: boolean | undefined;
    id: number = null!;
    image: string | undefined;
    index: number | undefined;
    keys: number[] | undefined;
    legacy: boolean | undefined;
    legacyStickerSlots: number | undefined;
    model: string | undefined;
    modelBinary: string | undefined;
    name: string = null!;
    rarity: CS2RarityColorValues = null!;
    specialsImage: string | undefined;
    statTrakless: boolean | undefined;
    statTrakOnly: boolean | undefined;
    stickerId: number | undefined;
    stickerIndex: number | undefined;
    stickerSlots: number | undefined;
    textureImage: string | undefined;
    tint: number | undefined;
    tournamentDesc: string | undefined;
    type: CS2ItemTypeValues = null!;
    voFallback: boolean | undefined;
    voFemale: boolean | undefined;
    voPrefix: string | undefined;
    wearMax: number | undefined;
    wearMin: number | undefined;

    private _contents: number[] | undefined;
    private _specials: number[] | undefined;
    private _teams: CS2ItemTeamValues | undefined;

    constructor(
        public economy: CS2EconomyInstance,
        public item: CS2Item,
        public language: CS2ItemTranslation
    ) {
        Object.assign(this, item);
        Object.assign(this, language);
        assert(typeof this.id === "number");
        assert(this.name);
        assert(this.type);
        assert(item.type === CS2ItemType.Stub || typeof this.rarity === "string");
    }

    set contents(value: number[] | undefined) {
        this._contents = value;
    }

    get contents(): CS2EconomyItem[] {
        this.expectContainer();
        return ensure(this._contents).map((id) => this.economy.get(id));
    }

    get parent(): CS2EconomyItem | undefined {
        return this.baseId !== undefined
            ? this.economy.items.has(this.baseId)
                ? this.economy.get(this.baseId)
                : undefined
            : undefined;
    }

    get rawContents(): number[] | undefined {
        return this._contents;
    }

    get rawSpecials(): number[] | undefined {
        return this._specials;
    }

    set specials(value: number[] | undefined) {
        this._specials = value;
    }

    get specials(): CS2EconomyItem[] | undefined {
        this.expectContainer();
        return this._specials?.map((id) => this.economy.get(id));
    }

    set teams(value: CS2ItemTeamValues) {
        this._teams = value;
    }

    get teams(): CS2TeamValues[] | undefined {
        switch (this._teams) {
            case CS2ItemTeam.Both:
                return CS2_TEAMS_BOTH;
            case CS2ItemTeam.T:
                return CS2_TEAMS_T;
            case CS2ItemTeam.CT:
                return CS2_TEAMS_CT;
            default:
                return undefined;
        }
    }

    get wrappedSticker(): CS2EconomyItem | undefined {
        return this.stickerId !== undefined ? this.economy.get(this.stickerId) : undefined;
    }

    isC4(): boolean {
        return this.category === "c4";
    }

    isPistol(): boolean {
        return this.category === "secondary";
    }

    isSMG(): boolean {
        return this.category === "smg";
    }

    isRifle(): boolean {
        return this.category === "rifle";
    }

    isSniperRifle(): boolean {
        return this.model !== undefined && CS2_SNIPER_RIFLE_MODELS.includes(this.model);
    }

    isMachinegun(): boolean {
        return this.model !== undefined && CS2_MACHINEGUN_MODELS.includes(this.model);
    }

    isHeavy(): boolean {
        return this.category === "heavy";
    }

    isEquipment(): boolean {
        return this.category === "equipment";
    }

    isInMidTiers(): boolean {
        return this.category !== undefined && CS2_MIDTIER_CATEGORIES.includes(this.category);
    }

    isInRifles(): boolean {
        return this.category !== undefined && CS2_RIFLE_CATEGORIES.includes(this.category);
    }

    isInMisc(): boolean {
        return this.category !== undefined && CS2_MISC_CATEGORIES.includes(this.category);
    }

    isAgent(): boolean {
        return this.type === CS2ItemType.Agent;
    }

    isCollectible(): boolean {
        return this.type === CS2ItemType.Collectible;
    }

    isContainer(): boolean {
        return this.type === CS2ItemType.Container;
    }

    isKey(): boolean {
        return this.type === CS2ItemType.Key;
    }

    isGloves(): boolean {
        return this.type === CS2ItemType.Gloves;
    }

    isGraffiti(): boolean {
        return this.type === CS2ItemType.Graffiti;
    }

    isMelee(): boolean {
        return this.type === CS2ItemType.Melee;
    }

    isMusicKit(): boolean {
        return this.type === CS2ItemType.MusicKit;
    }

    isPatch(): boolean {
        return this.type === CS2ItemType.Patch;
    }

    isSticker(): boolean {
        return this.type === CS2ItemType.Sticker;
    }

    isKeychain(): boolean {
        return this.type === CS2ItemType.Keychain;
    }

    isStub(): boolean {
        return this.type === CS2ItemType.Stub;
    }

    isTool(): boolean {
        return this.type === CS2ItemType.Tool;
    }

    isWeapon(): boolean {
        return this.type === CS2ItemType.Weapon;
    }

    isStorageUnit(): boolean {
        return this.isTool() && this.def === CS2_STORAGE_UNIT_TOOL_DEF;
    }

    isNameTag(): boolean {
        return this.isTool() && this.def === CS2_NAMETAG_TOOL_DEF;
    }

    isStatTrakSwapTool(): boolean {
        return this.isTool() && this.def === CS2_STATTRAK_SWAP_TOOL_DEF;
    }

    isContract(): boolean {
        return this.isTool() && this.def === CS2_CONTRACT_TOOL_DEF;
    }

    expectAgent(): this {
        assert(this.isAgent());
        return this;
    }

    expectPatch(): this {
        assert(this.isPatch());
        return this;
    }

    expectSticker(): this {
        assert(this.isSticker());
        return this;
    }

    expectKeychain(): this {
        assert(this.isKeychain());
        return this;
    }

    expectStorageUnit(): this {
        assert(this.isStorageUnit());
        return this;
    }

    expectNameTag(): this {
        assert(this.isNameTag());
        return this;
    }

    expectStatTrakSwapTool(): this {
        assert(this.isStatTrakSwapTool());
        return this;
    }

    expectContainer(): this {
        assert(this.isContainer());
        return this;
    }

    expectKey(): this {
        assert(this.isKey());
        return this;
    }

    hasWear(): boolean {
        return CS2_PAINTABLE_ITEMS.includes(this.type) && !this.free && this.index !== 0;
    }

    hasSeed(): boolean {
        return CS2_SEEDABLE_ITEMS.includes(this.type) && !this.free && this.index !== 0;
    }

    hasStickers(): boolean {
        return CS2_STICKERABLE_ITEMS.includes(this.type) && !this.isC4();
    }

    hasKeychains(): boolean {
        return CS2_KEYCHAINABLE_ITEMS.includes(this.type) && !this.isC4();
    }

    hasPatches(): boolean {
        return CS2_PATCHABLE_ITEMS.includes(this.type);
    }

    hasNametag(): boolean {
        return CS2_NAMETAGGABLE_ITEMS.includes(this.type) || this.isStorageUnit();
    }

    hasStatTrak(): boolean {
        return CS2_STATTRAKABLE_ITEMS.includes(this.type) && !this.free;
    }

    isWeaponCase(): boolean {
        return this.containerType === CS2ContainerType.WeaponCase;
    }

    isStickerCapsule(): boolean {
        return this.containerType === CS2ContainerType.StickerCapsule;
    }

    isGraffitiBox(): boolean {
        return this.containerType === CS2ContainerType.GraffitiBox;
    }

    isSouvenirCase(): boolean {
        return this.containerType === CS2ContainerType.SouvenirCase;
    }

    isInEquipments(): boolean {
        return CS2_EQUIPMENT_ITEMS.includes(this.type);
    }

    isInGraphicArts(): boolean {
        return CS2_GRAPHIC_ART_ITEMS.includes(this.type);
    }

    isInContainers(): boolean {
        return CS2_CONTAINER_ITEMS.includes(this.type);
    }

    isInDisplay(): boolean {
        return CS2_DISPLAY_ITEMS.includes(this.type);
    }

    isPaintable(): boolean {
        return CS2_PAINTABLE_ITEMS.includes(this.type);
    }

    getImage(wear?: number): string {
        assert(this.image);
        if (this.hasWear() && wear !== undefined) {
            const url = `${this.economy.baseUrl}${this.image}`;
            switch (true) {
                case wear < 1 / 3:
                    return url.replace(".webp", "_light.webp");
                case wear < 2 / 3:
                    return url.replace(".webp", "_medium.webp");
                default:
                    return url.replace(".webp", "_heavy.webp");
            }
        }
        return `${this.economy.baseUrl}${this.image}`;
    }

    getCollectionImage(): string {
        return `${this.economy.baseUrl}${this.collectionImage}`;
    }

    getSpecialsImage(): string {
        this.expectContainer();
        assert(this.rawSpecials);
        assert(this.specialsImage);
        return `${this.economy.baseUrl}${this.specialsImage}`;
    }

    getTextureImage(): string {
        return `${this.economy.baseUrl}${ensure(this.textureImage)}`;
    }

    getModelBinary(): string {
        const { modelBinary } = this.parent ?? this;
        return `${this.economy.baseUrl}${ensure(modelBinary)}`;
    }

    getMinimumWear(): number {
        return this.wearMin ?? CS2_MIN_WEAR;
    }

    getMaximumWear(): number {
        return this.wearMax ?? CS2_MAX_WEAR;
    }

    getMinimumSeed(): number {
        return this.isKeychain() ? CS2_MIN_KEYCHAIN_SEED : CS2_MIN_SEED;
    }

    getMaximumSeed(): number {
        return this.isKeychain() ? CS2_MAX_KEYCHAIN_SEED : CS2_MAX_SEED;
    }

    getStickerSlotCount(): number {
        return this.parent?.[this.legacy ? "legacyStickerSlots" : "stickerSlots"] ?? CS2_MAX_STICKERS;
    }

    groupContents(): Record<string, CS2EconomyItem[]> {
        const items: Record<string, CS2EconomyItem[]> = {};
        const specials = this.specials;
        for (const item of this.contents) {
            const rarity = CS2RarityColorName[item.rarity];
            if (!items[rarity]) {
                items[rarity] = [];
            }
            items[rarity].push(item);
        }
        if (specials !== undefined) {
            for (const item of specials) {
                const rarity = "special";
                if (!items[rarity]) {
                    items[rarity] = [];
                }
                items[rarity].push(item);
            }
        }
        return items;
    }

    listContents(hideSpecials = false): CS2EconomyItem[] {
        const specials = this.specials;
        const items = [...this.contents, ...(!hideSpecials && specials !== undefined ? specials : [])];
        return items.sort((a, b) => {
            return (
                (CS2RarityColorOrder[a.rarity] ?? CS2_RARITY_COLOR_DEFAULT) -
                (CS2RarityColorOrder[b.rarity] ?? CS2_RARITY_COLOR_DEFAULT)
            );
        });
    }

    unlockContainer(options?: {
        computeOdds?: (rarities: (typeof CS2_RARITY_ORDER)[number][]) => number[] | undefined;
    }): CS2UnlockedItem {
        // @see https://www.csgo.com.cn/news/gamebroad/20170911/206155.shtml
        const contents = this.groupContents();
        const keys = Object.keys(contents);
        const rarities = CS2_RARITY_ORDER.filter((rarity) => keys.includes(rarity));
        const odds = options?.computeOdds?.(rarities) ?? rarities.map((_, index) => CS2_BASE_ODD / Math.pow(5, index));
        const total = odds.reduce((acc, cur) => acc + cur, 0);
        const entries = rarities.map((rarity, index) => [rarity, ensure(odds[index]) / total] as const);
        const roll = Math.random();
        let [rollRarity] = ensure(entries[0]);
        let acc = 0;
        for (const [rarity, odd] of entries) {
            acc += odd;
            if (roll <= acc) {
                rollRarity = rarity;
                break;
            }
        }
        const stack = ensure(contents[rollRarity]);
        const unlocked = ensure(stack[Math.floor(Math.random() * stack.length)]);
        const hasStatTrak = this.statTrakless !== true;
        const alwaysStatTrak = this.statTrakOnly === true;
        return {
            attributes: {
                containerId: this.id,
                seed: unlocked.hasSeed() ? randomInt(CS2_MIN_SEED, CS2_MAX_SEED) : undefined,
                statTrak: hasStatTrak
                    ? unlocked.hasStatTrak()
                        ? alwaysStatTrak || Math.random() <= CS2_STATTRAK_ODD
                            ? 0
                            : undefined
                        : undefined
                    : undefined,
                wear: unlocked.hasWear()
                    ? Number(
                          randomFloat(unlocked.wearMin ?? CS2_MIN_WEAR, unlocked.wearMax ?? CS2_MAX_WEAR)
                              .toString()
                              .substring(0, CS2_WEAR_FACTOR.toString().length)
                      )
                    : undefined
            },
            id: unlocked.id,
            rarity: CS2RaritySoundName[unlocked.rarity],
            special: rollRarity === "special"
        };
    }
}

export const CS2Economy: CS2EconomyInstance = new CS2EconomyInstance();
