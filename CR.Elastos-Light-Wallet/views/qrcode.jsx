const React = require('react');

const QRCode = require('qrcode.react');

const Banner = require('./partial/banner.jsx');


module.exports = (props) => {
  const App = props.App;
  const GuiToggles = props.GuiToggles;
  return (<div id="qrcode">
    <div className="qrmain-div">
    <Banner App={App} GuiToggles={GuiToggles} page="qrcode"/>
      <div className="flex w100pct">
      <img className="flex1 scale-hover" src="artwork/voting-back.svg" height="38px" width="38px" onClick={(e) => GuiToggles.showHome()}/>
      <p className="address-text font_size24 margin_none color_white display_inline_block">Address</p>
      <div className="flex1"></div>
      </div>
      <p className="address-text margin_none color_white font_size20">QR Code</p>
      <QRCode value={App.getAddressOrBlank()} size={280} includeMargin={true} className="br30"/>
      <div className="qraddress-div">
        <p className="address-ex display_inline_block font_size20">{App.getAddress()}</p>
      </div>
      <div className="flex_center">
        <button className="qrcopy-button scale-hover" onClick={(e) => App.copyAddressToClipboard()}>
          <img src="artwork/copycut.svg" height="28px" width="28px" />
        </button>
        <p className="display_inline_block color_white font_size20 paddingleft_10px">Copy</p>
      </div>
    </div>
  </div>);
}
