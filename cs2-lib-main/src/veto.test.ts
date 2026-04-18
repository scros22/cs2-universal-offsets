/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Ian Lucas. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/

import { expect, test } from "vitest";
import { CS2_ACTIVE_MAP_POOL } from "./maps.ts";
import { CS2Veto, CS2VetoAction } from "./veto.ts";

test("bo1", () => {
    const veto = new CS2Veto("bo1", CS2_ACTIVE_MAP_POOL);
    expect(veto.getCurrentTeam()).toBe(0);
    expect(veto.choose("de_nuke")).toBe(true);
    expect(veto.getCurrentTeam()).toBe(1);
    expect(veto.choose("de_train")).toBe(true);
    expect(veto.getCurrentTeam()).toBe(0);
    expect(veto.choose("de_ancient")).toBe(true);
    expect(veto.getCurrentTeam()).toBe(1);
    expect(veto.choose("de_anubis")).toBe(true);
    expect(veto.getCurrentTeam()).toBe(0);
    expect(veto.choose("de_inferno")).toBe(true);
    expect(veto.getCurrentTeam()).toBe(1);
    expect(veto.choose("de_dust2")).toBe(true);
    expect(veto.choose("de_mirage")).toBe(false);
    expect(veto.getMaps()).toStrictEqual(["de_mirage"]);
});

test("bo2-a", () => {
    const veto = new CS2Veto("bo2", CS2_ACTIVE_MAP_POOL);
    // @ts-expect-error
    veto.toggleTeam = false;
    expect(veto.getCurrentTeam()).toBe(1);
    expect(veto.getCurrentAction()).toBe(CS2VetoAction.Ban);
    expect(veto.choose("de_nuke")).toBe(true);
    expect(veto.getCurrentTeam()).toBe(0);
    expect(veto.getCurrentAction()).toBe(CS2VetoAction.Ban);
    expect(veto.choose("de_train")).toBe(true);
    expect(veto.getCurrentTeam()).toBe(1);
    expect(veto.getCurrentAction()).toBe(CS2VetoAction.Ban);
    expect(veto.choose("de_ancient")).toBe(true);
    expect(veto.getCurrentTeam()).toBe(0);
    expect(veto.getCurrentAction()).toBe(CS2VetoAction.Ban);
    expect(veto.choose("de_anubis")).toBe(true);
    expect(veto.getCurrentTeam()).toBe(1);
    expect(veto.getCurrentAction()).toBe(CS2VetoAction.Ban);
    expect(veto.choose("de_inferno")).toBe(true);
    expect(veto.getCurrentAction()).toBe(undefined);
    expect(veto.choose("de_dust2")).toBe(false);
    expect(veto.choose("de_mirage")).toBe(false);
    expect(veto.getMaps()).toStrictEqual(["de_dust2", "de_mirage"]);
});

test("bo2-b", () => {
    const veto = new CS2Veto("bo2", CS2_ACTIVE_MAP_POOL);
    // @ts-expect-error
    veto.toggleTeam = true;
    expect(veto.getCurrentTeam()).toBe(0);
    expect(veto.choose("de_nuke")).toBe(true);
    expect(veto.getCurrentTeam()).toBe(1);
    expect(veto.choose("de_train")).toBe(true);
    expect(veto.getCurrentTeam()).toBe(0);
    expect(veto.choose("de_ancient")).toBe(true);
    expect(veto.getCurrentTeam()).toBe(1);
    expect(veto.choose("de_anubis")).toBe(true);
    expect(veto.getCurrentTeam()).toBe(0);
    expect(veto.choose("de_inferno")).toBe(true);
    expect(veto.choose("de_dust2")).toBe(false);
    expect(veto.choose("de_mirage")).toBe(false);
    expect(veto.getMaps()).toStrictEqual(["de_dust2", "de_mirage"]);
});

test("bo3", () => {
    const veto = new CS2Veto("bo3", CS2_ACTIVE_MAP_POOL);
    expect(veto.getCurrentTeam()).toBe(0);
    expect(veto.getCurrentAction()).toBe(CS2VetoAction.Ban);
    expect(veto.choose("de_nuke")).toBe(true);
    expect(veto.getCurrentTeam()).toBe(1);
    expect(veto.getCurrentAction()).toBe(CS2VetoAction.Ban);
    expect(veto.choose("de_train")).toBe(true);
    expect(veto.getCurrentTeam()).toBe(0);
    expect(veto.getCurrentAction()).toBe(CS2VetoAction.Pick);
    expect(veto.choose("de_ancient")).toBe(true);
    expect(veto.getCurrentTeam()).toBe(1);
    expect(veto.getCurrentAction()).toBe(CS2VetoAction.Pick);
    expect(veto.choose("de_anubis")).toBe(true);
    expect(veto.getCurrentTeam()).toBe(0);
    expect(veto.getCurrentAction()).toBe(CS2VetoAction.Ban);
    expect(veto.choose("de_inferno")).toBe(true);
    expect(veto.getCurrentTeam()).toBe(1);
    expect(veto.getCurrentAction()).toBe(CS2VetoAction.Ban);
    expect(veto.choose("de_dust2")).toBe(true);
    expect(veto.choose("de_mirage")).toBe(false);
    expect(veto.getMaps()).toStrictEqual(["de_ancient", "de_anubis", "de_mirage"]);
});

test("bo5", () => {
    const veto = new CS2Veto("bo5", CS2_ACTIVE_MAP_POOL);
    expect(veto.getCurrentTeam()).toBe(0);
    expect(veto.choose("de_nuke")).toBe(true);
    expect(veto.getCurrentTeam()).toBe(1);
    expect(veto.choose("de_train")).toBe(true);
    expect(veto.getCurrentTeam()).toBe(0);
    expect(veto.choose("de_ancient")).toBe(true);
    expect(veto.getCurrentTeam()).toBe(1);
    expect(veto.choose("de_anubis")).toBe(true);
    expect(veto.getCurrentTeam()).toBe(0);
    expect(veto.choose("de_inferno")).toBe(true);
    expect(veto.getCurrentTeam()).toBe(1);
    expect(veto.choose("de_dust2")).toBe(true);
    expect(veto.choose("de_mirage")).toBe(false);
    expect(veto.getMaps()).toStrictEqual(["de_ancient", "de_anubis", "de_inferno", "de_dust2", "de_mirage"]);
});

test("custom", () => {
    const veto = new CS2Veto("custom", CS2_ACTIVE_MAP_POOL, [
        CS2VetoAction.Pick,
        CS2VetoAction.Pick,
        CS2VetoAction.Ban,
        CS2VetoAction.Ban,
        CS2VetoAction.Ban,
        CS2VetoAction.Ban
    ]);
    expect(veto.getCurrentTeam()).toBe(0);
    expect(veto.choose("de_nuke")).toBe(true);
    expect(veto.getCurrentTeam()).toBe(1);
    expect(veto.choose("de_train")).toBe(true);
    expect(veto.getCurrentTeam()).toBe(0);
    expect(veto.choose("de_ancient")).toBe(true);
    expect(veto.getCurrentTeam()).toBe(1);
    expect(veto.choose("de_anubis")).toBe(true);
    expect(veto.getCurrentTeam()).toBe(0);
    expect(veto.choose("de_inferno")).toBe(true);
    expect(veto.getCurrentTeam()).toBe(1);
    expect(veto.choose("de_dust2")).toBe(true);
    expect(veto.choose("de_mirage")).toBe(false);
    expect(veto.getMaps()).toStrictEqual(["de_nuke", "de_train", "de_mirage"]);
});

test("random", () => {
    const veto = new CS2Veto("bo1", CS2_ACTIVE_MAP_POOL);
    expect(veto.getCurrentTeam()).toBe(0);
    expect(veto.choose()).toBe(true);
    expect(veto.getCurrentTeam()).toBe(1);
    expect(veto.choose()).toBe(true);
    expect(veto.getCurrentTeam()).toBe(0);
    expect(veto.choose()).toBe(true);
    expect(veto.getCurrentTeam()).toBe(1);
    expect(veto.choose()).toBe(true);
    expect(veto.getCurrentTeam()).toBe(0);
    expect(veto.choose()).toBe(true);
    expect(veto.getCurrentTeam()).toBe(1);
    expect(veto.choose()).toBe(true);
    expect(veto.choose()).toBe(false);
    expect(veto.getMaps().length).toBe(1);
});
