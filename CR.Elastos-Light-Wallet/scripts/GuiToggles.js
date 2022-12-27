'use strict';

/* modules */
const GuiUtils = require('./GuiUtils.js');

let app;


const init = (_app) => {
  app = _app;
};

const hide = (id) => {
  GuiUtils.hide(id);
};

const show = (id) => {
  GuiUtils.show(id);
};

const hideEverything = () => {
  hide('home');
  hide('landing');
  hide('loginMnemonic');
  hide('loginPrivateKey');
  hide('version');
  hide('voting');
  hide('qrcode');
  hide('generateMnemonic');
  hide('generatePrivateKey');
  hideAllBanners();
  hideAllMenus();
};

const hideAllMenus = () => {
  GuiUtils.hide('loginPrivateKeyBanner');
  GuiUtils.hide('loginMnemonicBanner');
  const menus = ['home', 'voting', 'loginPrivateKey', 'loginMnemonic'];
  menus.forEach((menu) => {
    hide(menu+'Menu');
    // hide(menu+'MenuOpen');
    hide(menu+'MenuClose');
  });
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
  app.setRefreshCandiatesFlag(true);
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
  app.setRefreshCandiatesFlag(false);
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

const showBanner = (name) => {
  show(name+'Banner');
};

const hideBanner = (name) => {
  hide(name+'Banner');
};

const showGenerateNewPrivateKey = () => {
  hideEverything();
  app.clearGlobalData();
  app.generatePrivateKeyHex();
  show('generatePrivateKey');
};

const showGenerateNewMnemonic = () => {
  hideEverything();
  app.clearGlobalData();
  app.generateMnemonic();
  show('generateMnemonic');
};

const showAllBanners = () => {
  GuiUtils.show('homeBanner');
  GuiUtils.show('votingBanner');
  GuiUtils.show('loginPrivateKeyBanner');
  GuiUtils.show('loginMnemonicBanner');
  GuiUtils.show('qrcodeBanner');
};

const hideAllBanners = () => {
  GuiUtils.hide('homeBanner');
  GuiUtils.hide('votingBanner');
  GuiUtils.hide('loginPrivateKeyBanner');
  GuiUtils.hide('loginMnemonicBanner');
  GuiUtils.hide('qrcodeBanner');
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
exports.showBanner = showBanner;
exports.hideBanner = hideBanner;
exports.showGenerateNewPrivateKey = showGenerateNewPrivateKey;
exports.showGenerateNewMnemonic = showGenerateNewMnemonic;
exports.showAllBanners = showAllBanners;
exports.hideAllBanners = hideAllBanners;
