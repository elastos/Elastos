"use strict";

/** imports */
const React = require('react');
const ReactDOM = require('react-dom');

const electron = require('electron');
const shell = electron.shell;
const remote = electron.remote;
const clipboard = electron.clipboard;

/** views */
const Home = require('./home.jsx');
const Landing = require('./landing.jsx');
const LoginMnemonic = require('./login-mnemonic.jsx');
const LoginPrivateKey = require('./login-private-key.jsx');

/** modules */
const App = require('../scripts/App.js');
const GuiToggles = require('../scripts/GuiToggles.js');
const CoinGecko = require('../scripts/CoinGecko.js');

/** functions */

const Version = () => {
  return remote.app.getVersion();
}

const onLinkClick = (event) => {
  event.preventDefault();
  shell.openExternal(event.currentTarget.href);
}

const openDevTools = () => {
  try {
    const window = remote.getCurrentWindow();
    window.webContents.openDevTools();
  } catch (e) {
    alert(`error:${e}`)
  }
}

class AppView extends React.Component {
  render() {
    return (<div id="app" className="display_inline_block ta_center va_top font_sans_10">
      <Home App={App} openDevTools={openDevTools} GuiToggles={GuiToggles} Version={Version}/>
      <Landing App={App} openDevTools={openDevTools} GuiToggles={GuiToggles} Version={Version}/>
      <LoginMnemonic App={App} openDevTools={openDevTools} GuiToggles={GuiToggles} Version={Version}/>
      <LoginPrivateKey App={App} openDevTools={openDevTools} GuiToggles={GuiToggles} Version={Version}/>
    </div>)
  }
}

const renderApp = () => {
  ReactDOM.render(<AppView/>, document.getElementById('main'));
};

const onLoad = () => {
  App.init(GuiToggles);
  GuiToggles.init(App);
  CoinGecko.init(App);
  App.setAppClipboard(clipboard);
  App.setAppDocument(document);
  App.setRenderApp(renderApp);
  renderApp();
  GuiToggles.showLanding();
  App.setPollForAllInfoTimer();
}

/** call initialization functions */
window.onload = onLoad;


renderApp();
