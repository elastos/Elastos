const React = require('react');

module.exports = (props) => {
  const app = props.app;
  const openDevTools = props.openDevTools;
  const Version = props.Version;
  return (
  <table id="home" className="bordered w750h520px">
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
          <button className="bgcolor_black_hover" title="Refresh Blockchain Data"  onClick={(e) => app.refreshBlockchainData()}>
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
                    <div className="font_size24">USD</div>
                    <div className="font_size24">Y</div>
                    <span className="color_orange">X</span>
                    <span className="color_orange">ELA</span>
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
              <span className="font_size12">ADDRESS</span>
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
                    <img src="artwork/log-out.svg"  title="Logout" onClick={(e) => app.showLogin()}/>
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
  </table>
  );
}
