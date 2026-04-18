/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Ian Lucas. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/

import { randomBoolean } from "./economy-container.ts";
import { type CS2Map } from "./maps.ts";
import { type EnumValues, assert, ensure } from "./utils.ts";

export const CS2VetoAction = {
    Available: 0,
    Pick: 1,
    Ban: 2
} as const;

export type CS2VetoActionValues = EnumValues<typeof CS2VetoAction>;

export const CS2VetoType = {
    BO1: "bo1",
    BO2: "bo2",
    BO3: "bo3",
    BO5: "bo5",
    Custom: "custom"
} as const;

export type CS2VetoTypeValues = EnumValues<typeof CS2VetoType>;

export interface CS2VetoMap {
    mapname: string;
    value: CS2VetoActionValues;
    team?: number;
}

export class CS2Veto {
    private actions: CS2VetoActionValues[];
    private maps: CS2VetoMap[];
    private pickedMaps: string[] = [];
    private toggleTeam = false;

    constructor(type: CS2VetoTypeValues, maps: CS2Map[], actions?: CS2VetoActionValues[]) {
        assert(type !== "custom" || actions !== undefined);
        assert(maps.length === 7);
        assert(actions === undefined || actions.length === 6);
        this.maps = maps.map((map) => ({
            mapname: map.mapname,
            value: CS2VetoAction.Available
        }));
        switch (type) {
            case CS2VetoType.BO1:
                this.actions = [
                    CS2VetoAction.Ban,
                    CS2VetoAction.Ban,
                    CS2VetoAction.Ban,
                    CS2VetoAction.Ban,
                    CS2VetoAction.Ban,
                    CS2VetoAction.Ban
                ];
                break;
            case CS2VetoType.BO2:
                this.toggleTeam = randomBoolean();
                this.actions = [
                    CS2VetoAction.Ban,
                    CS2VetoAction.Ban,
                    CS2VetoAction.Ban,
                    CS2VetoAction.Ban,
                    CS2VetoAction.Ban
                ];
                break;
            case CS2VetoType.BO3:
                this.actions = [
                    CS2VetoAction.Ban,
                    CS2VetoAction.Ban,
                    CS2VetoAction.Pick,
                    CS2VetoAction.Pick,
                    CS2VetoAction.Ban,
                    CS2VetoAction.Ban
                ];
                break;
            case CS2VetoType.BO5:
                this.actions = [
                    CS2VetoAction.Ban,
                    CS2VetoAction.Ban,
                    CS2VetoAction.Pick,
                    CS2VetoAction.Pick,
                    CS2VetoAction.Pick,
                    CS2VetoAction.Pick
                ];
                break;
            case CS2VetoType.Custom:
                this.actions = ensure(actions);
                break;
        }
    }

    private getAvailableMaps(): CS2VetoMap[] {
        return this.maps.filter((map) => map.value === CS2VetoAction.Available);
    }

    private getMap(mapname: string): CS2VetoMap | undefined {
        return this.maps.find((map) => map.mapname === mapname);
    }

    private getAvailableMapnames(): string[] {
        return this.getAvailableMaps().map((map) => map.mapname);
    }

    getCurrentAction(): CS2VetoActionValues | undefined {
        return this.actions[0];
    }

    getCurrentTeam(): number {
        return (this.toggleTeam ? 1 : 0) + (this.actions.length % 2) * (this.toggleTeam ? -1 : 1);
    }

    getRandomAvailableMapname(): string | undefined {
        const available = this.getAvailableMapnames();
        if (!available.length) {
            return undefined;
        }
        const index = Math.floor(Math.random() * available.length);
        const mapname = available[index];
        return mapname;
    }

    choose(mapname?: string): boolean {
        if (this.actions.length === 0) {
            return false;
        }
        if (mapname === undefined) {
            return this.random();
        }
        const map = this.getMap(mapname);
        if (map === undefined || map.value !== CS2VetoAction.Available) {
            return false;
        }
        const team = this.getCurrentTeam();
        const value = this.actions.shift();
        if (value === undefined) {
            return false;
        }
        if (value === CS2VetoAction.Pick) {
            this.pickedMaps.push(mapname);
        }
        this.maps = this.maps.map((map) => {
            if (map.mapname !== mapname) {
                return map;
            }
            return {
                ...map,
                value,
                team
            };
        });
        return true;
    }

    random(): boolean {
        var mapname = this.getRandomAvailableMapname();
        if (mapname === undefined) {
            return false;
        }
        return this.choose(mapname);
    }

    getState(): CS2VetoMap[] {
        return this.maps;
    }

    getMaps(): string[] {
        if (this.actions.length > 0) {
            return this.pickedMaps;
        }
        const available = this.getAvailableMapnames();
        return [...this.pickedMaps, ...available];
    }

    done(): boolean {
        return this.actions.length === 0;
    }
}
