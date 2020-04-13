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
  hide('homeMenu');
  hide('homeMenuOpen');
  hide('homeMenuClose');
  hide('version');
};

const showLanding = () => {
  hideEverything();
  app.clearSendData();
  app.clearGlobalData();
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
  show('homeMenuOpen');
  show('version');
};

const showHomeMenu = () => {
  show('homeMenu');
  hide('homeMenuOpen');
  show('homeMenuClose');
  hide('version');
};

const hideHomeMenu = () => {
  hide('homeMenu');
  show('homeMenuOpen');
  hide('homeMenuClose');
  show('version');
};

exports.init = init;
exports.showLanding = showLanding;
exports.showLoginMnemonic = showLoginMnemonic;
exports.showLoginPrivateKey = showLoginPrivateKey;
exports.showHome = showHome;
exports.showHomeMenu = showHomeMenu;
exports.hideHomeMenu = hideHomeMenu;
