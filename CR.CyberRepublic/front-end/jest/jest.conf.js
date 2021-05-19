const path = require('path');

module.exports = {
    "rootDir": path.resolve(__dirname, '../'),
    "setupFiles": [
        "<rootDir>/jest/setup.js"
    ],
    "collectCoverage": true,
    "collectCoverageFrom": [
        "src/**/*.{js,jsx,ts,tsx}",
        "!src/*.js",
        "!src/{I18N|data|model|module|config}/**/*.js",
        "!src/store/*.js"
    ],
    "testMatch": [
        "**/?(*.)(spec|test).js?(x)"
    ],
    "moduleNameMapper": {
        "^@/(.*)$": "<rootDir>/src/$1"
    }
};