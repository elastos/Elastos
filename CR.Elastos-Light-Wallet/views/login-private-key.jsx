const React = require('react');

module.exports = (props) => {
  const app = props.app;
  const guiToggles = props.guiToggles;
  const usePrivateKey = () => {
    app.getPublicKeyFromPrivateKey();
    guiToggles.showHome();
  }
  return (
  <table id="loginPrivateKey" className="bordered w750h520px">
    <tbody>
      <tr>
        <td colSpan="2">
          <div>Private Key</div>
        </td>
      </tr>
      <tr>
        <td colSpan="2">
          <input className="monospace" type="text" size="64" id="privateKey" placeholder="Private Key"></input>
        </td>
      </tr>
      <tr>
        <td class="ta_left">
          <div className="bordered bgcolor_black_hover display_inline_block" onClick={(e)=> usePrivateKey()}>Use Private Key</div>
        </td>
        <td class="ta_right">
          <div className="bordered bgcolor_black_hover display_inline_block ta_right" onClick={(e)=> guiToggles.showLanding()}>Back</div>
        </td>
      </tr>
    </tbody>
  </table>
  );
}
