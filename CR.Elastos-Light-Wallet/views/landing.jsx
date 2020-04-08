const React = require('react');

const LedgerMessage = (props) => {
  const app = props.app;
  let message = '';
  app.getMainConsole().log('LedgerMessage', app.getLedgerDeviceInfo());
  if (app.getLedgerDeviceInfo()) {
    if (app.getLedgerDeviceInfo().error) {
      message += 'Error:';
      if (app.getLedgerDeviceInfo().message) {
        message += app.getLedgerDeviceInfo().message;
      }
    } else {
      if (app.getLedgerDeviceInfo().message) {
        message += app.getLedgerDeviceInfo().message;
      }
    }
  }
  return message;
}

module.exports = (props) => {
  const app = props.app;
  const guiToggles = props.guiToggles;
  const openDevTools = props.openDevTools;
  const useLedger = () => {
    app.getPublicKeyFromLedger();
    guiToggles.showHome();
  }
return (
<table id="landing" className="bordered w750h520px color_white">
  <tbody>
    <tr>
      <td className="bordered w250px h30px ta_center va_top">
      </td>
      <td className="bordered w250px h30px ta_center va_top">
      </td>
      <td className="bordered w250px h30px ta_right va_top">
        <div className="bordered bgcolor_black_hover display_inline_block" title="Refresh Blockchain Data"  onClick={(e) => app.refreshBlockchainData()}>
          <img src="artwork/refresh-ccw.svg" />
        </div>
        <div className="bordered bgcolor_black_hover display_inline_block">
          <img src="artwork/code.svg"  title="Show Dev Tools" onClick={(e) => openDevTools()}/>
        </div>
      </td>
    </tr>
    <tr>
      <td className="bordered w250px h20px ta_center va_top">
        <div className="bordered bgcolor_black_hover "  onClick={(e) => guiToggles.showGenerateNewPrivateKey()}>Generate New Private Key</div>
      </td>
      <td className="bordered w250px h20px ta_center va_top">
        <div className="bordered bgcolor_black_hover"  onClick={(e) => guiToggles.showGenerateNewMnemonic()}>Generate New Mnemonic</div>
      </td>
      <td className="bordered w250px h20px ta_right va_top">
        Ledger Device Info:
        <br/>
        <LedgerMessage app={app}/>
      </td>
    </tr>
      <tr>
        <td className="bordered w250px h20px ta_center va_top">
          <span className="bordered bgcolor_black_hover"  onClick={(e) => guiToggles.showLoginMnemonic()}>Login Mnemonic</span>
        </td>
        <td className="bordered w250px h20px ta_center va_top">
          <span className="bordered bgcolor_black_hover"  onClick={(e) => guiToggles.showLoginPrivateKey()}>Login Private Key</span>
        </td>
        <td className="bordered w250px h20px ta_right va_top">
          <span className="bordered bgcolor_black_hover"  onClick={(e) => guiToggles.useLedger()}>
          </span>
        </td>
      </tr>
  </tbody>
</table>
);
}
