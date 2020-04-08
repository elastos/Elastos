const React = require('react');

module.exports = (props) => {
  const App = props.App;
  const GuiToggles = props.GuiToggles;
  const usePrivateKey = () => {
    App.getPublicKeyFromPrivateKey();
    GuiToggles.showHome();
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
        <td className="ta_left">
          <div className="bordered bgcolor_black_hover display_inline_block" onClick={(e)=> usePrivateKey()}>Use Private Key</div>
        </td>
        <td className="ta_right">
          <div className="bordered bgcolor_black_hover display_inline_block ta_right" onClick={(e)=> GuiToggles.showLanding()}>Back</div>
        </td>
      </tr>
    </tbody>
  </table>
  );
}
