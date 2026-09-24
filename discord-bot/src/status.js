// The live status message (edited in place) and the new-dump announcement.
// State (message ids, last announced dump) lives in data/state.json.

import fs from 'node:fs';
import path from 'node:path';
import { api, index, refreshIndex, ROOT, CFG, GITHUB_REPO } from './api.js';
import { statusEmbed, announcementEmbed } from './embeds.js';

const DATA_DIR = path.join(ROOT, 'data');
const STATE_FILE = path.join(DATA_DIR, 'state.json');

function loadState() {
  try { return JSON.parse(fs.readFileSync(STATE_FILE, 'utf8')); } catch { return {}; }
}
function saveState(state) {
  fs.mkdirSync(DATA_DIR, { recursive: true });
  fs.writeFileSync(STATE_FILE, JSON.stringify(state, null, 2));
}

const chanName = (key) => CFG.channels?.[key] ?? key;

function findChannel(guild, key) {
  const want = chanName(key);
  if (/^\d{15,}$/.test(want)) return guild.channels.cache.get(want) ?? null;
  return guild.channels.cache.find((c) => c.name === want && c.isTextBased?.()) ?? null;
}

/** Edit the one status message (create it once per guild). */
async function upsertStatus(guild, state, release) {
  const channel = findChannel(guild, 'status');
  if (!channel) return console.warn(`[status] ${guild.name}: no #${chanName('status')} channel`);
  const key = `${guild.id}:status`;
  const prevIds = Array.isArray(state[key]) ? state[key] : state[key] ? [state[key]] : [];
  const payload = { embeds: [statusEmbed(release)] };
  let msg = null;
  for (const id of prevIds) {
    try { msg = await channel.messages.fetch(id); await msg.edit(payload); break; } catch { msg = null; }
  }
  if (!msg) {
    msg = await channel.send(payload);
    try { await msg.pin(); } catch { /* needs Manage Messages; fine without */ }
  }
  state[key] = [msg.id];
}

/** Post once per new dump (build number or dump time changed). */
async function announceIfNew(guild, state, release) {
  const key = `${guild.id}:lastDump`;
  const prev = state[key] ?? { build: state[`${guild.id}:lastBuild`] ?? null, generatedAt: null };
  const cur = { build: index.build, generatedAt: index.generatedAt };
  if (!cur.build || (prev.build === cur.build && prev.generatedAt === cur.generatedAt)) return;
  const channel = findChannel(guild, 'announcements');
  if (channel) {
    const notesUrl = release ? `https://github.com/${GITHUB_REPO}/releases/tag/${release.tag}` : null;
    await channel.send({ embeds: [announcementEmbed(prev, release, notesUrl)] });
    console.log(`[announce] ${guild.name}: build ${prev.build ?? '?'} -> ${cur.build}`);
  }
  state[key] = cur;
  delete state[`${guild.id}:lastBuild`];
}

/** Old versions mirrored whole tables into channels; those messages are stale. Remove once. */
async function cleanupLegacy(guild, state) {
  for (const key of ['signatures', 'vtables', 'offsets', 'features', 'welcome']) {
    const k = `${guild.id}:${key}`;
    const ids = state[k];
    if (!Array.isArray(ids)) continue;
    const channel = findChannel(guild, key);
    for (const id of ids) {
      try { const m = await channel?.messages.fetch(id); await m?.delete(); } catch { /* gone already */ }
    }
    delete state[k];
    console.log(`[cleanup] ${guild.name}: removed legacy ${key} mirror (${ids.length} messages)`);
  }
}

let running = false;
export async function updateStatus(client, { force = false } = {}) {
  if (running) return;
  running = true;
  try {
    if (force || !index.builtAt || Date.now() - index.builtAt > 60_000) {
      try { await refreshIndex(); }
      catch (err) { console.error('[status] API refresh failed:', err.message); if (!index.builtAt) return; }
    }
    const release = await api.latestRelease();
    const state = loadState();
    for (const guild of client.guilds.cache.values()) {
      try {
        await cleanupLegacy(guild, state);
        await upsertStatus(guild, state, release);
        await announceIfNew(guild, state, release);
      } catch (err) {
        console.error(`[status] ${guild.name}:`, err.message);
      }
    }
    saveState(state);
  } finally {
    running = false;
  }
}
