const React = require('react');

const Menu = require('./partial/menu.jsx');

const Banner = require('./partial/banner.jsx');

module.exports = (props) => {
  const App = props.App;
  const openDevTools = props.openDevTools;
  const GuiToggles = props.GuiToggles;
  const useMnemonic = () => {
    const success = App.getPublicKeyFromMnemonic();
    if(success) {
      GuiToggles.showHome();
    }
  }
  const showMenu = () => {
    GuiToggles.showMenu('loginMnemonic');
  }
  return (

  <div id="loginMnemonic">
  <div className="login-div ">
   <Banner App={App} GuiToggles={GuiToggles} page="loginMnemonic"/>
   <Menu App={App} openDevTools={openDevTools} GuiToggles={GuiToggles} page="loginMnemonic"/>
   <header>
     <nav id="loginMnemonicMenuOpen" title="menu" onClick={(e) => showMenu()}>
       <img src="artwork/nav.svg" className="nav-icon dark-hover" onClick={(e) => showMenu()}/>
     </nav>
   </header>
    <div className="flex_center w100pct">
    <img className="flex1 scale-hover" src="artwork/voting-back.svg"  height="38px" width="38px" onClick={(e)=> GuiToggles.showLanding()}/>
    <img src="artwork/logonew.svg" height="80px" width="240px" />
    <div className="flex1"></div>
    </div>
    <p className="address-text font_size24 margin_none display_inline_block gradient-font">Enter Mnemonics</p>
    <textarea className="qraddress-div color_white textarea-placeholder padding_5px" type="text" rows="4" cols="50" id="mnemonic" placeholder="Enter 12 word mnemonic/seed phrase"></textarea>
    <div className="flex_center">
    <button className="proceed-btn scale-hover" onClick={(e)=> useMnemonic()}>
          <p>Proceed</p>
          </button>
  </div>

  <div>
  <p className="gradient-font font_size20 ta_center list-none" >Tips</p>
  <ul className="color_white ta_left">
    <li>Enter your 12 word mnemonic phrase above.</li>
    <li>Use a single space between each word, with no space before the first and last word.</li>
    <li>All words should be in lowercase.</li>
    <li>Take precautions when entering your mnemonic phrase, make sure no one is watching physically or virtually.</li>
  </ul>
  </div>
  </div>
</div>);
}












//   (
//   <table id="loginMnemonic" className="bordered w750h520px">
//     <tbody>
//       <tr>
//         <td colSpan="2">
//           <div>Mnemonic</div>
//         </td>
//       </tr>
//       <tr>
//         <td colSpan="2">
//           <textarea className="monospace" type="text" rows="4" cols="50" id="mnemonic" placeholder="Mnemonic"></textarea>
//         </td>
//       </tr>
//       <tr>
//         <td className="ta_left">
//           <div className="bordered bgcolor_black_hover display_inline_block" onClick={(e)=> useMnemonic()}>Use Mnemonic</div>
//         </td>
//         <td className="ta_right">
//           <div className="bordered bgcolor_black_hover display_inline_block" onClick={(e)=> GuiToggles.showLanding()}>Back</div>
//         </td>
//       </tr>
//     </tbody>
//   </table>
//   );
// }
