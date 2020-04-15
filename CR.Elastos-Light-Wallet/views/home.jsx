const React = require('react');

const Menu = require('./partial/menu.jsx');

const Branding = require('./partial/branding.jsx');

const Balance = require('./partial/balance.jsx');

const News = require('./partial/news.jsx');

const Staking = require('./partial/staking.jsx');

const SocialMedia = require('./partial/social-media.jsx');

module.exports = (props) => {
  const App = props.App;
  const openDevTools = props.openDevTools;
  const Version = props.Version;
  const GuiToggles = props.GuiToggles;

  const showHomeMenu = () => {
    GuiToggles.showHomeMenu();
  }

  const changeNodeUrl = () => {
    App.changeNodeUrl();
    GuiToggles.hideHomeMenu();
  }

  const sendIsFocus = () => {
    App.setSendHasFocus(true);
  }

  const sendIsNotFocus = () => {
    App.updateAmountAndFees();
    App.setSendHasFocus(false);
  }

  const updateAmountAndFeesAndRenderApp = (e) => {
    App.updateAmountAndFees();
    App.renderApp();
  }

  const showConfirmAndSeeFees = () => {
    // App.log('STARTED showConfirmAndSeeFees')
    App.setSendStep(2);
    App.updateAmountAndFees();
    App.setSendHasFocus(false);
    App.renderApp();
  }

  const cancelSend = () => {
    App.setSendStep(1);
    App.clearSendData();
    App.setSendHasFocus(false);
    App.renderApp();
  }

  const sendAmountToAddress = () => {
    App.setSendStep(1);
    App.sendAmountToAddress();
    App.renderApp();
  }

  const SendScreen = (props) => {
    // App.log('SendScreen', App.getSendStep());
    if(App.getSendStep() == 1) {
      return (
        <div>
          <SendScreenOne visibility=""/>
          <SendScreenTwo visibility="display_none"/>
        </div>
      )
    }
    if(App.getSendStep() == 2) {
      return (
        <div>
          <SendScreenOne visibility="display_none"/>
          <SendScreenTwo visibility=""/>
        </div>
      )
    }
  }

  const SendScreenOne = (props) => {
    const visibility = props.visibility;
    return (
      <div id="sendOne" className={`bordered w250px h200px bgcolor_black_hover ${visibility}`}>
        Send
        <div>Send Amount</div>
        <br/>
        <input className="monospace" type="text" size="14" id="sendAmount" placeholder="Send Amount" defaultValue={App.getSendAmount()} onFocus={(e) => sendIsFocus(e)} onBlur={(e) => sendIsNotFocus(e)}></input>
        <div className="gray_on_white">To Address</div>
        <br/>
        <input className="monospace" type="text" size="34" id="sendToAddress" placeholder="Send To Address"  defaultValue={App.getSendToAddress()} onFocus={(e) => sendIsFocus(e)} onBlur={(e) => sendIsNotFocus(e)}></input>
        <br/>
        <div>Send Status</div>
        <br/>
        <div className="h90px w100pct overflow_auto">
        <table>
          <tbody>
            {
              App.getSendToAddressStatuses().map((sendToAddressStatus, index) => {
                return (<tr key={index}>
                <td>{sendToAddressStatus}</td>
                </tr>)
              })
            }
            {
              App.getSendToAddressLinks().map((item, index) => {
                return (<tr key={index}>
                  <td>
                    <a href={item.txDetailsUrl} onClick={(e) => onLinkClick(e)}>{item.txHash}</a>
                  </td>
                </tr>)
              })
            }
          </tbody>
        </table>
        </div>
        <span className="bordered bgcolor_black_hover" onClick={(e) => showConfirmAndSeeFees()}>Next</span>
      </div>
    );
  }

  const SendScreenTwo = (props) => {
    const visibility = props.visibility;
    return (
      <div id="sendTwo" className={`bordered w250px h200px bgcolor_black_hover ${visibility}`}>
        Send
        <div>Fees (in Satoshis)</div>
        <input className="monospace" type="text" size="14" id="feeAmount" placeholder="Fees" defaultValue={App.DEFAULT_FEE_SATS} onFocus={(e) => sendIsFocus(e)} onBlur={(e) => sendIsNotFocus(e)}></input>
        <div onClick={(e) => showConfirmAndSeeFees()}>Estimated New Balance</div>
        <p>Your balance will be deducted {App.getSendAmount()}
          ELA + {App.getFeeAmountEla()}
          ELA in fees.</p>
          <span className="bordered bgcolor_black_hover" onClick={(e) => cancelSend()}>Back</span>
          <span className="bordered bgcolor_black_hover" onClick={(e) => sendAmountToAddress()}>Confirm</span>
      </div>
    )
  }

  return (
    <table id="home" className="bordered w750h520px">
      <tbody>
        <tr>
          <td colSpan="3" className="bordered w750px h20px ta_right va_top">
            <Menu App={App} openDevTools={openDevTools} GuiToggles={GuiToggles}/>
            <div id="version" className="display_inline_block">
              <Version/>
            </div>
            <div id="homeMenuOpen" className="h25px bordered display_inline_block bgcolor_black_hover" title="menu" onClick={(e) => showHomeMenu()}>
              <img src="artwork/more-vertical.svg" />
            </div>
            <div className="h25px bordered display_inline_block bgcolor_black_hover" title="Refresh Blockchain Data"  onClick={(e) => App.refreshBlockchainData()}>
              <img src="artwork/refresh-ccw.svg" />
            </div>
            <div className="h25px bordered display_inline_block bgcolor_black_hover">
              <img src="artwork/code.svg"  title="Show Dev Tools" onClick={(e) => openDevTools()}/>
            </div>
          </td>
        </tr>
        <tr>
          <td className="bordered w250px h200px ta_center va_top">
            <Branding/>
            <Balance App={App}/>
          </td>
          <td className="bordered w200px h200px ta_center va_top font_size12">
            <SendScreen />
          </td>
          <td className="bordered w250px h200px ta_center va_top">
            <div id="receive" className="bordered w250px h90px bgcolor_black_hover">
              Receive
              <div className="ta_left">
                <div className="font_size12">Address</div>
                <span className="bgcolor_black">
                  <img src="artwork/copy.svg" />
                </span>
                <span className="font_size12">{App.getAddress()}</span>
              </div>
            </div>
            <div id="receive" className="bordered w250px h90px bgcolor_black_hover">
              <div className="ta_left">
                <div className="font_size12">Ledger</div>
                <span className="bgcolor_black">
                  <img src="artwork/smartphone.svg" />
                </span>
                <span className="font_size12">Verify Address on Ledger</span>
              </div>
            </div>
          </td>
        </tr>
        <tr>
          <td className="bordered w250px h200px ta_center va_top">
            <Staking App={App} GuiToggles={GuiToggles}/>
            <News/>
            <SocialMedia GuiToggles={GuiToggles}/>
          </td>
          <td colSpan="2" className="bordered w400px h200px ta_center va_top">
            <div id="transactions" className="bordered w500px h300px bgcolor_black_hover font_size12">
              <div >Transaction List Status</div>
              <br/> {App.getTransactionHistoryStatus()}
              <div >Blockchain Status</div>
              <br/> {App.getBlockchainStatus()}
              <br/>
              <div className="display_inline_block">Previous Transactions ({App.getParsedTransactionHistory().length}
                total)</div>
              <div className="float_right display_inline_block">&nbsp;{App.getConfirmations()}&nbsp;
                Confirmations</div>
              <div className="float_right display_inline_block">&nbsp;{App.getBlockchainState().height}&nbsp;
                Blocks</div>
              <p></p>
              <div className="h180px overflow_auto">
                <table className="w100pct no_border whitespace_nowrap font_size12">
                  <tbody>
                    <tr>
                      <td className="no_border no_padding">Nbr</td>
                      <td className="no_border no_padding">Value</td>
                      <td className="no_border no_padding">Time</td>
                      <td className="no_border no_padding">Type</td>
                      <td className="no_border no_padding">TX</td>
                    </tr>
                    {
                      App.getParsedTransactionHistory().map((item, index) => {
                        return (<tr key={index}>
                          <td className="no_border no_padding">{item.n}</td>
                          <td className="no_border no_padding">{item.value}&nbsp;ELA</td>
                          <td className="no_border no_padding">{item.time}</td>
                          <td className="no_border no_padding">{item.type}</td>
                          <td className="no_border no_padding">
                            <a className="exit_link" href={item.txDetailsUrl} onClick={(e) => onLinkClick(e)}>{item.txHashWithEllipsis}</a>
                          </td>
                        </tr>)
                      })
                    }
                  </tbody>
                </table>
              </div>
            </div>
          </td>
        </tr>
      </tbody>
    </table>
  );
}
