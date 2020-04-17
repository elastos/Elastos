const React = require('react');

const QRCode = require('qrcode.react');

module.exports = (props) => {
  const App = props.App;
  const GuiToggles = props.GuiToggles;
  return (<div id="qrcode">
    <p className="address-text">Address</p>
    <button className="copy-button">
      <img src="artwork/copyicon.svg" className="copy-icon" height="26px" width="26px" />
    </button>
    <p className="address-ex">{App.getAddress()}</p>
    <img src="artwork/voting-back.svg" onClick={(e) => GuiToggles.showHome()}/>
    <p className="address-text">QR Code</p>
    <QRCode value={App.getAddressOrBlank()} size={300}/>
  </div>);
}
