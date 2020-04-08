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

const UseLedgerButton = (props) => {
  const app = props.app;
  const guiToggles = props.guiToggles;
  const useLedger = () => {
    app.getPublicKeyFromLedger();
    guiToggles.showHome();
  }
  if (
    app.getLedgerDeviceInfo()
    ? app.getLedgerDeviceInfo().enabled
    : false) {
    return (<span className="bordered bgcolor_black_hover" onClick={(e) => useLedger()}>Use Ledger</span>);
  } else {
    return (<span className="bordered bgcolor_black td_linethrough">Use Ledger</span>);
  }
}

module.exports = (props) => {
  const app = props.app;
  const guiToggles = props.guiToggles;
  const openDevTools = props.openDevTools;
  const Version = props.Version;
  return (<table id="landing" className="w750h520px color_white no_padding no_border">
      <tbody className="bordered bgcolor_black">
        <tr>
          <td className="bordered w250px h30px ta_center va_top">
          </td>
          <td className="bordered w250px h30px ta_center va_top">
          </td>
          <td className="bordered w250px h30px ta_right va_top">
            <table className="w100pct margin_none">
              <tbody>
                <tr>
                  <td>
                    <Version/>
                  </td>
                  <td>
                    <div className="bordered bgcolor_black_hover w25px" title="Refresh Blockchain Data"  onClick={(e) => app.refreshBlockchainData()}>
                      <img src="artwork/refresh-ccw.svg" />
                    </div>
                  </td>
                  <td>
                    <div className="bordered bgcolor_black_hover w25px">
                      <img src="artwork/code.svg"  title="Show Dev Tools" onClick={(e) => openDevTools()}/>
                    </div>
                  </td>
                </tr>
              </tbody>
            </table>
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
              <UseLedgerButton app={app} />
            </td>
          </tr>
      </tbody>
    </table>);
}
