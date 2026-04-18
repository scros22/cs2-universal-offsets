/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Ian Lucas. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/

import { expect, test } from "vitest";
import { CS2KeyValues3 } from "./keyvalues3.ts";

test("keyvalues3 basics", () => {
    expect(
        CS2KeyValues3.parse(`<!-- kv3 encoding:text:version{e21c7f3c-8a33-41c5-9977-a76d3a32aa0d} format:generic:version{7412167c-06e9-4698-aff2-e63eb59037e7} -->
{
	boolValue = false
	intValue = 128
	doubleValue = 64.000000
	stringValue = "hello world"
	stringThatIsAResourceReference = resource:"particles/items3_fx/star_emblem.vpcf"
	arrayValue =
	[
		1,
		2,
	]
	objectValue =
	{
		n = 5
		s = "foo"
	}
}`)
    ).toStrictEqual({
        boolValue: false,
        intValue: 128,
        doubleValue: 64,
        stringValue: "hello world",
        stringThatIsAResourceReference: "resource:particles/items3_fx/star_emblem.vpcf",
        arrayValue: [1, 2],
        objectValue: {
            n: 5,
            s: "foo"
        }
    });
});
