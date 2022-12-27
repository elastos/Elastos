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
        deviceName: 'iPhone XR',
        platformName: 'iOS',
        platformVersion: '12.1',
        orientation: 'PORTRAIT',
        maxInstances: 1,
        // The path to the app
        app: join(process.cwd(), './apps/example.app'),
        
        noReset: true,
        newCommandTimeout: 50,
    },
];

exports.config = config;