// Embed builders. Every builder stays inside Discord's limits (title 256,
// description 4096, field value 1024, 25 fields, 6000 total) by construction.

import { EmbedBuilder } from 'discord.js';
import { SITE, index } from './api.js';

export const ACCENT = 0xfbac18;   // site --color-accent
export const RED = 0xe5484d;

const hex = (v) => {
  if (v == null) return '—';
  if (typeof v === 'string') return v.startsWith('0x') ? v : `0x${parseInt(v, 10).toString(16).toUpperCase()}`;
  return `0x${Number(v).toString(16).toUpperCase()}`;
};
const code = (s, lang = '') => '```' + lang + '\n' + s + '\n```';
const clip = (s, n) => (s.length > n ? s.slice(0, n - 1) + '…' : s);
const link = (tab, query) => `${SITE}/?tab=${tab}${query ? `&q=${encodeURIComponent(query)}` : ''}`;
const ts = (iso) => (iso ? `<t:${Math.floor(new Date(iso).getTime() / 1000)}:R>` : '—');

export function base(title, url) {
  const e = new EmbedBuilder().setColor(ACCENT).setTitle(clip(title, 256));
  if (url) e.setURL(url);
  e.setFooter({ text: `cs2-sdk.com · build ${index.build ?? '?'}` });
  return e;
}

export function errorEmbed(msg) {
  return new EmbedBuilder().setColor(RED).setDescription(clip(msg, 4000));
}

// --- status / announcement ------------------------------------------------------
export function statusEmbed(release) {
  const s = index.summary ?? {};
  const sig = index.signatures.length;
  const e = base('CS2 SDK — Live Status', SITE)
    .setDescription(`Live dump status from **cs2-sdk.com**. Use \`/sig\`, \`/offset\`, \`/schema\`, \`/convar\`, \`/search\` here, or the batch API (\`/api\`).`)
    .addFields(
      { name: 'Build', value: `\`${index.build ?? '?'}\``, inline: true },
      { name: 'Dumped', value: ts(index.generatedAt), inline: true },
      { name: 'Latest release', value: release ? `[${release.tag}](${release.url})` : '—', inline: true },
      { name: 'Signatures', value: `**${sig.toLocaleString()}** functions${index.summary ? '' : ''}`, inline: true },
      { name: 'Offsets', value: `**${(s.offsets ?? index.offsets.length).toLocaleString()}** globals`, inline: true },
      { name: 'Schema classes', value: `**${index.classCount.toLocaleString()}**`, inline: true },
      { name: 'Interfaces', value: `**${index.interfaceCount.toLocaleString()}** vtables`, inline: true },
      { name: 'ConVars', value: `**${(s.convars ?? 0).toLocaleString()}** + ${(s.commands ?? 0).toLocaleString()} commands`, inline: true },
      { name: 'Weapons', value: `**${s.weapons ?? 0}**`, inline: true },
      { name: 'Game events', value: `**${s.gameevents ?? 0}**`, inline: true },
      { name: 'Net messages', value: `**${s.netmessages ?? 0}**`, inline: true },
      { name: 'Engine structs', value: `**${s.engine ?? 0}**`, inline: true },
    )
    .setTimestamp(new Date());
  e.setFooter({ text: 'cs2-sdk.com · updates automatically · last checked' });
  return e;
}

export function announcementEmbed(prev, release, notesUrl) {
  const s = index.summary ?? {};
  const lines = [
    `A fresh dump for CS2 build **${index.build}** is live on **[cs2-sdk.com](${SITE})**${prev?.build && prev.build !== index.build ? ` (previous: ${prev.build})` : ''}.`,
    '',
    `• **${index.signatures.length.toLocaleString()}** signatures · **${(s.offsets ?? 0).toLocaleString()}** offsets · **${index.classCount.toLocaleString()}** schema classes`,
    `• **${(s.convars ?? 0).toLocaleString()}** convars · **${s.weapons ?? 0}** weapons · **${s.gameevents ?? 0}** game events · **${s.engine ?? 0}** engine structs`,
  ];
  if (release) lines.push('', `Dumper release: **[${release.tag}](${release.url})**${notesUrl ? ` · [notes](${notesUrl})` : ''}`);
  lines.push('', `Batch API: \`POST ${SITE}/api/query\` — try \`/api\` here.`);
  return base(`New dump — build ${index.build}`, SITE).setDescription(lines.join('\n')).setTimestamp(new Date());
}

// --- lookups -------------------------------------------------------------------
export function signatureEmbed(s, matchedAs) {
  const e = base(s.name, link('signatures', s.name));
  const meta = [`**Module** \`${s.module}\``, `**RVA** \`${hex(s.rva)}\``, `**Resolve** \`${s.resolve}\``];
  if (s.aliases?.length) meta.push(`**Also known as** ${s.aliases.map((a) => `\`${a}\``).join(', ')}`);
  if (matchedAs && matchedAs !== s.name) meta.push(`_matched via_ \`${matchedAs}\``);
  let desc = meta.join('\n') + '\n\n**Pattern (IDA)**\n' + code(s.pattern);
  if (s.prototype) desc += '**Prototype**\n' + code(clip(s.prototype, 900), 'cpp');
  if (desc.length > 4000) desc = clip(desc, 4000);
  e.setDescription(desc);
  return e;
}

export function offsetEmbed(o) {
  const dec = parseInt(o.value, 16);
  const kind = { global: 'global (a2x dwXxx)', signature: 'global resolved by signature', interface: 'registered interface instance' }[o.kind] ?? o.kind;
  const e = base(o.name, link('offsets', o.name)).addFields(
    { name: 'Module', value: `\`${o.module}\``, inline: true },
    { name: 'RVA', value: `\`${o.value}\``, inline: true },
    { name: 'Decimal', value: `\`${Number.isFinite(dec) ? dec.toLocaleString() : '—'}\``, inline: true },
    { name: 'Kind', value: kind, inline: true },
  );
  if (o.hpp_name && o.hpp_name !== o.name) e.addFields({ name: 'offsets.hpp', value: `\`offsets::${o.module.replace('.dll', '')}::${o.hpp_name}\``, inline: true });
  if (o.deref) e.addFields({ name: 'Note', value: 'The RVA holds a **pointer** to the object — read it first.', inline: false });
  return e;
}

export function classEmbed(c, field) {
  const e = base(c.name, link('classes', c.name));
  const head = [`**Module** \`${c.module}\``, c.parent ? `**Parent** \`${c.parent}\`` : null, c.size != null ? `**Size** \`${hex(c.size)}\`` : null, `**Fields** ${c.fields.length}`].filter(Boolean).join('  ·  ');
  if (field) {
    const f = c.fields.find((x) => x.name === field) ?? c.fields.find((x) => x.name.toLowerCase() === field.toLowerCase());
    if (!f) return e.setDescription(head + `\n\nNo field \`${field}\` on this class.`);
    return e.setDescription(head + '\n\n' + code(`${hex(f.offset).padEnd(8)} ${f.type}  ${f.name}`, 'cpp'));
  }
  const rows = c.fields.map((f) => `${hex(f.offset).padEnd(8)} ${clip(f.type, 34).padEnd(34)} ${f.name}`);
  let body = '', shown = 0;
  for (const r of rows) {
    if (body.length + r.length + 1 > 3300) break;
    body += (body ? '\n' : '') + r; shown++;
  }
  const more = rows.length - shown;
  e.setDescription(head + '\n\n' + code(body || '(no fields)', 'cpp') + (more > 0 ? `…and ${more} more — [full class on the site](${link('classes', c.name)})` : ''));
  return e;
}

export function convarEmbed(cv) {
  const e = base(cv.name, link('convars', cv.name));
  const isCmd = cv.type == null;
  e.addFields(
    { name: isCmd ? 'Kind' : 'Value', value: isCmd ? 'ConCommand' : `\`${cv.value ?? ''}\``, inline: true },
    ...(isCmd ? [] : [{ name: 'Type', value: `\`${cv.type}\``, inline: true }]),
    { name: 'Flags', value: cv.flag_names?.length ? cv.flag_names.map((f) => `\`${f}\``).join(' ') : '—', inline: false },
  );
  if (cv.description) e.setDescription(clip(cv.description, 1500));
  return e;
}

export function weaponEmbed(w) {
  const e = base(w.name, link('weapons', w.name));
  const f = (v, d = 3) => (typeof v === 'number' ? Number(v.toFixed(d)).toString() : String(v ?? '—'));
  e.addFields(
    { name: 'Damage', value: `${w.damage}`, inline: true },
    { name: 'Headshot ×', value: f(w.headshot_multiplier), inline: true },
    { name: 'Armor ratio', value: f(w.armor_ratio), inline: true },
    { name: 'Penetration', value: f(w.penetration), inline: true },
    { name: 'Range', value: `${f(w.range, 0)} (mod ${f(w.range_modifier)})`, inline: true },
    { name: 'Cycle time', value: `${f(w.cycle_time)} s`, inline: true },
    { name: 'Price', value: `$${w.price}`, inline: true },
    { name: 'Max speed', value: `${f(w.max_speed, 0)}`, inline: true },
    { name: 'Bullets', value: `${w.num_bullets}`, inline: true },
    { name: 'Spread', value: f(w.spread, 4), inline: true },
    { name: 'Inaccuracy stand / move', value: `${f(w.inaccuracy_stand, 4)} / ${f(w.inaccuracy_move, 4)}`, inline: true },
    { name: 'Recoil', value: f(w.recoil_magnitude), inline: true },
  );
  return e;
}

export function eventEmbed(ev) {
  const e = base(ev.name, link('gameevents', ev.name));
  const rows = (ev.fields ?? []).map((f) => `${clip(f.type, 18).padEnd(18)} ${f.name}${f.description ? `  // ${f.description}` : ''}`);
  e.setDescription(`**id** \`${ev.id}\`${ev.local ? '  ·  local' : ''}  ·  **${rows.length}** fields\n\n` + code(clip(rows.join('\n') || '(no fields)', 3500), 'cpp'));
  return e;
}

export function structEmbed(st) {
  const e = base(st.name, `${SITE}/?tab=engine`);
  const rows = (st.fields ?? []).map((f) => `${String(f.offset).padEnd(7)} ${clip(f.type, 30).padEnd(30)} ${f.name}`);
  const fns = (st.functions ?? []).map((fn) => `${fn.name} = ${fn.rva ?? 'unresolved'}`);
  let desc = clip(st.desc ?? '', 600) + '\n';
  if (st.instance_rva) desc += `\n**Instance** \`${st.instance_rva}\`  `;
  if (st.size) desc += `**Size** \`${st.size}\``;
  desc += '\n' + code(clip(rows.join('\n'), 2600), 'cpp');
  if (fns.length) desc += '**Functions**\n' + code(clip(fns.join('\n'), 600));
  if (st.header) desc += `[drop-in header](${SITE}/raw/${st.header})`;
  e.setDescription(clip(desc, 4000));
  return e;
}

export function searchEmbed(text, hits) {
  const e = base(`Search: ${clip(text, 60)}`, `${SITE}/?q=${encodeURIComponent(text)}`);
  const sections = [];
  const add = (label, items, fmt) => { if (items.length) sections.push(`**${label}**\n${items.map(fmt).join('\n')}`); };
  add('Signatures', hits.signatures, (s) => `\`${s.name}\` · ${s.module} · \`${hex(s.rva)}\``);
  add('Offsets', hits.offsets, (o) => `\`${o.name}\` · ${o.module} · \`${o.value}\``);
  add('Classes', hits.classes, (c) => `\`${c.name}\` · ${c.module} · ${c.fields.length} fields`);
  add('ConVars', hits.convars, (c) => `\`${c.name}\` = \`${clip(String(c.value ?? ''), 24)}\``);
  add('Engine structs', hits.structs, (s) => `\`${s.name}\``);
  e.setDescription(sections.length ? clip(sections.join('\n\n'), 4000) : 'Nothing matched.');
  return e;
}

export function queryEmbed(data, request) {
  const e = base('Batch query', `${SITE}/?tab=api`);
  const parts = [];
  for (const [kind, found] of Object.entries(data.found ?? {})) {
    const rows = Object.entries(found).map(([k, v]) => {
      if (v && typeof v === 'object') return `${k} = ${v.pattern ?? v.value ?? JSON.stringify(v)}`;
      return `${k} = ${v}`;
    });
    if (rows.length) parts.push(`**${kind}**\n` + code(clip(rows.join('\n'), 1200)));
  }
  const missing = Object.entries(data.missing ?? {}).flatMap(([k, v]) => v.map((n) => `${k}/${n}`));
  if (missing.length) parts.push(`**Not found:** ${missing.map((m) => `\`${m}\``).join(', ')}`);
  parts.push(`Same request over HTTP:\n` + code(`curl -X POST ${SITE}/api/query -H 'Content-Type: application/json' -d '${JSON.stringify(request)}'`, 'bash'));
  e.setDescription(clip(parts.join('\n'), 4000));
  return e;
}

export function apiEmbed() {
  const e = base('cs2-sdk.com API', `${SITE}/?tab=api`);
  e.setDescription([
    'Public, no auth, CORS enabled. Everything on the site is available as JSON.',
    '',
    '**Batch query** — several names in one call:',
    code(`curl -X POST ${SITE}/api/query \\\n  -H 'Content-Type: application/json' \\\n  -d '{"signatures":{"CreateMove":true,"TraceShape":true},"offsets":["dwEntityList"],"schemas":["C_BaseEntity.m_iHealth"]}'`, 'bash'),
    'Names match exactly, case-insensitively, without the class prefix, or by method name alone (`SetVoiceData`).',
    '',
    '**More endpoints**',
    code(`GET /api/summary           build + dataset counts\nGET /api/signatures        every signature (aliases folded)\nGET /api/offsets?detail=1  every resolved global\nGET /api/schemas?module=client_dll&class=C_BaseEntity\nGET /api/convars?q=name    GET /api/weapons   GET /api/gameevents\nGET /api/engine            GET /raw/<file>    GET /llms.txt`),
    `Full docs: ${SITE}/?tab=api`,
  ].join('\n'));
  return e;
}
