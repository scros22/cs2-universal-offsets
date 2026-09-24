// pm2 process file. Run `pm2 start ecosystem.config.cjs` from this directory.
module.exports = {
  apps: [
    {
      name: 'cs2-sdk-bot',
      script: 'src/index.js',
      interpreter: 'node',
      autorestart: true,
      max_restarts: 20,
      restart_delay: 5000,
      max_memory_restart: '200M',
      env: { NODE_ENV: 'production' },
      time: true,
    },
  ],
};
