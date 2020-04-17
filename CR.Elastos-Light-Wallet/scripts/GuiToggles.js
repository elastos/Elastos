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
  hide('votingMenu');
  hide('votingMenuOpen');
  hide('votingMenuClose');
  hide('version');
  hide('voting');
  hide('qrcode');
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

const showMenu = (name) => {
  show(name+'Menu');
  hide(name+'MenuOpen');
  show(name+'MenuClose');
  hide('version');
};

const hideMenu = (name) => {
  hide(name+'Menu');
  show(name+'MenuOpen');
  hide(name+'MenuClose');
  show('version');
};

const showVoting = () => {
  hideEverything();
  app.clearSendData();
  show('voting');
  show('votingMenuOpen');
};

const showQRCode = () => {
  hideEverything();
  app.clearSendData();
  show('qrcode');
};

exports.init = init;
exports.showLanding = showLanding;
exports.showLoginMnemonic = showLoginMnemonic;
exports.showLoginPrivateKey = showLoginPrivateKey;
exports.showHome = showHome;
exports.showMenu = showMenu;
exports.hideMenu = hideMenu;
exports.showVoting = showVoting;
exports.showQRCode = showQRCode;
