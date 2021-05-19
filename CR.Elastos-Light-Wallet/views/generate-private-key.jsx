const React = require('react');

module.exports = (props) => {
  const App = props.App;
  const GuiToggles = props.GuiToggles;
  const regenerate = () => {
    App.generatePrivateKeyHex();
    App.renderApp();
  }
  return (
  <table id="generatePrivateKey" className="w750h520px color_white no_padding no_border">
    <tbody>
      <tr>
        <td>
          <div className="">New Private Key</div>
          <br/>
          <br/>
          {App.getGeneratedPrivateKeyHex()}
          <br/>
          <br/>
          <hr/>
          <strong>
            Reminder: Save this private key.
            <br/>
            If you lose this key, there will be no way to recover your coins.
            <br/>
            Keep a backup of it in a safe place.
            <br/>
            To use this key, copy it (you can use the convenient copy button), and use to log in to the wallet.
            <br/>
          </strong>
          <br/>
          <div className="h25px bordered display_inline_block bgcolor_black_hover" onClick={(e) => regenerate()}>Regenerate</div>
          <div className="h25px bordered display_inline_block bgcolor_black_hover" onClick={(e) => App.copyPrivateKeyToClipboard()}>Copy</div>
          <div className="h25px bordered display_inline_block bgcolor_black_hover" onClick={(e) => GuiToggles.showLanding()}>Done</div>
          <br/>
        </td>
      </tr>
    </tbody>
  </table>
  );
}
