const { join } = require('path');
const { config } = require('./wdio.shared.conf');

// ============
// Specs
// ============
config.specs = [
  './test/specs/*.js',
];

// ============
// Capabilities
// ============
// For all capabilities please check
// http://appium.io/docs/en/writing-running-appium/caps/#general-capabilities
config.capabilities = [
    {
        // The defaults you need to have in your config
        automationName: 'UiAutomator2',
        deviceName: 'Genymotion Samsung',
        platformName: 'Android',
        platformVersion: '8.0',
        orientation: 'PORTRAIT',
        maxInstances: 1,
        app: join(process.cwd(), './apps/app-debug.apk'),
        // Read the reset strategies very well, they differ per platform, see
        // http://appium.io/docs/en/writing-running-appium/other/reset-strategies/
        noReset: true,
        newCommandTimeout: 240,
    },
];

exports.config = config;