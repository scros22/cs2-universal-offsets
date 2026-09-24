// Thin client for the cs2-sdk web API plus an in-memory index used for
// autocomplete. The web server is the single source of truth; the bot never
// reads the dump files itself.

import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

const __dirname = path.dirname(fileURLToPath(import.meta.url));
export const ROOT = path.resolve(__dirname, '..');
export const CFG = JSON.parse(fs.readFileSync(path.join(ROOT, 'config.json'), 'utf8'));

export const API = CFG.apiBase ?? 'http://127.0.0.1:7777/api';
export const SITE = (CFG.brand?.url ?? 'https://cs2-sdk.com').replace(/\/$/, '');
export const SESSION = CFG.session ?? 'latest';
export const GITHUB_REPO = CFG.githubRepo ?? 'scros22/cs2-universal-offsets';

const TIMEOUT_MS = 10_000;

async function getJSON(url, { timeout = TIMEOUT_MS } = {}) {
  const ctl = new AbortController();
  const t = setTimeout(() => ctl.abort(), timeout);
  try {
    const r = await fetch(url, { headers: { Accept: 'application/json', 'User-Agent': 'cs2-sdk-discord-bot' }, signal: ctl.signal });
    if (!r.ok) throw new Error(`${url.replace(API, '/api')} -> HTTP ${r.status}`);
    return await r.json();
  } finally {
    clearTimeout(t);
  }
}

const q = (o) => Object.entries(o).filter(([, v]) => v != null && v !== '').map(([k, v]) => `${k}=${encodeURIComponent(v)}`).join('&');

// --- endpoints ----------------------------------------------------------------
export const api = {
  summary:    () => getJSON(`${API}/summary?${q({ session: SESSION })}`),
  bundle:     () => getJSON(`${API}/bundle?${q({ session: SESSION })}`),
  offsets:    () => getJSON(`${API}/offsets?${q({ session: SESSION, detail: 1 })}`),
  schemas:    (module) => getJSON(`${API}/schemas?${q({ session: SESSION, module })}`),
  convars:    (query) => getJSON(`${API}/convars?${q({ session: SESSION, q: query })}`),
  weapons:    (query) => getJSON(`${API}/weapons?${q({ session: SESSION, q: query })}`),
  gameevents: (query) => getJSON(`${API}/gameevents?${q({ session: SESSION, q: query })}`),
  engine:     () => getJSON(`${API}/engine?${q({ session: SESSION })}`),
  /** Batch query: exactly what the site's /api/query does. `kinds` = {signatures:[..], offsets:[..], schemas:[..], convars:[..]} */
  query: async (kinds, detail = true) => {
    const ctl = new AbortController();
    const t = setTimeout(() => ctl.abort(), TIMEOUT_MS);
    try {
      const r = await fetch(`${API}/query?${q({ session: SESSION })}`, {
        method: 'POST', signal: ctl.signal,
        headers: { 'Content-Type': 'application/json', Accept: 'application/json', 'User-Agent': 'cs2-sdk-discord-bot' },
        body: JSON.stringify({ ...kinds, detail }),
      });
      const data = await r.json().catch(() => null);
      if (!r.ok || !data) throw new Error(data?.error ?? `/api/query -> HTTP ${r.status}`);
      return data;
    } finally {
      clearTimeout(t);
    }
  },
  latestRelease: async () => {
    try {
      const r = await getJSON(`https://api.github.com/repos/${GITHUB_REPO}/releases/latest`, { timeout: 8000 });
      return { tag: r.tag_name, name: r.name, url: r.html_url, published_at: r.published_at };
    } catch {
      return null;
    }
  },
};

// --- autocomplete index -------------------------------------------------------
// Rebuilt by refreshIndex() on a timer and whenever the dump changes. Every
// list is an array of {name, ...} sorted by name; lookups are substring,
// case-insensitive, with prefix matches ranked first.

export const index = {
  builtAt: 0,
  generatedAt: null,
  build: null,
  summary: null,
  signatures: [],   // {name, aliases, module, rva, resolve, pattern, prototype}
  offsets: [],      // {name, hpp_name, module, value, kind, deref}
  classes: [],      // {name, module, parent, size, fields:[{name,offset,type}]}
  classCount: 0,
  interfaceCount: 0,
  convars: [],      // {name, value, type, flag_names, description}
  commands: [],     // {name, description, flag_names}
  weapons: [],
  gameevents: [],   // {name, id, local, fields:[{name,type}]}
  structs: [],      // engine structs
};

export async function refreshIndex() {
  const [summary, bundle, offsets, engine, convars, weapons, gameevents] = await Promise.all([
    api.summary(), api.bundle(), api.offsets(), api.engine(),
    api.convars(''), api.weapons(''), api.gameevents(''),
  ]);
  const sigs = (bundle.signatures?.signatures ?? []).filter((s) => s.found !== false && s.pattern);
  const classes = [];
  const mods = bundle.schemaModules ?? bundle.manifest?.modules?.map((m) => m.replace('.dll', '_dll')) ?? [];
  const schemaResults = await Promise.allSettled(mods.map((m) => api.schemas(m)));
  for (const r of schemaResults) {
    if (r.status !== 'fulfilled' || !r.value?.classes) continue;
    for (const c of r.value.classes) classes.push({ ...c, module: r.value.module });
  }
  let ifaces = 0;
  for (const m of Object.values(bundle.vtables ?? {})) ifaces += Object.keys(m ?? {}).length;

  index.summary = summary;
  index.build = summary.build_number ?? bundle.manifest?.build_number ?? null;
  index.generatedAt = summary.generated_at ?? bundle.manifest?.generated_at ?? null;
  index.signatures = sigs.sort((a, b) => a.name.localeCompare(b.name));
  index.offsets = (offsets.offsets ?? []).sort((a, b) => a.name.localeCompare(b.name));
  index.classes = classes.sort((a, b) => a.name.localeCompare(b.name));
  index.classCount = classes.length;
  index.interfaceCount = ifaces;
  index.convars = (convars.convars ?? []).sort((a, b) => a.name.localeCompare(b.name));
  index.commands = (convars.commands ?? []).sort((a, b) => a.name.localeCompare(b.name));
  index.weapons = (weapons.weapons ?? []).sort((a, b) => a.name.localeCompare(b.name));
  index.gameevents = (gameevents.events ?? []).sort((a, b) => a.name.localeCompare(b.name));
  index.structs = engine.structs ?? [];
  index.builtAt = Date.now();
  return index;
}

/** Substring search over `items[key]` (and optional extra keys), prefix matches first. */
export function suggest(items, text, { key = 'name', extra = [], limit = 25 } = {}) {
  const t = (text ?? '').trim().toLowerCase();
  const scored = [];
  for (const it of items) {
    const names = [it[key], ...extra.flatMap((k) => (Array.isArray(it[k]) ? it[k] : it[k] ? [it[k]] : []))].filter(Boolean);
    let best = 0;
    for (const n of names) {
      const l = String(n).toLowerCase();
      if (!t) { best = 1; break; }
      if (l === t) { best = 4; break; }
      if (l.startsWith(t)) best = Math.max(best, 3);
      else if (l.includes(t)) best = Math.max(best, 2);
    }
    if (best) scored.push([best, it]);
  }
  scored.sort((a, b) => b[0] - a[0] || String(a[1][key]).localeCompare(String(b[1][key])));
  return scored.slice(0, limit).map((s) => s[1]);
}
