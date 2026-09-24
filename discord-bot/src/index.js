// cs2-sdk Discord bot: live status, new-dump announcements, slash commands.

import { Client, GatewayIntentBits, REST, Routes, ActivityType, Events } from 'discord.js';
import { CFG, index, refreshIndex } from './api.js';
import { definitions, autocomplete, handle } from './commands.js';
import { updateStatus } from './status.js';

if (!CFG.token || !CFG.clientId) {
  console.error('config.json needs "token" and "clientId".');
  process.exit(1);
}

const STATUS_MINUTES = Number(CFG.statusIntervalMinutes ?? 5);
const client = new Client({ intents: [GatewayIntentBits.Guilds] });
const rest = new REST({ version: '10' }).setToken(CFG.token);

async function registerCommands(guildId) {
  await rest.put(Routes.applicationGuildCommands(CFG.clientId, guildId), { body: definitions });
  console.log(`[commands] registered ${definitions.length} commands in guild ${guildId}`);
}

function setPresence() {
  client.user?.setPresence({
    activities: [{ name: `build ${index.build ?? '…'} · cs2-sdk.com`, type: ActivityType.Watching }],
    status: 'online',
  });
}

client.once(Events.ClientReady, async () => {
  console.log(`[ready] ${client.user.tag} in ${client.guilds.cache.size} guild(s)`);
  for (const g of client.guilds.cache.values()) {
    try { await registerCommands(g.id); } catch (err) { console.error('[commands]', g.id, err.message); }
  }
  try { await refreshIndex(); } catch (err) { console.error('[index]', err.message); }
  setPresence();
  await updateStatus(client, { force: true });
  setInterval(async () => {
    await updateStatus(client, { force: true });
    setPresence();
  }, STATUS_MINUTES * 60_000);
});

client.on(Events.GuildCreate, async (g) => {
  try { await registerCommands(g.id); await updateStatus(client, { force: true }); } catch (err) { console.error('[guildCreate]', err.message); }
});

client.on(Events.InteractionCreate, async (interaction) => {
  try {
    if (interaction.isAutocomplete()) return await autocomplete(interaction);
    if (interaction.isChatInputCommand()) return await handle(interaction);
  } catch (err) {
    console.error('[interaction]', err);
  }
});

client.on(Events.Error, (err) => console.error('[client]', err));
process.on('unhandledRejection', (err) => console.error('[unhandled]', err));

client.login(CFG.token);
