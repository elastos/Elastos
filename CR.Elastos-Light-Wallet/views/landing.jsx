const React = require('react');

const LedgerMessage = (props) => {
  const App = props.App;
  let message = '';
  App.getMainConsole().log('LedgerMessage', App.getLedgerDeviceInfo());
  if (App.getLedgerDeviceInfo()) {
    if (App.getLedgerDeviceInfo().error) {
      message += 'Error:';
      if (App.getLedgerDeviceInfo().message) {
        message += App.getLedgerDeviceInfo().message;
      }
    } else {
      if (App.getLedgerDeviceInfo().message) {
        message += App.getLedgerDeviceInfo().message;
      }
    }
  }
  return message;
}

const UseLedgerButton = (props) => {
  const App = props.App;
  const GuiToggles = props.GuiToggles;
  const useLedger = () => {
    App.getPublicKeyFromLedger();
    GuiToggles.showHome();
  }
  if (
    App.getLedgerDeviceInfo()
    ? App.getLedgerDeviceInfo().enabled
    : false) {
    return (<span className="bordered bgcolor_black_hover" onClick={(e) => useLedger()}>Use Ledger</span>);
  } else {
    return (<span className="bordered bgcolor_black td_linethrough">Use Ledger</span>);
  }
}

module.exports = (props) => {
  const App = props.App;
  const GuiToggles = props.GuiToggles;
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
                    <div className="bordered bgcolor_black_hover w25px" title="Refresh Blockchain Data"  onClick={(e) => App.refreshBlockchainData()}>
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
            <div className="bordered bgcolor_black_hover "  onClick={(e) => GuiToggles.showGenerateNewPrivateKey()}>Generate New Private Key</div>
          </td>
          <td className="bordered w250px h20px ta_center va_top">
            <div className="bordered bgcolor_black_hover"  onClick={(e) => GuiToggles.showGenerateNewMnemonic()}>Generate New Mnemonic</div>
          </td>
          <td className="bordered w250px h20px ta_right va_top">
            Ledger Device Info:
            <br/>
            <LedgerMessage App={App}/>
          </td>
        </tr>
          <tr>
            <td className="bordered w250px h20px ta_center va_top">
              <span className="bordered bgcolor_black_hover"  onClick={(e) => GuiToggles.showLoginMnemonic()}>Login Mnemonic</span>
            </td>
            <td className="bordered w250px h20px ta_center va_top">
              <span className="bordered bgcolor_black_hover"  onClick={(e) => GuiToggles.showLoginPrivateKey()}>Login Private Key</span>
            </td>
            <td className="bordered w250px h20px ta_right va_top">
              <UseLedgerButton App={App} GuiToggles={GuiToggles} />
            </td>
          </tr>
      </tbody>
    </table>);
}
