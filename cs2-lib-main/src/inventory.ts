/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Ian Lucas. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/

import {
    CS2_MAX_KEYCHAINS,
    CS2_MAX_KEYCHAIN_SEED,
    CS2_MAX_PATCHES,
    CS2_MAX_STATTRAK,
    CS2_MAX_STICKERS,
    CS2_MAX_STICKER_ROTATION,
    CS2_MAX_STICKER_WEAR,
    CS2_MAX_WEAR,
    CS2_MIN_KEYCHAIN_SEED,
    CS2_MIN_STICKER_ROTATION,
    CS2_MIN_STICKER_WEAR,
    CS2_MIN_WEAR,
    CS2_STICKER_WEAR_FACTOR
} from "./economy-constants.ts";
import { CS2ItemType, type CS2ItemTypeValues, type CS2UnlockedItem } from "./economy-types.ts";
import { CS2Economy, CS2EconomyInstance, CS2EconomyItem } from "./economy.ts";
import { resolveInventoryData } from "./inventory-upgrader.ts";
import { CS2Team, type CS2TeamValues } from "./teams.ts";
import { type Interface, type MapValue, type RecordValue, assert, ensure, float } from "./utils.ts";

export interface CS2BaseInventoryItem {
    containerId?: number;
    equipped?: boolean;
    equippedCT?: boolean;
    equippedT?: boolean;
    id: number;
    keychains?: Record<
        string,
        {
            id: number;
            seed?: number;
            x?: number;
            y?: number;
            z?: number;
        }
    >;
    nameTag?: string;
    patches?: Record<string, number>;
    seed?: number;
    statTrak?: number;
    stickers?: Record<
        string,
        {
            id: number;
            rotation?: number;
            schema?: number;
            wear?: number;
            x?: number;
            y?: number;
        }
    >;
    storage?: Record<number, CS2BaseInventoryItem>;
    updatedAt?: number;
    wear?: number;
}

export interface CS2InventoryData {
    items: Record<number, CS2BaseInventoryItem>;
    version: number;
}

export interface CS2InventoryOptions {
    maxItems: number;
    storageUnitMaxItems: number;
}

export interface CS2InventorySpec extends CS2InventoryOptions {
    economy: CS2EconomyInstance;
    data: CS2InventoryData;
}

export const CS2_INVENTORY_VERSION = 1;
export const CS2_INVENTORY_TIMESTAMP = 1707696138408;
// prettier-ignore
export const CS2_INVENTORY_EQUIPPABLE_ITEMS: CS2ItemTypeValues[] = [CS2ItemType.Agent, CS2ItemType.Collectible, CS2ItemType.Gloves, CS2ItemType.Graffiti, CS2ItemType.Melee, CS2ItemType.MusicKit, CS2ItemType.Weapon];

export function getTimestamp(): number {
    return Math.ceil((Date.now() - CS2_INVENTORY_TIMESTAMP) / 1000);
}

export function getNextUid(map: Map<number, unknown>): number {
    let uid = 0;
    while (true) {
        if (!map.has(uid)) {
            return uid;
        }
        uid++;
    }
}

export class CS2Inventory {
    private economy: CS2EconomyInstance;
    private items: Map<number, CS2InventoryItem>;
    readonly options: Readonly<CS2InventoryOptions>;

    static parse(stringValue: string | undefined | null, economy?: CS2EconomyInstance): CS2InventoryData | undefined {
        return resolveInventoryData(stringValue ?? undefined, economy);
    }

    constructor({ economy, data, maxItems, storageUnitMaxItems }: Partial<CS2InventorySpec> = {}) {
        this.economy = economy ?? CS2Economy;
        this.items = data !== undefined ? this.toInventoryItems(data.items) : new Map();
        this.options = {
            maxItems: maxItems ?? 256,
            storageUnitMaxItems: storageUnitMaxItems ?? 32
        };
    }

    private validateAddable(item: CS2EconomyItem): void {
        if (item.isGloves()) {
            assert(item.free || !item.base);
        }
    }

    private validateStickers(stickers?: CS2BaseInventoryItem["stickers"], item?: CS2EconomyItem): void {
        if (stickers === undefined) {
            return;
        }
        const entries = Object.entries(stickers);
        assert(entries.length <= CS2_MAX_STICKERS);
        assert(item === undefined || item.hasStickers());
        // @todo: validate x and y offsets, for now apps must implement it on their own.
        for (const [key, { id: stickerId, wear, rotation, x, y, schema }] of entries) {
            const slot = parseInt(key, 10);
            assert(slot >= 0 && slot < CS2_MAX_STICKERS);
            this.economy.getById(stickerId).expectSticker();
            if (wear !== undefined) {
                assert(Number.isFinite(wear));
                assert(String(wear).length <= String(CS2_STICKER_WEAR_FACTOR).length);
                assert(wear >= CS2_MIN_STICKER_WEAR && wear <= CS2_MAX_STICKER_WEAR);
            }
            if (rotation !== undefined) {
                assert(Number.isInteger(rotation));
                assert(rotation >= CS2_MIN_STICKER_ROTATION && rotation <= CS2_MAX_STICKER_ROTATION);
            }
            if (x !== undefined) {
                assert(Number.isFinite(x));
            }
            if (y !== undefined) {
                assert(Number.isFinite(y));
            }
            if (schema !== undefined) {
                assert(Number.isInteger(schema));
                assert(schema >= 0 && schema < (item?.getStickerSlotCount() ?? CS2_MAX_STICKERS));
            }
        }
    }

    private validateKeychains(keychains?: CS2BaseInventoryItem["keychains"], item?: CS2EconomyItem): void {
        if (keychains === undefined) {
            return;
        }
        const entries = Object.entries(keychains);
        assert(entries.length <= CS2_MAX_KEYCHAINS);
        assert(item === undefined || item.hasKeychains());
        // @todo: validate x and y offsets, for now apps must implement it on their own.
        for (const [key, { id: keychainId, seed, x, y, z }] of entries) {
            const slot = parseInt(key, 10);
            assert(slot >= 0 && slot <= CS2_MAX_KEYCHAINS - 1);
            this.economy.getById(keychainId).expectKeychain();
            if (seed !== undefined) {
                assert(Number.isFinite(seed));
                assert(Number.isInteger(seed));
                assert(seed >= CS2_MIN_KEYCHAIN_SEED && seed <= CS2_MAX_KEYCHAIN_SEED);
            }
            if (x !== undefined) {
                assert(Number.isFinite(x));
            }
            if (y !== undefined) {
                assert(Number.isFinite(y));
            }
            if (z !== undefined) {
                assert(Number.isFinite(z));
            }
        }
    }

    private validatePatches(patches?: CS2BaseInventoryItem["patches"], item?: CS2EconomyItem): void {
        if (patches === undefined) {
            return;
        }
        assert(item === undefined || item.isAgent());
        for (const [key, patchId] of Object.entries(patches)) {
            const slot = parseInt(key, 10);
            assert(slot >= 0 && slot <= CS2_MAX_PATCHES - 1);
            assert(patchId === undefined || this.economy.getById(patchId).isPatch());
        }
    }

    public removeInvalidItemReferences(item: CS2BaseInventoryItem): void {
        if (item.patches !== undefined) {
            for (const [slot, patchId] of Object.entries(item.patches)) {
                if (!this.economy.items.has(patchId)) {
                    delete item.patches[slot];
                }
            }
        }
        if (item.stickers !== undefined) {
            for (const [slot, sticker] of Object.entries(item.stickers)) {
                if (!this.economy.items.has(sticker.id)) {
                    delete item.stickers[slot];
                }
            }
        }
    }

    public validateBaseInventoryItem({
        id,
        keychains,
        nameTag,
        patches,
        seed,
        statTrak,
        stickers,
        wear
    }: CS2BaseInventoryItem): void {
        const item = this.economy.getById(id);
        this.economy.validateWear(wear, item);
        this.economy.validateSeed(seed, item);
        this.economy.validateNametag(nameTag, item);
        this.economy.validateStatTrak(statTrak, item);
        this.validateAddable(item);
        this.validatePatches(patches, item);
        this.validateStickers(stickers, item);
        this.validateKeychains(keychains, item);
    }

    private toInventoryItems(items: Record<number, CS2BaseInventoryItem>): Map<number, CS2InventoryItem> {
        return new Map(
            Object.entries(items)
                .filter(([, { id }]) => this.economy.items.has(id))
                .map(([key, value]) => {
                    this.removeInvalidItemReferences(value);
                    const uid = parseInt(key, 10);
                    return [uid, new CS2InventoryItem(this, uid, value, this.economy.getById(value.id))] as const;
                })
        );
    }

    private toBaseInventoryItems(items: Map<number, CS2InventoryItem>): Record<number, CS2BaseInventoryItem> {
        return Object.fromEntries(Array.from(items).map(([key, value]) => [key, value.asBase()]));
    }

    stringify(): string {
        return JSON.stringify(this.getData());
    }

    isFull(): boolean {
        return this.items.size >= this.options.maxItems;
    }

    add(item: CS2BaseInventoryItem): this {
        assert(!this.isFull());
        const uid = getNextUid(this.items);
        const economyItem = this.economy.getById(item.id);
        assert(!economyItem.isStub());
        this.items.set(
            uid,
            new CS2InventoryItem(
                this,
                uid,
                Object.assign(item, {
                    equipped: undefined,
                    equippedCT: undefined,
                    equippedT: undefined,
                    updatedAt: getTimestamp()
                }),
                economyItem
            )
        );
        return this;
    }

    private addInventoryItem(item: CS2InventoryItem): this {
        assert(!this.isFull());
        const uid = getNextUid(this.items);
        item.equipped = undefined;
        item.equippedCT = undefined;
        item.equippedT = undefined;
        item.uid = uid;
        item.updatedAt = getTimestamp();
        this.items.set(uid, item);
        return this;
    }

    addWithNametag(nameTagUid: number, id: number, nameTag: string): this {
        this.get(nameTagUid).expectNameTag();
        this.economy.requireNametag(nameTag);
        this.items.delete(nameTagUid);
        this.add({ id, nameTag });
        return this;
    }

    addWithSticker(stickerUid: number, id: number, slot: number): this {
        const sticker = this.get(stickerUid).expectSticker();
        this.items.delete(stickerUid);
        this.add({
            id,
            stickers: { [slot]: { id: sticker.id } }
        });
        return this;
    }

    edit(itemUid: number, properties: Partial<CS2BaseInventoryItem>): this {
        const item = this.get(itemUid);
        assert(properties.id === undefined || properties.id === item.id);
        item.edit(properties, { updatedAt: getTimestamp() });
        return this;
    }

    equip(itemUid: number, team?: CS2TeamValues): this {
        const item = this.get(itemUid);
        assert(item.equipped === undefined);
        assert(team !== CS2Team.CT || item.equippedCT === undefined);
        assert(team !== CS2Team.T || item.equippedT === undefined);
        assert(CS2_INVENTORY_EQUIPPABLE_ITEMS.includes(item.type));
        assert(team === undefined || item.teams?.includes(team));
        assert(team !== undefined || item.teams === undefined);
        for (const [otherUid, otherItem] of this.items) {
            if (itemUid === otherUid) {
                otherItem.equipped = team === undefined ? true : undefined;
                otherItem.equippedCT = team === CS2Team.CT ? true : otherItem.equippedCT;
                otherItem.equippedT = team === CS2Team.T ? true : otherItem.equippedT;
            } else {
                if (otherItem.type === item.type && (!item.isWeapon() || otherItem.model === item.model)) {
                    otherItem.equipped = team === undefined ? undefined : otherItem.equipped;
                    otherItem.equippedCT = team === CS2Team.CT ? undefined : otherItem.equippedCT;
                    otherItem.equippedT = team === CS2Team.T ? undefined : otherItem.equippedT;
                }
            }
        }
        return this;
    }

    unequip(uid: number, team?: CS2TeamValues): this {
        const item = this.get(uid);
        item.equipped = team === undefined ? undefined : item.equipped;
        item.equippedCT = team === CS2Team.CT ? undefined : item.equippedCT;
        item.equippedT = team === CS2Team.T ? undefined : item.equippedT;
        return this;
    }

    unlockContainer(unlockedItem: CS2UnlockedItem, containerUid: number, keyUid?: number): this {
        const containerItem = this.get(containerUid);
        this.economy.validateUnlockedItem(containerItem, unlockedItem);
        const keyItem = keyUid !== undefined ? this.get(keyUid) : undefined;
        this.economy.validateContainerAndKey(containerItem, keyItem);
        this.items.delete(containerUid);
        if (keyUid !== undefined) {
            this.items.delete(keyUid);
        }
        this.add({
            ...unlockedItem.attributes,
            id: unlockedItem.id,
            updatedAt: getTimestamp()
        });
        return this;
    }

    renameItem(nameTagUid: number, renameableUid: number, nameTag?: string): this {
        nameTag = this.economy.trimNametag(nameTag);
        this.get(nameTagUid).expectNameTag();
        const renameable = this.get(renameableUid);
        this.economy.validateNametag(nameTag, renameable);
        renameable.nameTag = nameTag;
        renameable.updatedAt = getTimestamp();
        this.items.delete(nameTagUid);
        return this;
    }

    renameStorageUnit(storageUid: number, nameTag: string): this {
        const trimmed = this.economy.trimNametag(nameTag);
        const storageUnit = this.get(storageUid);
        storageUnit.expectStorageUnit();
        this.economy.requireNametag(trimmed);
        storageUnit.nameTag = trimmed;
        storageUnit.updatedAt = getTimestamp();
        return this;
    }

    isStorageUnitFull(storageUid: number): boolean {
        return this.get(storageUid).storage?.size === this.options.storageUnitMaxItems;
    }

    getStorageUnitSize(storageUid: number): number {
        return this.get(storageUid).storage?.size ?? 0;
    }

    isStorageUnitFilled(storageUid: number): boolean {
        return this.getStorageUnitSize(storageUid) > 0;
    }

    canDepositToStorageUnit(storageUid: number, size = 1): boolean {
        return (
            this.get(storageUid).nameTag !== undefined &&
            this.getStorageUnitSize(storageUid) + size <= this.options.storageUnitMaxItems
        );
    }

    canRetrieveFromStorageUnit(storageUid: number, size = 1): boolean {
        return this.getStorageUnitSize(storageUid) - size >= 0 && this.size() + size <= this.options.maxItems;
    }

    getStorageUnitItems(storageUid: number): CS2InventoryItem[] {
        return Array.from(this.get(storageUid).storage?.values() ?? []);
    }

    depositToStorageUnit(storageUid: number, depositUids: number[]): this {
        const item = this.get(storageUid);
        item.expectStorageUnit();
        assert(depositUids.length > 0);
        assert(this.canDepositToStorageUnit(storageUid, depositUids.length));
        for (const sourceUid of depositUids) {
            assert(!this.get(sourceUid).isStorageUnit());
        }
        const storage = item.storage ?? new Map<number, CS2InventoryItem>();
        for (const sourceUid of depositUids) {
            const uid = getNextUid(storage);
            storage.set(
                uid,
                Object.assign(this.get(sourceUid), {
                    equipped: undefined,
                    equippedCT: undefined,
                    equippedT: undefined,
                    uid
                })
            );
            this.items.delete(sourceUid);
        }
        item.storage = storage;
        item.updatedAt = getTimestamp();
        return this;
    }

    retrieveFromStorageUnit(storageUid: number, retrieveUids: number[]): this {
        const item = this.get(storageUid);
        item.expectStorageUnit();
        const storage = item.storage;
        assert(storage !== undefined);
        assert(retrieveUids.length > 0);
        assert(this.canRetrieveFromStorageUnit(storageUid, retrieveUids.length));
        for (const uid of retrieveUids) {
            assert(storage.has(uid));
        }
        for (const uid of retrieveUids) {
            const item = ensure(storage.get(uid));
            this.addInventoryItem(item);
            storage.delete(uid);
        }
        item.storage = storage.size > 0 ? storage : undefined;
        item.updatedAt = getTimestamp();
        return this;
    }

    applyItemSticker(targetUid: number, stickerUid: number, slot: number): this {
        assert(slot >= 0 && slot <= CS2_MAX_STICKERS - 1);
        const target = this.get(targetUid);
        const sticker = this.get(stickerUid);
        assert(target.hasStickers());
        sticker.expectSticker();
        target.stickers ??= new Map();
        assert(target.stickers.get(slot) === undefined);
        target.stickers.set(slot, { id: sticker.id });
        target.updatedAt = getTimestamp();
        this.items.delete(stickerUid);
        return this;
    }

    scrapeItemSticker(targetUid: number, slot: number): this {
        const target = this.get(targetUid);
        assert(target.stickers !== undefined);
        const sticker = ensure(target.stickers.get(slot));
        const wear = sticker.wear ?? 0;
        const nextWear = float(wear + CS2_STICKER_WEAR_FACTOR);
        if (nextWear > CS2_MAX_WEAR) {
            target.stickers.delete(slot);
            if (target.stickers.size === 0) {
                target.stickers = undefined;
            }
            return this;
        }
        sticker.wear = nextWear;
        target.updatedAt = getTimestamp();
        return this;
    }

    applyItemPatch(targetUid: number, patchUid: number, slot: number): this {
        assert(slot >= 0 && slot <= CS2_MAX_PATCHES - 1);
        const target = this.get(targetUid).expectAgent();
        const patch = this.get(patchUid).expectPatch();
        target.patches ??= new Map();
        assert(target.patches.get(slot) === undefined);
        target.patches.set(slot, patch.id);
        target.updatedAt = getTimestamp();
        this.items.delete(patchUid);
        return this;
    }

    removeItemPatch(targetUid: number, slot: number): this {
        const target = this.get(targetUid);
        assert(target.patches !== undefined);
        assert(target.patches.get(slot) !== undefined);
        target.patches.delete(slot);
        if (target.patches.size === 0) {
            target.patches = undefined;
        }
        target.updatedAt = getTimestamp();
        return this;
    }

    incrementItemStatTrak(targetUid: number): this {
        const target = this.get(targetUid);
        assert(target.statTrak !== undefined);
        if (target.statTrak < CS2_MAX_STATTRAK) {
            target.statTrak++;
            target.updatedAt = getTimestamp();
        }
        return this;
    }

    swapItemsStatTrak(statTrakSwapToolUid: number, fromUid: number, toUid: number): this {
        assert(fromUid !== toUid);
        this.get(statTrakSwapToolUid).expectStatTrakSwapTool();
        const fromItem = this.get(fromUid);
        const toItem = this.get(toUid);
        assert(fromItem.statTrak !== undefined && toItem.statTrak !== undefined);
        assert(fromItem.type === toItem.type);
        assert(fromItem.isMusicKit() || fromItem.def === toItem.def);
        const fromStattrak = fromItem.statTrak;
        fromItem.statTrak = toItem.statTrak;
        fromItem.updatedAt = getTimestamp();
        toItem.statTrak = fromStattrak;
        toItem.updatedAt = getTimestamp();
        this.items.delete(statTrakSwapToolUid);
        return this;
    }

    remove(uid: number): this {
        this.items.delete(uid);
        return this;
    }

    removeAll(): this {
        this.items.clear();
        return this;
    }

    get(uid: number): CS2InventoryItem {
        return ensure(this.items.get(uid));
    }

    getAll(): CS2InventoryItem[] {
        return Array.from(this.items.values());
    }

    getAllAsBase(): CS2BaseInventoryItem[] {
        return Object.values(this.toBaseInventoryItems(this.items));
    }

    setAll(items: Map<number, CS2InventoryItem>): this {
        this.items = items;
        return this;
    }

    getData(): CS2InventoryData {
        return {
            items: this.toBaseInventoryItems(this.items),
            version: CS2_INVENTORY_VERSION
        };
    }

    size(): number {
        return this.items.size;
    }

    move(options: Partial<CS2InventorySpec> = {}): CS2Inventory {
        return new CS2Inventory({
            ...this.options,
            economy: this.economy,
            ...options
        }).setAll(this.items);
    }
}

export class CS2InventoryItem
    extends CS2EconomyItem
    implements Interface<Omit<CS2BaseInventoryItem, "keychains" | "patches" | "stickers" | "storage">>
{
    containerId: number | undefined;
    equipped: boolean | undefined;
    equippedCT: boolean | undefined;
    equippedT: boolean | undefined;
    keychains: Map<number, RecordValue<CS2BaseInventoryItem["keychains"]>> | undefined;
    nameTag: string | undefined;
    patches: Map<number, number> | undefined;
    seed: number | undefined;
    statTrak: number | undefined;
    stickers: Map<number, RecordValue<CS2BaseInventoryItem["stickers"]>> | undefined;
    storage: Map<number, CS2InventoryItem> | undefined;
    updatedAt: number | undefined;
    wear: number | undefined;

    private assign({ keychains, patches, stickers, storage }: Partial<CS2BaseInventoryItem>): void {
        if (patches !== undefined) {
            this.patches = new Map(
                Object.entries(patches)
                    .filter(([, patchId]) => this.economy.items.has(patchId))
                    .map(([slot, patchId]) => [parseInt(slot, 10), patchId])
            );
        }
        if (stickers !== undefined) {
            this.stickers = new Map(
                Object.entries(stickers)
                    .filter(([, { id }]) => this.economy.items.has(id))
                    .map(([slot, sticker]) => [parseInt(slot, 10), sticker])
            );
        }
        if (keychains !== undefined) {
            this.keychains = new Map(
                Object.entries(keychains)
                    .filter(([, { id }]) => this.economy.items.has(id))
                    .map(([slot, keychain]) => [parseInt(slot, 10), keychain])
            );
        }
        if (storage !== undefined) {
            assert(this.isStorageUnit());
            this.storage = new Map(
                Object.entries(storage)
                    .filter(([, { id }]) => this.economy.items.has(id))
                    .map(([key, value]) => {
                        this.inventory.removeInvalidItemReferences(value);
                        const economyItem = this.economy.getById(value.id);
                        assert(value.storage === undefined);
                        const uid = parseInt(key, 10);
                        return [uid, new CS2InventoryItem(this.inventory, uid, value, economyItem)];
                    })
            );
        }
    }

    constructor(
        private inventory: CS2Inventory,
        public uid: number,
        baseInventoryItem: CS2BaseInventoryItem,
        { economy, item, language }: CS2EconomyItem
    ) {
        super(economy, item, language);
        inventory.validateBaseInventoryItem(baseInventoryItem);
        Object.assign(this, baseInventoryItem);
        this.assign(baseInventoryItem);
    }

    edit(...sources: Partial<CS2BaseInventoryItem>[]): void {
        for (const source of sources) {
            Object.assign(this, source);
            this.assign(source);
        }
    }

    allStickers(): [number, MapValue<CS2InventoryItem["stickers"]> | undefined][] {
        const entries: [number, MapValue<CS2InventoryItem["stickers"]> | undefined][] = [];
        for (let slot = 0; slot < CS2_MAX_STICKERS; slot++) {
            const sticker = this.stickers?.get(slot);
            entries.push([slot, sticker]);
        }
        return entries;
    }

    someStickers(): [number, MapValue<CS2InventoryItem["stickers"]>][] {
        return this.allStickers().filter(
            (value): value is [number, MapValue<CS2InventoryItem["stickers"]>] => value[1] !== undefined
        );
    }

    getStickersCount(): number {
        return this.stickers?.size ?? 0;
    }

    allKeychains(): [number, MapValue<CS2InventoryItem["keychains"]> | undefined][] {
        const entries: [number, MapValue<CS2InventoryItem["keychains"]> | undefined][] = [];
        for (let slot = 0; slot < CS2_MAX_KEYCHAINS; slot++) {
            const keychain = this.keychains?.get(slot);
            entries.push([slot, keychain]);
        }
        return entries;
    }

    someKeychains(): [number, MapValue<CS2InventoryItem["keychains"]>][] {
        return this.allKeychains().filter(
            (value): value is [number, MapValue<CS2InventoryItem["keychains"]>] => value[1] !== undefined
        );
    }

    getKeychainsCount(): number {
        return this.keychains?.size ?? 0;
    }

    allPatches(): [number, number | undefined][] {
        const entries: [number, number | undefined][] = [];
        for (let slot = 0; slot < CS2_MAX_PATCHES; slot++) {
            const patch = this.patches?.get(slot);
            entries.push([slot, patch]);
        }
        return entries;
    }

    somePatches(): [number, number][] {
        return this.allPatches().filter((value): value is [number, number] => value[1] !== undefined);
    }

    getPatchesCount(): number {
        return this.patches?.size ?? 0;
    }

    getWear(): number {
        return this.wear ?? this.wearMin ?? CS2_MIN_WEAR;
    }

    getStickerWear(slot: number): number {
        return this.stickers?.get(slot)?.wear ?? CS2_MIN_STICKER_WEAR;
    }

    getKeychainSeed(slot: number): number {
        return this.keychains?.get(slot)?.seed ?? CS2_MIN_KEYCHAIN_SEED;
    }

    override getImage(wear?: number): string {
        return super.getImage(wear ?? this.getWear());
    }

    asBase(): CS2BaseInventoryItem {
        return {
            containerId: this.containerId,
            equipped: this.equipped,
            equippedCT: this.equippedCT,
            equippedT: this.equippedT,
            id: this.id,
            keychains: this.keychains !== undefined ? Object.fromEntries(this.keychains) : undefined,
            nameTag: this.nameTag,
            patches: this.patches !== undefined ? Object.fromEntries(this.patches) : undefined,
            seed: this.seed,
            statTrak: this.statTrak,
            stickers: this.stickers !== undefined ? Object.fromEntries(this.stickers) : undefined,
            storage:
                this.storage !== undefined
                    ? Object.fromEntries(Array.from(this.storage).map(([key, value]) => [key, value.asBase()]))
                    : undefined,
            updatedAt: this.updatedAt,
            wear: this.wear
        } satisfies Interface<CS2BaseInventoryItem>;
    }
}
