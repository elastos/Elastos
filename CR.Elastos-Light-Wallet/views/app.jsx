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
const GeneratePrivateKey = require('./generate-private-key.jsx');
const GenerateMnemonic = require('./generate-mnemonic.jsx');
const Voting = require('./voting.jsx');
const QRCode = require('./qrcode.jsx');

/** modules */
const App = require('../scripts/App.js');
const GuiToggles = require('../scripts/GuiToggles.js');
const CoinGecko = require('../scripts/CoinGecko.js');

/** constants */
const onLinkClickWhiteList = [
  'https://blockchain.elastos.org',
  'https://news.elastos.org/',
  'https://twitter.com/ElastosInfo',
  'https://www.facebook.com/elastosorg/',
  'https://t.me/elastosgroup',
];


/** functions */

const Version = () => {
  return remote.app.getVersion();
}

const onLinkClick = (event) => {
  event.preventDefault();

  const url = event.currentTarget.href;
  let isInWhitelist = false;
  onLinkClickWhiteList.forEach((prefix) => {
    if(url.startsWith(prefix)) {
      isInWhitelist = true;
    }
  })
  if(!isInWhitelist) {
    alert(url);
    return;
  }

  shell.openExternal(url);
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

  constructor(props) {
    super(props);
    this.state = { visible: false };
    // this.splash = this.splash.bind(this);
    this.unsplash = this.unsplash.bind(this)
    
  }

  componentDidMount(){
    this.unsplash()
  }

  unsplash() {
    setTimeout(() => this.setState({visible: true}), 4000) 
  }
  

  render() {

    // return (<div id="app" className="display_inline_block ta_center va_top font_sans_10">
    return (<div id="app"  >

      
      {!this.state.visible && 
      <div className='splash-div h100pct w100pct'>
      <img src="artwork/logonew.svg" height="80px" width="240px" />
      <img src='artwork/iconlw.svg' height="100px" width="100px" className="rotate" /> 
      <div></div>
      </div>}

    <div style={this.state.visible ? {display: 'block'} : {display: 'none'}}>
      <Home App={App} openDevTools={openDevTools} onLinkClick={onLinkClick} GuiToggles={GuiToggles} Version={Version}/>
      <Landing App={App} openDevTools={openDevTools} GuiToggles={GuiToggles} Version={Version}/>
      <LoginMnemonic App={App} openDevTools={openDevTools} GuiToggles={GuiToggles} Version={Version}/>
      <LoginPrivateKey App={App} openDevTools={openDevTools} GuiToggles={GuiToggles} Version={Version}/>
      <GeneratePrivateKey App={App} openDevTools={openDevTools} GuiToggles={GuiToggles} Version={Version}/>
      <GenerateMnemonic App={App} openDevTools={openDevTools} GuiToggles={GuiToggles} Version={Version}/>
      <Voting App={App} openDevTools={openDevTools} onLinkClick={onLinkClick} GuiToggles={GuiToggles} Version={Version}/>
      <QRCode App={App} openDevTools={openDevTools} GuiToggles={GuiToggles} Version={Version}/>

    </div>

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
