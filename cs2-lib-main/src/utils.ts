/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Ian Lucas. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/

export type EnumValues<T> = T[keyof T];

// @see https://stackoverflow.com/a/69895725
export type Interface<T extends object> = {
    [key in T extends any ? { [K in keyof T]-?: {} extends Pick<T, K> ? never : K }[keyof T] : never]: T[key];
} & {
    [key in T extends any ? { [K in keyof T]-?: {} extends Pick<T, K> ? K : never }[keyof T] : never]-?:
        | T[key]
        | undefined;
};

export type MapValue<T> = T extends Map<any, infer I> ? I : never;

export type RecordValue<T> = T extends Record<any, infer I> ? I : never;

export function compare<T, U>(var1: T, var2: U): boolean {
    return var1 === undefined || var1 === (typeof var1 === "boolean" ? var2 || false : var2);
}

export function float(literal: number, fractionDigits: number = 2): number {
    return parseFloat(literal.toFixed(fractionDigits));
}

export function ensure<T>(value: T, message?: string): NonNullable<T> & {} {
    assert(value !== undefined && value !== null, message);
    return value;
}

export function safe<T>(fn: () => T): false | T {
    try {
        return fn();
    } catch {
        return false;
    }
}

export function assert(condition: any, message?: string): asserts condition {
    if (condition) {
        return;
    }
    throw new Error(message);
}

export function fail(message?: string): never {
    throw new Error(message);
}

export function isNotUndefined<T>(value: T): value is NonNullable<T> {
    return value !== undefined;
}
