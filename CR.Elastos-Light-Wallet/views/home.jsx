const React = require('react');

module.exports = (props) => {
  const App = props.App;
  const openDevTools = props.openDevTools;
  const Version = props.Version;
  return ([

    
  /* <table id="home" className="bordered w750h520px">
    <tbody>
      <tr>
        <td className="bordered w250px h20px ta_center va_top">
        </td>
        <td className="bordered w250px h20px ta_center va_top">
        </td>
        <td className="bordered w250px h20px ta_right va_top">
          <Version/>
          <button className="bgcolor_black_hover" title="menu">
            <img src="artwork/menu.svg" />
          </button>
          <button className="bgcolor_black_hover" title="Refresh Blockchain Data"  onClick={(e) => App.refreshBlockchainData()}>
            <img src="artwork/refresh-ccw.svg" />
          </button>
          <button className="bgcolor_black_hover">
            <img src="artwork/code.svg"  title="Show Dev Tools" onClick={(e) => openDevTools()}/>
          </button>
        </td>
      </tr>
      <tr>
        <td className="bordered w250px h200px ta_center va_top">
          <div id="branding" className="bordered w250px h90px bgcolor_black_hover">
            Branding
          </div>
          <div id="balance" className="bordered w250px h90px bgcolor_black_hover position_relative">
            <table>
              <tbody>
                <tr>
                  <td className="w50px">
                    <a className="rotate_n90 exit_link" target="_blank" href="https://api.coingecko.com/api/v3/simple/price?ids=elastos&vs_currencies=usd">Balance</a>
                  </td>
                  <td className="w100px ta_left">
                    <span className="font_size24">USD&nbsp;</span>
                    <span className="font_size24">{App.getUSDBalance()}</span>
                    <br />
                    <span className="color_orange">{App.getELABalance()}</span>
                    <span className="color_orange">&nbsp;ELA</span>
                  </td>
                </tr>
              </tbody>
            </table>
          </div>
        </td>
        <td className="bordered w200px h200px ta_center va_top">
          <div id="send" className="bordered w200px h200px bgcolor_black_hover">
            Send
          </div>
        </td>
        <td className="bordered w200px h200px ta_center va_top">
          <div id="receive" className="bordered w200px h100px bgcolor_black_hover">
            Receive
            <div className="ta_left">
              <div className="font_size12">Address</div>
              <button className="bgcolor_black">
                <img src="artwork/copy.svg" />
              </button>
              <span className="font_size12">{App.getAddress()}</span>
            </div>
          </div>
          <div id="receive" className="bordered w200px h100px bgcolor_black_hover">
            <div className="ta_left">
              <div className="font_size12">Ledger</div>
              <button className="bgcolor_black">
                <img src="artwork/smartphone.svg" />
              </button>
              <span className="font_size12">Verify Address on Ledger</span>
            </div>
          </div>
        </td>
      </tr>
      <tr>
        <td className="bordered w250px h200px ta_center va_top">
          <div id="staking" className="bordered w250px h110px bgcolor_black_hover position_relative">
            <table>
              <tbody>
                <tr>
                  <td className="w50px">
                    <div className="rotate_n90">Staking</div>
                  </td>
                  <td className="w150px ta_left">
                    <span className="font_size12">Z</span>
                    <span className="font_size12">Candidates Total</span>
                    <div className="font_size24">Vote Now</div>
                  </td>
                </tr>
              </tbody>
            </table>
          </div>
          <div id="news" className="bordered w250px h110px bgcolor_black_hover">
            <a className="exit_link" target="_blank" href="https://news.elastos.org/feed/">News</a>
          </div>
          <div className="bordered w250px h50px">
            <table className="w100pct">
              <tbody>
                <tr>
                  <td id="facebook" className="w50px h50px ta_center va_bottom bgcolor_black_hover">
                    <a className="exit_link" target="_blank" href="https://www.facebook.com/elastosorg/"><img src="artwork/facebook.svg" /></a>
                  </td>
                  <td id="twitter" className="w50px h50px ta_center va_bottom bgcolor_black_hover">
                    <a className="exit_link" target="_blank" href="https://twitter.com/Elastos_org"><img src="artwork/twitter.svg" /></a>
                  </td>
                  <td id="logout" className="w100px h50px ta_center va_bottom bgcolor_black_hover">

                  <button className="bgcolor_black_hover">
                    <img src="artwork/log-out.svg"  title="Logout" onClick={(e) => App.showLogin()}/>
                  </button>
                    Logout
                  </td>
                </tr>
              </tbody>
            </table>
          </div>
        </td>
        <td colSpan="2" className="bordered w400px h200px ta_center va_top">
          <div id="transactions" className="bordered w400px h300px bgcolor_black_hover">
            Transactions
          </div>
        </td>
      </tr>
    </tbody>
  </table> */

    
      <div id="home" class="gridback w780h520px">

        <div class="logo-info">
          <img src="artwork/lightwalletogo.svg" class="logoimage" height="40px" width="123px" />
          <header>
            <img src="artwork/system.svg" class="system-icon" />
            <img src="artwork/refreshicon.svg" class="refresh-icon" />
            <nav>
              <img src="artwork/nav.svg" class="nav-icon" onClick={(e) => openDevTools()} />
            </nav>
          </header>
          <div class="pricearea">
            <p class="balance">balance</p>
            <p class="usd-head">USD</p>
            <p class="usd-balance">{App.getUSDBalance()}</p>
            <p class="ela-balance gradient-font">{App.getELABalance()} ELA</p>
          </div> 

          <div class="stakingarea">
            <p class="stakingtitle">staking</p>
            <p class="candidate-total">95 candidates total</p>
            <p class="votenow gradient-font">Vote now</p>
            <img src="" alt="" class="arrow-right" />
          </div>

          
          <div id="scroll-radio">

          </div>

          <div>
            <p class="article-days">3 days ago</p>
            <p class="article-title">Elastos Financial Report</p>
          </div>


        </div>

        <div class="send-area"> 
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



  ]);

}
