'use strict';

/* modules */
const GuiUtils = require('./GuiUtils.js');

let app;

const init = (_app) => {
  app = _app;
};

const hide = (id) => {
  GuiUtils.get(id).style = 'display:none;';
};

const show = (id) => {
  GuiUtils.get(id).style = 'display:default;';
};

const hideEverything = () => {
  hide('home');
  hide('landing');
  hide('loginMnemonic');
  hide('loginPrivateKey');
  hide('temp');
};

const showLanding = () => {
  hideEverything();
  app.clearSendData();
  app.refreshBlockchainData();
  show('landing');
};

const showLoginMnemonic = () => {
  hideEverything();
  app.clearSendData();
  show('loginMnemonic');
};

const showLoginPrivateKey = () => {
  hideEverything();
  app.clearSendData();
  show('loginPrivateKey');
};

const showHome = () => {
  hideEverything();
  app.clearSendData();
  show('home');
};

exports.init = init;
exports.showLanding = showLanding;
exports.showLoginMnemonic = showLoginMnemonic;
exports.showLoginPrivateKey = showLoginPrivateKey;
exports.showHome = showHome;
