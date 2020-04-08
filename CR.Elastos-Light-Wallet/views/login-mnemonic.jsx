const React = require('react');

module.exports = (props) => {
  const app = props.app;
  const guiToggles = props.guiToggles;
  const useMnemonic = () => {
    app.getPublicKeyFromMnemonic();
    guiToggles.showHome();
  }
  return (
  <table id="loginMnemonic" className="bordered w750h520px">
    <tbody>
      <tr>
        <td colSpan="2">
          <div>Mnemonic</div>
        </td>
      </tr>
      <tr>
        <td colSpan="2">
          <textarea className="monospace" type="text" rows="4" cols="50" id="mnemonic" placeholder="Mnemonic"></textarea>
        </td>
      </tr>
      <tr>
        <td class="ta_left">
          <div className="bordered bgcolor_black_hover display_inline_block" onClick={(e)=> useMnemonic()}>Use Mnemonic</div>
        </td>
        <td class="ta_right">
          <div className="bordered bgcolor_black_hover display_inline_block" onClick={(e)=> guiToggles.showLanding()}>Back</div>
        </td>
      </tr>
    </tbody>
  </table>
  );
}
