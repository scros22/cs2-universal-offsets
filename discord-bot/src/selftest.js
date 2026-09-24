// Renders every embed against the live API and checks Discord's size limits.
// Run with: npm run selftest   (no Discord login needed)

import { api, index, refreshIndex, suggest } from './api.js';
import * as E from './embeds.js';

const limits = (embed) => {
  const j = embed.toJSON();
  const total = (j.title?.length ?? 0) + (j.description?.length ?? 0) + (j.footer?.text?.length ?? 0)
    + (j.fields ?? []).reduce((n, f) => n + f.name.length + f.value.length, 0);
  const problems = [];
  if ((j.title?.length ?? 0) > 256) problems.push('title>256');
  if ((j.description?.length ?? 0) > 4096) problems.push('description>4096');
  if ((j.fields ?? []).length > 25) problems.push('fields>25');
  for (const f of j.fields ?? []) { if (f.name.length > 256) problems.push(`field name>256 (${f.name})`); if (f.value.length > 1024) problems.push(`field value>1024 (${f.name})`); if (!f.value) problems.push(`empty field (${f.name})`); }
  if (total > 6000) problems.push(`total ${total}>6000`);
  return { total, problems };
};

let failed = 0;
const check = (label, embed) => {
  const { total, problems } = limits(embed);
  console.log(`${problems.length ? 'FAIL' : ' ok '} ${label.padEnd(44)} ${String(total).padStart(5)} chars ${problems.join(', ')}`);
  if (problems.length) failed++;
};

await refreshIndex();
console.log(`index: build ${index.build}, ${index.signatures.length} sigs, ${index.offsets.length} offsets, ${index.classes.length} classes, ${index.convars.length} convars, ${index.weapons.length} weapons, ${index.gameevents.length} events, ${index.structs.length} structs`);
const release = await api.latestRelease();
check('status', E.statusEmbed(release));
check('announcement', E.announcementEmbed({ build: 14175 }, release, release?.url));
// worst cases: longest pattern, longest prototype, most aliases
const longest = [...index.signatures].sort((a, b) => b.pattern.length - a.pattern.length)[0];
check(`sig longest pattern (${longest.name})`, E.signatureEmbed(longest));
const proto = [...index.signatures].filter((s) => s.prototype).sort((a, b) => b.prototype.length - a.prototype.length)[0];
check(`sig longest prototype (${proto.name})`, E.signatureEmbed(proto));
const aliased = [...index.signatures].sort((a, b) => (b.aliases?.length ?? 0) - (a.aliases?.length ?? 0))[0];
check(`sig most aliases (${aliased.name})`, E.signatureEmbed(aliased, aliased.aliases?.[0]));
check('offset', E.offsetEmbed(index.offsets.find((o) => o.deref) ?? index.offsets[0]));
const bigClass = [...index.classes].sort((a, b) => b.fields.length - a.fields.length)[0];
check(`class most fields (${bigClass.name}, ${bigClass.fields.length})`, E.classEmbed(bigClass));
check('class field', E.classEmbed(bigClass, bigClass.fields[0].name));
const longDesc = [...index.convars].sort((a, b) => (b.description?.length ?? 0) - (a.description?.length ?? 0))[0];
check(`convar longest description (${longDesc.name})`, E.convarEmbed(longDesc));
check('command', E.convarEmbed(index.commands[0]));
check('weapon', E.weaponEmbed(index.weapons.find((w) => w.name === 'weapon_ak47') ?? index.weapons[0]));
const bigEvent = [...index.gameevents].sort((a, b) => b.fields.length - a.fields.length)[0];
check(`event most fields (${bigEvent.name})`, E.eventEmbed(bigEvent));
for (const st of index.structs) check(`struct ${st.name}`, E.structEmbed(st));
check('search', E.searchEmbed('Trace', {
  signatures: suggest(index.signatures, 'Trace', { extra: ['aliases'], limit: 8 }), offsets: suggest(index.offsets, 'Trace', { limit: 6 }),
  classes: suggest(index.classes, 'Trace', { limit: 6 }), convars: suggest(index.convars, 'trace', { limit: 6 }), structs: suggest(index.structs, 'Trace', { limit: 4 }),
}));
const qres = await api.query({ signatures: ['CreateMove', 'SetVoiceData', 'NoSuchThing'], offsets: ['dwEntityList'], schemas: ['C_BaseEntity.m_iHealth'] }, false);
check('query', E.queryEmbed(qres, { signatures: ['CreateMove', 'SetVoiceData'], offsets: ['dwEntityList'] }));
check('api', E.apiEmbed());
console.log(`autocomplete 'trace' -> ${suggest(index.signatures, 'trace', { extra: ['aliases'] }).slice(0, 5).map((s) => s.name).join(', ')}`);
console.log(failed ? `\n${failed} embed(s) over limits` : '\nall embeds within Discord limits');
process.exit(failed ? 1 : 0);
