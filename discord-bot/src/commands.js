// Slash commands: definitions, autocomplete, and handlers.

import { SlashCommandBuilder, PermissionFlagsBits, MessageFlags } from 'discord.js';
import { api, index, suggest, refreshIndex } from './api.js';
import * as E from './embeds.js';
import { updateStatus } from './status.js';

const ac = (o) => o.setRequired(true).setAutocomplete(true);

export const definitions = [
  new SlashCommandBuilder().setName('build').setDescription('Current CS2 build and dump status.'),
  new SlashCommandBuilder().setName('sig').setDescription('Look up a signature (pattern, RVA, prototype).')
    .addStringOption((o) => ac(o.setName('name').setDescription('Function name — any alias works'))),
  new SlashCommandBuilder().setName('offset').setDescription('Look up a global offset (dwXxx, signature global or interface).')
    .addStringOption((o) => ac(o.setName('name').setDescription('Offset name'))),
  new SlashCommandBuilder().setName('schema').setDescription('Schema class layout, or one field of it.')
    .addStringOption((o) => ac(o.setName('class').setDescription('Class name')))
    .addStringOption((o) => o.setName('field').setDescription('Field name (optional)').setAutocomplete(true)),
  new SlashCommandBuilder().setName('convar').setDescription('ConVar or ConCommand: value, type, flags.')
    .addStringOption((o) => ac(o.setName('name').setDescription('ConVar / command name'))),
  new SlashCommandBuilder().setName('weapon').setDescription('Weapon stats read live from CCSWeaponBaseVData.')
    .addStringOption((o) => ac(o.setName('name').setDescription('weapon_ak47, awp, …'))),
  new SlashCommandBuilder().setName('event').setDescription('Game event and its typed fields.')
    .addStringOption((o) => ac(o.setName('name').setDescription('Event name'))),
  new SlashCommandBuilder().setName('struct').setDescription('Engine struct layout (CUserCmd, CCSGOInput, CBaseUserCmdPB, …).')
    .addStringOption((o) => ac(o.setName('name').setDescription('Struct name'))),
  new SlashCommandBuilder().setName('search').setDescription('Search signatures, offsets, classes, convars and structs at once.')
    .addStringOption((o) => o.setName('text').setDescription('Anything').setRequired(true)),
  new SlashCommandBuilder().setName('query').setDescription('Batch lookup — the same request the site API takes.')
    .addStringOption((o) => o.setName('signatures').setDescription('Comma-separated signature names'))
    .addStringOption((o) => o.setName('offsets').setDescription('Comma-separated offset names'))
    .addStringOption((o) => o.setName('schemas').setDescription('Comma-separated Class or Class.field'))
    .addStringOption((o) => o.setName('convars').setDescription('Comma-separated convar names')),
  new SlashCommandBuilder().setName('api').setDescription('How to use the cs2-sdk.com API.'),
  new SlashCommandBuilder().setName('refresh').setDescription('Re-read the dump and refresh the status embed now.')
    .setDefaultMemberPermissions(PermissionFlagsBits.ManageGuild),
].map((c) => c.setDMPermission(false).toJSON());

const choices = (items, fmt = (x) => x.name) => items.map((it) => {
  const name = String(fmt(it)).slice(0, 100);
  return { name, value: String(it.name).slice(0, 100) };
});

export async function autocomplete(interaction) {
  const cmd = interaction.commandName;
  const focused = interaction.options.getFocused(true);
  const text = focused.value ?? '';
  let out = [];
  try {
    if (cmd === 'sig') {
      out = choices(suggest(index.signatures, text, { extra: ['aliases'] }), (s) => `${s.name}  ·  ${s.module}`);
    } else if (cmd === 'offset') {
      out = choices(suggest(index.offsets, text, { extra: ['hpp_name'] }), (o) => `${o.name}  ·  ${o.module}  ·  ${o.value}`);
    } else if (cmd === 'schema') {
      if (focused.name === 'class') {
        out = choices(suggest(index.classes, text), (c) => `${c.name}  ·  ${c.module}`);
      } else {
        const cls = interaction.options.getString('class') ?? '';
        const c = index.classes.find((x) => x.name === cls) ?? index.classes.find((x) => x.name.toLowerCase() === cls.toLowerCase());
        out = c ? choices(suggest(c.fields, text), (f) => `${f.name}  ·  0x${Number(f.offset).toString(16).toUpperCase()}`) : [];
      }
    } else if (cmd === 'convar') {
      out = choices(suggest([...index.convars, ...index.commands], text), (c) => (c.type ? `${c.name} = ${String(c.value ?? '').slice(0, 30)}` : `${c.name}  (command)`));
    } else if (cmd === 'weapon') {
      out = choices(suggest(index.weapons, text));
    } else if (cmd === 'event') {
      out = choices(suggest(index.gameevents, text), (e) => `${e.name}  ·  ${e.fields?.length ?? 0} fields`);
    } else if (cmd === 'struct') {
      out = choices(suggest(index.structs, text));
    }
  } catch (err) {
    console.error('autocomplete', cmd, err);
  }
  await interaction.respond(out.slice(0, 25)).catch(() => {});
}

const ci = (a, b) => String(a).toLowerCase() === String(b).toLowerCase();

export async function handle(interaction) {
  const cmd = interaction.commandName;
  const reply = (embed, ephemeral = false) => interaction.editReply({ embeds: [embed], ...(ephemeral ? { flags: MessageFlags.Ephemeral } : {}) });
  await interaction.deferReply();

  try {
    if (cmd === 'build') {
      const release = await api.latestRelease();
      return reply(E.statusEmbed(release));
    }
    if (cmd === 'sig') {
      const name = interaction.options.getString('name', true).trim();
      const res = await api.query({ signatures: [name] }, true);
      const hit = res.found?.signatures?.[name];
      if (!hit) return reply(E.errorEmbed(`No signature matches \`${name}\`. Try \`/search ${name}\`.`));
      const full = index.signatures.find((s) => s.name === hit.name) ?? { ...hit, aliases: [] };
      return reply(E.signatureEmbed({ ...full, ...hit, aliases: full.aliases }, name));
    }
    if (cmd === 'offset') {
      const name = interaction.options.getString('name', true).trim();
      const o = index.offsets.find((x) => x.name === name) ?? index.offsets.find((x) => ci(x.name, name) || ci(x.hpp_name, name))
        ?? index.offsets.find((x) => ci(x.name, 'dw' + name));
      if (!o) return reply(E.errorEmbed(`No offset named \`${name}\`.`));
      return reply(E.offsetEmbed(o));
    }
    if (cmd === 'schema') {
      const cls = interaction.options.getString('class', true).trim();
      const field = interaction.options.getString('field')?.trim();
      const c = index.classes.find((x) => x.name === cls) ?? index.classes.find((x) => ci(x.name, cls));
      if (!c) return reply(E.errorEmbed(`No schema class named \`${cls}\`.`));
      return reply(E.classEmbed(c, field));
    }
    if (cmd === 'convar') {
      const name = interaction.options.getString('name', true).trim();
      const cv = [...index.convars, ...index.commands].find((x) => x.name === name) ?? [...index.convars, ...index.commands].find((x) => ci(x.name, name));
      if (!cv) return reply(E.errorEmbed(`No convar or command named \`${name}\`.`));
      return reply(E.convarEmbed(cv));
    }
    if (cmd === 'weapon') {
      let name = interaction.options.getString('name', true).trim().toLowerCase();
      if (!name.startsWith('weapon_')) name = 'weapon_' + name;
      const w = index.weapons.find((x) => x.name === name) ?? index.weapons.find((x) => x.name.includes(name.replace('weapon_', '')));
      if (!w) return reply(E.errorEmbed(`No weapon named \`${name}\` in this dump.`));
      return reply(E.weaponEmbed(w));
    }
    if (cmd === 'event') {
      const name = interaction.options.getString('name', true).trim();
      const ev = index.gameevents.find((x) => x.name === name) ?? index.gameevents.find((x) => ci(x.name, name));
      if (!ev) return reply(E.errorEmbed(`No game event named \`${name}\`.`));
      return reply(E.eventEmbed(ev));
    }
    if (cmd === 'struct') {
      const name = interaction.options.getString('name', true).trim();
      const st = index.structs.find((x) => x.name === name) ?? index.structs.find((x) => ci(x.name, name));
      if (!st) return reply(E.errorEmbed(`No engine struct named \`${name}\`. Available: ${index.structs.map((s) => `\`${s.name}\``).join(', ')}`));
      return reply(E.structEmbed(st));
    }
    if (cmd === 'search') {
      const text = interaction.options.getString('text', true).trim();
      const hits = {
        signatures: suggest(index.signatures, text, { extra: ['aliases'], limit: 8 }),
        offsets: suggest(index.offsets, text, { extra: ['hpp_name'], limit: 6 }),
        classes: suggest(index.classes, text, { limit: 6 }),
        convars: suggest(index.convars, text, { limit: 6 }),
        structs: suggest(index.structs, text, { limit: 4 }),
      };
      return reply(E.searchEmbed(text, hits));
    }
    if (cmd === 'query') {
      const split = (s) => (s ?? '').split(',').map((x) => x.trim()).filter(Boolean);
      const req = {};
      for (const k of ['signatures', 'offsets', 'schemas', 'convars']) {
        const v = split(interaction.options.getString(k));
        if (v.length) req[k] = v;
      }
      if (!Object.keys(req).length) return reply(E.errorEmbed('Give at least one list, e.g. `/query signatures: CreateMove,TraceShape offsets: dwEntityList`.'));
      const res = await api.query(req, false);
      return reply(E.queryEmbed(res, req));
    }
    if (cmd === 'api') return reply(E.apiEmbed());
    if (cmd === 'refresh') {
      await refreshIndex();
      await updateStatus(interaction.client, { force: true });
      return reply(E.errorEmbed('Refreshed.').setColor(E.ACCENT));
    }
    return reply(E.errorEmbed('Unknown command.'));
  } catch (err) {
    console.error(`/${cmd}`, err);
    return reply(E.errorEmbed(`Something went wrong talking to the API: ${String(err.message ?? err).slice(0, 300)}`));
  }
}
