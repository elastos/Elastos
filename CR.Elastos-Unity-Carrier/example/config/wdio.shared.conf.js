exports.config = {
  // ====================
  // Runner and framework
  // Configuration
  // ====================
  runner: 'local',
  framework: 'jasmine',
  jasmineNodeOpts: {
      // Updated the timeout to 30 seconds due to possible longer appium calls
      // When using XPATH
      defaultTimeoutInterval: 30000,
  },
  sync: true,
  logPath : "./",
  logLevel: 'silent',
  deprecationWarnings: true,
  bail: 0,
  baseUrl: '',
  waitforTimeout: 5000,
  connectionRetryTimeout: 50000,
  connectionRetryCount: 3,
  reporters: [ 'spec' ],

  // ====================
  // Appium Configuration
  // ====================
  services: [ 'appium' ],
  appium: {
      // For options see
      // https://github.com/webdriverio/webdriverio/tree/master/packages/wdio-appium-service
      args: {
          // For arguments see
          // https://github.com/webdriverio/webdriverio/tree/master/packages/wdio-appium-service
      },
  },

  port: 4723,

  // ====================
  // Some hooks
  // ====================
 
  onPrepare: function (config, capabilities) {
    },
    /**
     * Gets executed just before initialising the webdriver session and test framework. It allows you
     * to manipulate configurations depending on the capability or spec.
     * @param {Object} config wdio configuration object
     * @param {Array.<Object>} capabilities list of capabilities details
     * @param {Array.<String>} specs List of spec file paths that are to be run
     */
    beforeSession: function (config, capabilities, specs) {
        require('@babel/register');
    },
    /**
     * Gets executed before test execution begins. At this point you can access to all global
     * variables like `browser`. It is the perfect place to define custom commands.
     * @param {Array.<Object>} capabilities list of capabilities details
     * @param {Array.<String>} specs List of spec file paths that are to be run
     */
    before: function (capabilities, specs) {
    },
    /**
     * Hook that gets executed before the suite starts
     * @param {Object} suite suite details
     */
    beforeSuite: function (suite) {
    },
    /**
     * Hook that gets executed _before_ a hook within the suite starts (e.g. runs before calling
     * beforeEach in Mocha)
     */
    beforeHook: function () {
    },
    /**
     * Hook that gets executed _after_ a hook within the suite ends (e.g. runs after calling
     * afterEach in Mocha)
     */
    afterHook: function () {
    },
    /**
     * Function to be executed before a test (in Mocha/Jasmine) or a step (in Cucumber) starts.
     * @param {Object} test test details
     */
    beforeTest: function (test) {
    },
    //
    /**
     * Runs before a WebdriverIO command gets executed.
     * @param {String} commandName hook command name
     * @param {Array} args arguments that command would receive
     */
    beforeCommand: function (commandName, args) {
    },
    /**
     * Runs after a WebdriverIO command gets executed
     * @param {String} commandName hook command name
     * @param {Array} args arguments that command would receive
     * @param {Number} result 0 - command success, 1 - command error
     * @param {Object} error error object if any
     */
    afterCommand: function (commandName, args, result, error) {
    },
    /**
     * Function to be executed after a test (in Mocha/Jasmine) or a step (in Cucumber) ends.
     * @param {Object} test test details
     */
    afterTest: function (test) {
    },
    /**
     * Hook that gets executed after the suite has ended
     * @param {Object} suite suite details
     */
    afterSuite: function (suite) {
    },
    /**
     * Gets executed after all tests are done. You still have access to all global variables from
     * the test.
     * @param {Number} result 0 - test pass, 1 - test fail
     * @param {Array.<Object>} capabilities list of capabilities details
     * @param {Array.<String>} specs List of spec file paths that ran
     */
    after: function (result, capabilities, specs) {
    },
    /**
     * Gets executed right after terminating the webdriver session.
     * @param {Object} config wdio configuration object
     * @param {Array.<Object>} capabilities list of capabilities details
     * @param {Array.<String>} specs List of spec file paths that ran
     */
    afterSession: function (config, capabilities, specs) {
    },
    /**
     * Gets executed after all workers got shut down and the process is about to exit.
     * @param {Object} exitCode 0 - success, 1 - fail
     * @param {Object} config wdio configuration object
     * @param {Array.<Object>} capabilities list of capabilities details
     */
    onComplete: function (exitCode, config, capabilities) {
    },
    //
    // Cucumber specific hooks
    beforeFeature: function (feature) {
    },
    beforeScenario: function (scenario) {
    },
    beforeStep: function (step) {
    },
    afterStep: function (stepResult) {
    },
    afterScenario: function (scenario) {
    },
    afterFeature: function (feature) {
    }
};