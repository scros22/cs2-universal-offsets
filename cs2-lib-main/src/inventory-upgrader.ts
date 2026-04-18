/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Ian Lucas. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/

import { CS2Economy, CS2EconomyInstance } from "./economy.ts";
import { type CS2InventoryData, CS2_INVENTORY_VERSION } from "./inventory.ts";

const upgrades: Record<
    number,
    (
        data: any,
        economy: CS2EconomyInstance
    ) => {
        items: {
            [k: string]: any;
        };
        version: number;
    }
> = {
    1: (data: any, economy: CS2EconomyInstance) => {
        function walkV0(v0: any) {
            const v1: any = {};
            for (let [key, value] of Object.entries<any>(v0)) {
                switch (key) {
                    case "caseid":
                        key = "containerId";
                        break;
                    case "equipped":
                    case "equippedCT":
                    case "equippedT":
                        if (economy.get(v0.id).isPatch()) {
                            value = undefined;
                        }
                        break;
                    case "nametag":
                        key = "nameTag";
                        break;
                    case "stattrak":
                        key = "statTrak";
                        break;
                    case "stickers":
                        value =
                            value !== undefined
                                ? Object.fromEntries(
                                      value
                                          .map((stickerId: number, slot: number) => [
                                              slot,
                                              {
                                                  id: stickerId,
                                                  wear: v0.stickerswear?.[slot] || undefined
                                              }
                                          ])
                                          .filter(([, { id }]: any) => id !== 0)
                                  )
                                : undefined;
                        break;
                    case "stickerswear":
                        continue;
                    case "storage":
                        value =
                            value !== undefined
                                ? Object.fromEntries(value.map((v0: any) => [v0.uid, walkV0(v0)]))
                                : undefined;
                        break;
                    case "uid":
                        continue;
                    case "updatedat":
                        key = "updatedAt";
                        break;
                }
                v1[key] = value;
            }
            return v1;
        }
        return {
            items: Object.fromEntries(data.map((v0: any) => [v0.uid, walkV0(v0)])),
            version: 1
        };
    }
};

export function resolveInventoryData(
    stringValue?: string,
    economy: CS2EconomyInstance = CS2Economy
): CS2InventoryData | undefined {
    try {
        if (!stringValue) {
            return undefined;
        }
        let value = JSON.parse(stringValue);
        const currentVersion = value.version ?? 0;
        for (let i = currentVersion + 1; i <= CS2_INVENTORY_VERSION; i++) {
            const upgrade = upgrades[i];
            if (upgrade !== undefined) {
                value = upgrade(value, economy);
            }
        }
        return value;
    } catch {
        return undefined;
    }
}
