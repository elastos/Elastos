const React = require('react');

module.exports = (props) => {
  const App = props.App;
  const GuiToggles = props.GuiToggles;
  const regenerate = () => {
    App.generateMnemonic();
    App.renderApp();
  }
  return (
    <div id="generateMnemonic">
    <div className="login-div ">
      <div className="flex_center w100pct">
      <img className="flex1 scale-hover" src="artwork/voting-back.svg" height="38px" width="38px" onClick={(e)=> GuiToggles.showLanding()}/>
      <img src="artwork/logonew.svg" height="80px" width="240px" />
      <div className="flex1"></div>
      </div>
      <p className="address-text font_size24 margin_none display_inline_block gradient-font">Create New Wallet (Mnemonics)</p>
      <div className="qraddress-div">
        <p className="address-ex display_inline_block font_size20 padding_5px">{App.getGeneratedMnemonic()}</p>
      </div>
      <div className="flex_center">
      <button className="proceed-btn scale-hover" onClick={(e) => GuiToggles.showLanding()}>
            <p>Done</p>
            </button>
    </div>
    
    <div>
    <p className="gradient-font font_size20 ta_center list-none" >Tips</p>
    <ul className="color_white ta_left">
      <li className="color_red font_size16">On a piece of paper, write down this mnemonic phrase in the exact same order it appears.</li>
      <li className="color_red font_size16">If you lose this mnemonic phrase, there will be no way to recover your coins.</li>
      <li>Make sure to keep a backup of the mnemonic it in a safe place that only you can access.</li>
      <li>Once you have written it down correctly, login to your wallet using these mnemonics.</li>
    </ul>
    </div>
    </div>
  </div>);
  }