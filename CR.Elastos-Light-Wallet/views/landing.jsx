const React = require('react');

const LedgerMessage = (props) => {
  const App = props.App;
  let message = '';
  // App.getMainConsole().log('LedgerMessage', App.getLedgerDeviceInfo());
  if (App.getLedgerDeviceInfo()) {
    if (App.getLedgerDeviceInfo().error) {
      message += 'Error:';
      if (App.getLedgerDeviceInfo().message) {
        message += App.getLedgerDeviceInfo().message;
      }
    } else {
      if (App.getLedgerDeviceInfo().message) {
        message += App.getLedgerDeviceInfo().message;
      }
    }
  }
  return message;
}

const UseLedgerButton = (props) => {
  const App = props.App;
  const GuiToggles = props.GuiToggles;
  const useLedger = () => {
    App.getPublicKeyFromLedger();
    if (!App.getLedgerDeviceInfo().error) {
      GuiToggles.showHome();
    }
  }
  if (
    App.getLedgerDeviceInfo()
    ? App.getLedgerDeviceInfo().enabled
    : false) {
    return (<img src="artwork/ledgerconnected.svg" width="235px" height="198px" className="ledgercon ledgeranimation dark-hover" onClick={(e) => useLedger()}></img>);
  } else {
    return (<img src="artwork/ledgernotconnected.svg" width="140px" height="36px" title="Not Connected" className="ledgernotcon dark-hover"></img>);
  }
}

module.exports = (props) => {
  const App = props.App;
  const GuiToggles = props.GuiToggles;
  const openDevTools = props.openDevTools;
  const Version = props.Version;
  return (
  
    <div id="landing">
    <img src="artwork/refreshicon.svg" className="refresh-icon" title="Refresh" onClick={(e) => App.refreshBlockchainData()}/>

    <div className="login-div ">
      <img src="artwork/logonew.svg" height="80px" width="240px" className="flexgrow_pt35"/>

      <p className="address-text font_size24 margin_none display_inline_block gradient-font">Create New Wallet</p>
      <div className="flex_center">
        <button className="home-btn scale-hover landing-btnbg" onClick={(e) => GuiToggles.showGenerateNewMnemonic()}>
          Create
        </button>
      </div>
      <p className="address-text font_size24 margin_none display_inline_block gradient-font">Import Wallet</p>
      <div className="flex_center">
        <button className="home-btn scale-hover landing-btnbg" onClick={(e) => GuiToggles.showLoginMnemonic()}>
          Login with Mnemonics
        </button>
      </div>
      <div className="flex_center">
        <button className="home-btn scale-hover landing-btnbg" onClick={(e) => GuiToggles.showLoginPrivateKey()}>
          Login with Private Key
        </button>
      </div>
      <p className="address-text font_size24 margin_none display_inline_block gradient-font">Ledger</p>
      <p className="color_white font_size16 w80pct word-breakword">Ledger Status:
        <LedgerMessage App={App}/></p>
    </div>
    <div>
      <UseLedgerButton App={App} GuiToggles={GuiToggles}/>
    </div>

  </div>);
}
