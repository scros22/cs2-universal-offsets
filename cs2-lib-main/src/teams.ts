/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Ian Lucas. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/

import { type EnumValues } from "./utils.ts";

export const CS2Team = {
    None: 0,
    T: 2,
    CT: 3
} as const;

export type CS2TeamValues = EnumValues<typeof CS2Team>;

export function toggleCS2Team(team: CS2TeamValues): CS2TeamValues {
    return team === CS2Team.CT ? CS2Team.T : CS2Team.CT;
}
