/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Ian Lucas. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/

import { CS2Item } from "../src/economy-types";

export type CS2Language = {
    lang: {
        Tokens: { [key: string]: string | undefined };
    };
};

export type CS2GameItems = {
    items_game: {
        alternate_icons2: {
            weapon_icons: {
                [weaponIconKey: string]: {
                    icon_path: string;
                };
            };
        };
        item_sets: {
            [itemSetKey: string]: {
                name?: string;
                set_description?: string;
                items: {
                    [itemId: string]: string;
                };
            };
        };
        client_loot_lists: {
            [clientLootListKey: string]: {
                [itemOrLootListKey: string]: string;
            };
        };
        colors: {
            [colorKey: string]:
                | {
                      hex_color: string;
                  }
                | undefined;
        };
        graffiti_tints: {
            [graffitiTintKey: string]: {
                id: string;
                hex_color: string;
            };
        };
        items: {
            [itemIndex: string]: {
                associated_items?: {
                    [itemIndex: string]: string;
                };
                attributes?: {
                    ["set supply crate series"]?: {
                        attribute_class?: string;
                        value?: string;
                    };
                    ["pedestal display model"]?: string;
                    ["tournament event id"]?: {
                        value: string;
                    };
                };
                baseitem?: string;
                flexible_loadout_slot?: string;
                image_inventory?: string;
                image_unusual_item?: string;
                item_description?: string;
                item_name?: string;
                item_rarity: string;
                loot_list_name?: string;
                model_player?: string;
                name: string;
                prefab?: string;
                tags?: {
                    ItemSet?: {
                        tag_value?: string;
                    };
                    StickerCapsule?: {
                        [tagKey: string]: string;
                    };
                };
                tool?: {
                    type?: string;
                    use_string?: string;
                };
                used_by_classes?: Record<string, string>;
                vo_prefix?: string;
            };
        };
        keychain_definitions: {
            [keychainIndex: string]: {
                name: string;
                loc_name: string;
                loc_description: string;
                item_rarity: string;
                image_inventory: string;
                pedestal_display_model: string;
            };
        };
        music_definitions: {
            [musicIndex: string]: {
                image_inventory: string;
                loc_description: string;
                loc_name: string;
                name: string;
            };
        };
        paint_kits: {
            [paintKitKey: string]: {
                composite_material_path?: string;
                description_string?: string;
                description_tag?: string;
                name?: string;
                use_legacy_model?: string;
                wear_remap_max?: string;
                wear_remap_min?: string;
            };
        };
        paint_kits_rarity: {
            [paintKitKey: string]: string;
        };
        prefabs: {
            [prefabKey: string]:
                | {
                      image_inventory?: string;
                      item_class: string;
                      item_description?: string;
                      item_name: string;
                      item_rarity: string;
                      model_player?: string;
                      prefab: string;
                      used_by_classes: Record<string, string>;
                      visuals: {
                          weapon_type: string;
                      };
                  }
                | undefined;
        };
        rarities: {
            [rarityKey: string]: {
                color?: string;
            };
        };
        revolving_loot_lists: {
            [revolvingLootListKey: string]: string;
        };
        sticker_kits: {
            [stickerIndex: string]: {
                description_string?: string;
                item_name: string;
                item_rarity: string;
                name: string;
                patch_material?: string;
                sticker_material: string;
                tournament_event_id?: string;
            };
        };
    };
};

export type CompositeMaterialData = {
    m_Points: {
        m_vecCompositeMaterialAssemblyProcedures: {
            m_vecCompositeInputContainers: {
                m_strAlias: string;
                m_vecLooseVariables: {
                    m_strName: string;
                    m_strTextureRuntimeResourcePath: string;
                }[];
            }[];
        }[];
    }[];
};

export type MaterialData = {
    m_textureParams: { m_name: string; m_pValue: string }[];
};

export type ModelData = {
    StickerMarkup?: {
        Index: number;
        LegacyModel: boolean;
        Offset: number[];
        Rotation: number;
        Scale: number;
    }[];
};

export type CS2ExtendedItem = CS2Item & {
    className?: string;
    descToken?: string;
    nameToken?: string;
};

export type CS2ExportItem = CS2Item & {
    className: undefined;
    descToken: undefined;
    nameToken: undefined;
};
