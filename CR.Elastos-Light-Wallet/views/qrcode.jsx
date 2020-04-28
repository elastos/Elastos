const React = require('react');

const QRCode = require('qrcode.react');

module.exports = (props) => {
  const App = props.App;
  const GuiToggles = props.GuiToggles;
  return (<div id="qrcode">
    <div className="qrmain-div">
      <div className="flex w100pct">
      <img className="flex1" src="artwork/voting-back.svg" onClick={(e) => GuiToggles.showHome()}/>
      <p className="address-text font_size24 margin_none color_white display_inline_block">Address</p>
      <div className="flex1"></div>
      </div>
      <p className="address-text margin_none color_white font_size14">QR Code</p>
      <QRCode value={App.getAddressOrBlank()} size={200} includeMargin={true} className="br30"/>
      <div className="qraddress-div">
        <p className="address-ex display_inline_block font_size14">{App.getAddress()}</p>
      </div>
      <div className="flex_center">
        <button className="btn_none" onClick={(e) => App.copyAddressToClipboard()}>
          <img src="artwork/copy36.svg" height="36px" width="36px" />
        </button>
        <p className="display_inline_block color_white font_size14 paddingleft_10px">Copy</p>
      </div>
    </div>
  </div>);
}
