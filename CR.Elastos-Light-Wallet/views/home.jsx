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
      <div  id="sendOne" className={`send-area ${visibility}`} > 
        <img src="artwork/sendicon.svg" class="send-icon" />
        <p class="send-text">Send</p>
        <input type="text" size="34" id="sendToAddress" className="ela-address__input" placeholder="Enter ELA Address" defaultValue={App.getSendToAddress()} onFocus={(e) => sendIsFocus(e)} onBlur={(e) => sendIsNotFocus(e)}/>
        <input type="text" size="14" id="sendAmount" className="ela-send__amount" placeholder="Amount" defaultValue={App.getSendAmount()} onFocus={(e) => sendIsFocus(e)} onBlur={(e) => sendIsNotFocus(e)}/>
        <div class="quick-elaselector">
          <button class="quick-elaselector__icon quarter">1/4</button>
          <button class="quick-elaselector__icon half">Half</button>
          <button class="quick-elaselector__icon all">All</button>
        </div>
        <p class="elatext-send">ELA</p>
        <button class="next-button" onClick={(e) => showConfirmAndSeeFees()}>
        <p>Next</p>
        </button>

        <div className="h100px w100pct overflow_auto">
      {/* <div id="sendOne" className={`bordered w250px h200px bgcolor_black_hover ${visibility}`}>
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
        <div className="h90px w100pct overflow_auto"> */}
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

      </div>
        );
      }
  

  const SendScreenTwo = (props) => {
    const visibility = props.visibility;
    return (
      <div id="sendTwo" className={`send-area ${visibility}`}>
        <img src="artwork/sendicon.svg" class="send-icon" title="Refresh Blockchain Data"  onClick={(e) => App.refreshBlockchainData()}/>
        <p class="send-text">Send</p>
        <div class="fees-text">Fees (in Satoshis)</div>
        <input type="text" size="14" id="feeAmount" placeholder="Fees" defaultValue={App.DEFAULT_FEE_SATS} onFocus={(e) => sendIsFocus(e)} onBlur={(e) => sendIsNotFocus(e)}></input>
        <div class="estimate-new"onClick={(e) => showConfirmAndSeeFees()}>Estimated New Balance</div>
        <p class="fees-balance">Your balance will be deducted <span> {App.getSendAmount()} ELA</span>
          + <br /> <span>{App.getFeeAmountEla()}</span> 
           ELA in fees.</p>
          <span className="send-back" onClick={(e) => cancelSend()}> Back </span>
          <button className="sendela-button" onClick={(e) => sendAmountToAddress()}>
          <p>Send ELA</p>
          </button>
      </div>
    )
  }

  return (
    <div id="home" class="gridback w780h520px">
     <Menu App={App} openDevTools={openDevTools} GuiToggles={GuiToggles}/>
     {/* <div id="homeMenuOpen" className="h25px bordered display_inline_block bgcolor_black_hover" title="menu" onClick={(e) => showHomeMenu()}>
       <img src="artwork/more-vertical.svg" />
     </div> */}
        <div id="version" className="display_inline_block hidden">
           <Version/>
       </div>
    <div class="logo-info">
      <Branding/>
      <header>
        <img src="artwork/system.svg" class="system-icon" />
        <img src="artwork/refreshicon.svg" class="refresh-icon" />
        <nav id="homeMenuOpen" title="menu" onClick={(e) => showHomeMenu()}>
          <img src="artwork/nav.svg" class="nav-icon" onClick={(e) => showHomeMenu()}/>
        </nav>
      </header>
      <div class="pricearea">
       <Balance App={App}/>
      </div> 
    
      <div class="stakingarea">
       <Staking App={App} GuiToggles={GuiToggles}/>
      </div>
    
      
      <div id="scroll-radio">
    
      </div>
    
      <div>
        <News/>
      </div>

      <div class="send-area send-bg"></div>
    
      <div class="send-area"> 
                  <SendScreen />

      </div>
    
    </div>
    <div class="receive-area">
        <img src="artwork/sendicon.svg" class="rec-icon" />
        <p class="rec-text">Receive</p>
        <p class="address-text">Address</p>
        <button class="copy-button">
          <img src="artwork/copyicon.svg" class="copy-icon" height="26px" width="26px" />
        </button>
        <p class="address-ex">{App.getAddress()}</p>
        <img src="artwork/qricon.svg" class="qr-icon" height="54px" width="54px" />
        <p class="scanqr-text">Scan <strong>QR code</strong> to get <br />ELA Address</p>
        <p class="howqr-text gradient-font">How QR works?</p>
        <img src="artwork/separator.svg" class="rec-separator" />
        <p class="ledger-heading">Ledger</p>
        <img src="artwork/ledgericon.svg" alt="" class="ledger-icon" height="24px" width="38px" />
        <p class="verifyledger-text">Verify address on <br /><strong>ledger</strong></p>

</div>
  
<div class="transaction-area">
  <p class="transactions-heading">Transactions</p>
  <p class="blockcount"><span>Blocks:</span><span>{App.getBlockchainState().height}</span> </p>

    <div class="txtablediv">
  
  <table class="txtable">
    <tr class="txtable-headrow">
      <td>VALUE</td>
      <td>DATE</td>
      <td>TYPE</td>
      <td>TX</td>
    </tr>

    {
      App.getParsedTransactionHistory().map((item, index) => {
        return (<tr className="txtable-row" key={index}>
        <td>{item.value}&nbsp;<span class="dark-font">ELA</span></td>
        <td>{item.time}</td>
        <td>{item.type}</td>
        <td>
            <a className="exit_link" href={item.txDetailsUrl} onClick={(e) => onLinkClick(e)}>{item.txHashWithEllipsis}</a>
        </td>
        </tr>)
      })
      }

    {/* <tr class="txtable-row">
      <td>250 <span class="dark-font">ELA</span></td>
      <td>2020-02-17 <span class="dark-font">10:50</span></td>
      <td>Received</td>
      <td>5bfa9573d7bc89472a4b8ec5f1da0ed0947…</td>
    </tr> */}

    {/* <tr class="txtable-row">
      <td>100 <span class="dark-font">ELA</span></td>
      <td>2020-02-12 <span class="dark-font">15:40</span></td>
      <td>Sent</td>
      <td>de02a581c2af72bee1ca</td>
    </tr>

    <tr class="txtable-row">
      <td>1000 <span class="dark-font">ELA</span></td>
      <td>2020-02-10 <span class="dark-font"> 20:40</span></td>
      <td>Received</td>
      <td>5bfa9573d7bc89472a4b</td>
    </tr> */}
  </table>

  </div>

  <div>
    
  <SocialMedia GuiToggles={GuiToggles}/>

  </div>



</div>

</div>
    {/* <table id="home" className="bordered w750h520px">
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
    </table> */}
  );
};


  //     <div id="sendOne" className={`bordered w250px h200px bgcolor_black_hover ${visibility}`}>
  //       Send
  //       <div>Send Amount</div>
  //       <br/>
  //       <input className="monospace" type="text" size="14" id="sendAmount" placeholder="Send Amount" defaultValue={App.getSendAmount()} onFocus={(e) => sendIsFocus(e)} onBlur={(e) => sendIsNotFocus(e)}></input>
  //       <div className="gray_on_white">To Address</div>
  //       <br/>
  //       <input className="monospace" type="text" size="34" id="sendToAddress" placeholder="Send To Address"  defaultValue={App.getSendToAddress()} onFocus={(e) => sendIsFocus(e)} onBlur={(e) => sendIsNotFocus(e)}></input>
  //       <br/>
  //       <div>Send Status</div>
  //       <br/>
  //       <div className="h100px w100pct overflow_auto">
  //       <table>
  //         <tbody>
  //           {
  //             App.getSendToAddressStatuses().map((sendToAddressStatus, index) => {
  //               return (<tr key={index}>
  //               <td>{sendToAddressStatus}</td>
  //               </tr>)
  //             })
  //           }
  //           {
  //             App.getSendToAddressLinks().map((item, index) => {
  //               return (<tr key={index}>
  //                 <td>
  //                   <a href={item.txDetailsUrl} onClick={(e) => onLinkClick(e)}>{item.txHash}</a>
  //                 </td>
  //               </tr>)
  //             })
  //           }
  //         </tbody>
  //       </table>
  //       </div>
  //       <span className="bordered bgcolor_black_hover" onClick={(e) => showConfirmAndSeeFees()}>Next</span>
  //     </div>
  //   );
  // }


//     <table id="home" className="gridback w780h520px">
//       <tbody>
//         <tr>
//           <td colSpan="3" className="bordered w750px h20px ta_right va_top">
//             <Menu App={App} openDevTools={openDevTools} GuiToggles={GuiToggles}/>
//             <div id="version" className="display_inline_block">
//               <Version/>
//             </div>
//             <div id="homeMenuOpen" className="h25px bordered display_inline_block bgcolor_black_hover" title="menu" onClick={(e) => showHomeMenu()}>
//               <img src="artwork/more-vertical.svg" />
//             </div>

//             <div className="h25px bordered display_inline_block bgcolor_black_hover" title="Refresh Blockchain Data"  onClick={(e) => App.refreshBlockchainData()}>
//               <img src="artwork/refresh-ccw.svg" />
//             </div>
//             <div className="h25px bordered display_inline_block bgcolor_black_hover">
//               <img src="artwork/code.svg"  title="Show Dev Tools" onClick={(e) => openDevTools()}/>
//             </div>
//           </td>
//         </tr>
//         <tr>
//           <td className="bordered w250px h200px ta_center va_top">
//             <Branding/>
//             <Balance App={App}/>
//           </td>
//           <td className="bordered w200px h200px ta_center va_top font_size12">
//             <SendScreen />
//           </td>
//           <td className="bordered w200px h200px ta_center va_top">
//             <div id="receive" className="bordered w200px h100px bgcolor_black_hover">
//               Receive
//               <div className="ta_left">
//                 <div className="font_size12">Address</div>
//                 <span className="bgcolor_black">
//                   <img src="artwork/copy.svg" />
//                 </span>
//                 <span className="font_size12">{App.getAddress()}</span>
//               </div>
//             </div>
//             <div id="receive" className="bordered w200px h100px bgcolor_black_hover">
//               <div className="ta_left">
//                 <div className="font_size12">Ledger</div>
//                 <span className="bgcolor_black">
//                   <img src="artwork/smartphone.svg" />
//                 </span>
//                 <span className="font_size12">Verify Address on Ledger</span>
//               </div>
//             </div>
//           </td>
//         </tr>
//         <tr>
//           <td className="bordered w250px h200px ta_center va_top">
//             <Staking App={App} GuiToggles={GuiToggles}/>
//             <News/>
//             <SocialMedia GuiToggles={GuiToggles}/>
//           </td>
//           <td colSpan="2" className="bordered w400px h200px ta_center va_top">
//             <div id="transactions" className="bordered w500px h300px bgcolor_black_hover font_size12">
//               <div >Transaction List Status</div>
//               <br/> {App.getTransactionHistoryStatus()}
//               <div >Blockchain Status</div>
//               <br/> {App.getBlockchainStatus()}
//               <br/>
//               <div className="display_inline_block">Previous Transactions ({App.getParsedTransactionHistory().length}
//                 total)</div>
//               <div className="float_right display_inline_block">&nbsp;{App.getConfirmations()}&nbsp;
//                 Confirmations</div>
//               <div className="float_right display_inline_block">&nbsp;{App.getBlockchainState().height}&nbsp;
//                 Blocks</div>
//               <p></p>
//               <div className="h100px overflow_auto">
//                 <table className="w100pct no_border whitespace_nowrap font_size12">
//                   <tbody>
//                     <tr>
//                       <td className="no_border no_padding">Nbr</td>
//                       <td className="no_border no_padding">Value</td>
//                       <td className="no_border no_padding">Time</td>
//                       <td className="no_border no_padding">Type</td>
//                       <td className="no_border no_padding">TX</td>
//                     </tr>
//                     {
//                       App.getParsedTransactionHistory().map((item, index) => {
//                         return (<tr key={index}>
//                           <td className="no_border no_padding">{item.n}</td>
//                           <td className="no_border no_padding">{item.value}&nbsp;ELA</td>
//                           <td className="no_border no_padding">{item.time}</td>
//                           <td className="no_border no_padding">{item.type}</td>
//                           <td className="no_border no_padding">
//                             <a className="exit_link" href={item.txDetailsUrl} onClick={(e) => onLinkClick(e)}>{item.txHashWithEllipsis}</a>
//                           </td>
//                         </tr>)
//                       })
//                     }
//                   </tbody>
//                 </table>
//               </div>
//             </div>
//           </td>
//         </tr>
//       </tbody>
//     </table>
//   );
// } 

//  <div id="home" class="gridback w780h520px">

// <div class="logo-info">
//   <img src="artwork/lightwalletogo.svg" class="logoimage" height="40px" width="123px" />
//   <header>
//     <img src="artwork/system.svg" class="system-icon" />
//     <img src="artwork/refreshicon.svg" class="refresh-icon" />
//     <nav>
//       <img src="artwork/nav.svg" class="nav-icon" onClick={(e) => openDevTools()} />
//     </nav>
//   </header>
//   <div class="pricearea">
//     <p class="balance">balance</p>
//     <p class="usd-head">USD</p>
//     <p class="usd-balance">{App.getUSDBalance()}</p>
//     <p class="ela-balance gradient-font">{App.getELABalance()} ELA</p>
//   </div> 

//   <div class="stakingarea">
//     <p class="stakingtitle">staking</p>
//     <p class="candidate-total">95 candidates total</p>
//     <p class="votenow gradient-font">Vote now</p>
//     <img src="" alt="" class="arrow-right" />
//   </div>

  
//   <div id="scroll-radio">

//   </div>

//   <div>
//     <p class="article-days">3 days ago</p>
//     <p class="article-title">Elastos Financial Report</p>
//   </div>


// </div>

{/* <div class="send-area"> 
  <img src="artwork/sendicon.svg" class="send-icon" />
  <p class="send-text">Send</p>
  <button class="next-button">
  <p>Next</p>
  </button>
  <input type="text" id="ela-address__input" placeholder="Enter ELA Address" />
  <input type="number" id="ela-send__amount" placeholder="500" />
  <div class="quick-elaselector">
    <button class="quick-elaselector__icon quarter">1/4</button>
    <button class="quick-elaselector__icon half">Half</button>
    <button class="quick-elaselector__icon all">All</button>
  </div>
  <p class="elatext-send">ELA</p>

</div> */}

{/* <div class="receive-area">
  <img src="artwork/sendicon.svg" class="rec-icon" />
  <p class="rec-text">Receive</p>
  <p class="address-text">Address</p>
  <button class="copy-button">
    <img src="artwork/copyicon.svg" class="copy-icon" height="26px" width="26px" />
  </button>
  <p class="address-ex">{App.getAddress()}</p>
  <img src="artwork/qricon.svg" class="qr-icon" height="54px" width="54px" />
  <p class="scanqr-text">Scan <strong>QR code</strong> to get <br />ELA Address</p>
  <p class="howqr-text gradient-font">How QR works?</p>
  <img src="artwork/separator.svg" class="rec-separator" />
  <p class="ledger-heading">Ledger</p>
  <img src="artwork/ledgericon.svg" alt="" class="ledger-icon" height="24px" width="38px" />
  <p class="verifyledger-text">Verify address on <br /><strong>ledger</strong></p>

</div>
  
<div class="transaction-area">
  <p class="transactions-heading">Transactions</p>
  <p class="blockcount"><span>Blocks:</span><span>500001</span> </p>
  
  <table class="txtable">
    <tr class="txtable-headrow">
      <td>VALUE</td>
      <td>DATE</td>
      <td>TYPE</td>
      <td>TX</td>
    </tr>

    <tr class="txtable-row">
      <td>250 <span class="dark-font">ELA</span></td>
      <td>2020-02-17 <span class="dark-font">10:50</span></td>
      <td>Received</td>
      <td>5bfa9573d7bc89472a4b8ec5f1da0ed0947…</td>
    </tr>

    <tr class="txtable-row">
      <td>100 <span class="dark-font">ELA</span></td>
      <td>2020-02-12 <span class="dark-font">15:40</span></td>
      <td>Sent</td>
      <td>de02a581c2af72bee1ca</td>
    </tr>

    <tr class="txtable-row">
      <td>1000 <span class="dark-font">ELA</span></td>
      <td>2020-02-10 <span class="dark-font"> 20:40</span></td>
      <td>Received</td>
      <td>5bfa9573d7bc89472a4b</td>
    </tr>
  </table>

  <footer>
    <img src="artwork/tw.svg" height="22px" width="22px" />
    <img src="artwork/fb.svg" height="22px" width="22px" />
    <div class="logout-footer">
      <p class="logout-text">Logout onClick={(e) => App.showLogin()}</p>
      <img src="artwork/logout.svg" class="logout-image" onClick={(e) => App.showLogin()}/>
    </div>

  </footer>



</div>

</div>



]);  */}
