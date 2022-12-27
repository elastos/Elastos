const React = require('react');

const QRCode = require('qrcode.react');

const Menu = require('./partial/menu.jsx');

const Banner = require('./partial/banner.jsx');

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
  const onLinkClick = props.onLinkClick;

  const showMenu = () => {
    GuiToggles.showMenu('home');
  }

  const sendIsFocus = () => {
    App.setSendHasFocus(true);
  }

  const sendIsNotFocus = () => {
    App.setSendHasFocus(false);
    // App.updateAmountAndFees();
  }

  const updateAmountAndFeesAndRenderApp = (e) => {
    App.updateAmountAndFees();
    App.renderApp();
  }

  const showConfirmAndSeeFees = () => {
    // App.log('STARTED showConfirmAndSeeFees')
    App.setSendHasFocus(false);
    const isValid = App.validateInputs();
    if (isValid) {
      App.setSendStep(2);
    }
    App.renderApp();
  }

  const cancelSend = () => {
    App.setSendStep(1);
    App.clearSendData();
    App.setSendHasFocus(false);
    App.renderApp();
  }

  const sendAmountToAddress = () => {
    const isValid = App.updateAmountAndFees();
    if (isValid) {
      App.setSendStep(1);
      App.sendAmountToAddress();
    }
    App.renderApp();
  }

  const SendScreen = (props) => {
    // App.log('SendScreen', App.getSendStep());
    if (App.getSendStep() == 1) {
      return (<div>
        <SendScreenOne visibility=""/>
        <SendScreenTwo visibility="display_none"/>
      </div>)
    }
    if (App.getSendStep() == 2) {
      return (<div>
        <SendScreenOne visibility="display_none"/>
        <SendScreenTwo visibility=""/>
      </div>)
    }
  }

  const SendScreenOne = (props) => {
    const visibility = props.visibility;
    return (<div id="sendOne" className={`send-area ${visibility}`}>
      <img src="artwork/sendicon.svg" className="send-icon"/>
      <p className="send-text">Send</p>
      <input type="text" size="34" id="sendToAddress" className="ela-address__input" placeholder="Enter ELA Address" defaultValue={App.getSendToAddress()} onFocus={(e) => sendIsFocus(e)} onBlur={(e) => sendIsNotFocus(e)}/>
      <input type="text" size="14" id="sendAmount" className="ela-send__amount" placeholder="Amount" defaultValue={App.getSendAmount()} onFocus={(e) => sendIsFocus(e)} onBlur={(e) => sendIsNotFocus(e)}/> {/* <div className="quick-elaselector">
          <button className="quick-elaselector__icon quarter">1/4</button>
          <button className="quick-elaselector__icon half">Half</button>
          <button className="quick-elaselector__icon all">All</button>
        </div> */
      }
      <p className="elatext-send">ELA</p>
      <button className="next-button" onClick={(e) => showConfirmAndSeeFees()}>
        <p>Next</p>
      </button>

      {/* <div className="h100px w100pct overflow_auto">
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
        </div> */
      }

    </div>);
  }

  const SendScreenTwo = (props) => {
    const visibility = props.visibility;
    return (
      <div id="sendTwo" className={`send-area ${visibility}`}>
        <img src="artwork/sendicon.svg" className="send-icon" title="Refresh Blockchain Data"  onClick={(e) => App.refreshBlockchainData()}/>
        <p className="send-text">Send</p>
        <div className="fees-text">Fees (in Satoshi ELA)</div>
        <input type="text" size="14" id="feeAmount" placeholder="Fees" defaultValue={App.getFee()} onFocus={(e) => sendIsFocus(e)} onBlur={(e) => sendIsNotFocus(e)}></input>
        <div className="estimate-new dark-hover cursor_def br5" onClick={(e) => showConfirmAndSeeFees()}>Recalculate</div>
        <p className="fees-balance">Your are sending <span> {App.getSendAmount()} ELA</span> + <span>{App.getFeeAmountEla()} ELA</span> in fees.</p>
          <span className="send-back dark-hover cursor_def" onClick={(e) => cancelSend()}><img src="artwork/arrow.svg" alt="" className="rotate_180 arrow-back" />Back </span>
          <button className="sendela-button" onClick={(e) => sendAmountToAddress()}>
          <p>Send ELA</p>
          </button>
      </div>
    )
  }

  return (<div id="home" className="gridback w780h520px">
    <Banner App={App} GuiToggles={GuiToggles} page="home"/>
    <Menu App={App} openDevTools={openDevTools} GuiToggles={GuiToggles} page="home"/> {/* <div id="homeMenuOpen" className="h25px bordered display_inline_block bgcolor_black_hover" title="menu" onClick={(e) => showHomeMenu()}>
       <img src="artwork/more-vertical.svg" />
     </div> */
    }
    <div id="version" className="display_inline_block hidden">
      <Version/>
    </div>
    <div className="logo-info">
      <Branding/>
      <header>
        <img src="artwork/refreshicon.svg" className="refresh-icon" title="Refresh" onClick={(e) => App.refreshBlockchainData()} />
        <nav id="homeMenuOpen" title="Menu" onClick={(e) => showMenu()}>
          <img src="artwork/nav.svg" className="nav-icon dark-hover" onClick={(e) => showMenu()}/>
        </nav>
      </header>
      <div className="pricearea">
        <Balance App={App}/>
      </div>

      <div className="stakingarea">
        <Staking App={App} GuiToggles={GuiToggles}/>
      </div>

      <div id="scroll-radio"></div>

      <div>
        <News App={App} onLinkClick={onLinkClick}/>
      </div>

    </div>

    <div className="send-area">
      <SendScreen/>

    </div>

    <div className="receive-area">
      <img src="artwork/sendicon.svg" className="rec-icon"/>
      <p className="rec-text">Receive</p>
      <p className="address-text">Address</p>
      <button className="copy-button scale-hover" onClick={(e) => App.copyAddressToClipboard()}>
        <img src="artwork/copycut.svg" className="copy-icon" height="20px" width="20px"/>
      </button>
      <p className="address-ex word-breakall">{App.getAddress()}</p>
      {/* <img id="qricon" src="artwork/qricon.svg" className="qr-icon" height="54px" width="54px" /> */}
      <button className="qr-icon btn_none br5" title="Click to enlarge" onClick={(e) => GuiToggles.showQRCode()}>
        <QRCode value={App.getAddressOrBlank()} size={78} includeMargin={true} className="scale-hover"/>
      </button>
      <p className="scanqr-text">Scan
        <strong> QR code </strong>
        to get
        <br/>ELA Address</p>
      <p className="howqr-text gradient-font">Click QR code to Enlarge</p>
      <img src="artwork/separator.svg" className="rec-separator"/>
      <p className="ledger-heading">Ledger</p>
      <img src="artwork/ledgericon.svg" alt="" className="ledger-icon scale-hover" height="36px" width="57px" title="Please verify above address on Ledger" onClick={(e) => App.verifyLedgerBanner()}/>
      <p className="verifyledger-text">Please verify above address<br/>
        <strong>on Ledger Device</strong>
      </p>
    </div>

    <div className="transaction-area">
      <p className="transactions-heading">Transactions</p>
      <p className="blockcount transactionstatus">
        <span>Status:</span>
        <span>{App.getTransactionHistoryStatus()}</span>
      </p>
      <p className="blockcount">
        <span>Blocks:</span>
        <span>{App.getBlockchainState().height}</span>
      </p>

      <div className="txtablediv scrollbar">

        <table className="txtable">
          <tbody>
            <tr className="txtable-headrow">
              <td>VALUE</td>
              <td>DATE</td>
              <td>TYPE</td>
              <td>TX</td>
            </tr>

            {
              App.getParsedTransactionHistory().map((item, index) => {
                return (<tr className="txtable-row" key={index}>
                  <td>{item.value}&nbsp;<span className="dark-font">ELA</span>
                  </td>
                  <td>{item.time}</td>
                  <td>{item.type}</td>
                  <td>
                    <a className="exit_link" href={item.txDetailsUrl} onClick={(e) => onLinkClick(e)}>{item.txHashWithEllipsis}</a>
                  </td>
                </tr>)
              })
            }

          </tbody>
        </table>

      </div>

      <div>

        <SocialMedia GuiToggles={GuiToggles} onLinkClick={onLinkClick}/>

      </div>

    </div>

  </div>)
};
