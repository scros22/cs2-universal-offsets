# cs2-lib

> A TypeScript library for manipulating Counter-Strike 2 data

This library contains items and data from Counter-Strike 2, including utility functions and classes for interacting with them using TypeScript.

## Install

```sh
npm install @ianlucas/cs2-lib
```

## Usage

```typescript
import { CS2Economy, CS2_ITEMS } from "@ianlucas/cs2-lib";
import { english } from "@ianlucas/cs2-lib/translations";

CS2Economy.use({
    items: CS2_ITEMS,
    language: english
});

const item = CS2Economy.getById(307);

item.name;
//=> "AWP | Dragon Lore"
```

## Download asset images

By default the library uses my CDN at `cdn.cstrike.app` for image URLs. If you need to self-host your content, you can find and download the latest images at [cs2-lib-assets repository](https://github.com/ianlucas/cs2-lib-assets). Make sure to point the assets base URL to your website:

```typescript
CS2Economy.use({
    assetsBaseUrl: "https://your-website.com/assets",
    items: CS2_ITEMS,
    language: english
});
```
