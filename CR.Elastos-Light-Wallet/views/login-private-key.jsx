const React = require('react');

const Menu = require('./partial/menu.jsx');

const Banner = require('./partial/banner.jsx');

module.exports = (props) => {
  const App = props.App;
  const openDevTools = props.openDevTools;
  const GuiToggles = props.GuiToggles;
  const usePrivateKey = () => {
    const success = App.getPublicKeyFromPrivateKey();
    if(success) {
      GuiToggles.showHome();
    }
  }
  const showMenu = () => {
    GuiToggles.showMenu('loginPrivateKey');
  }
  return (
  <div id="loginPrivateKey">
  <div className="login-div ">
   <Banner App={App} GuiToggles={GuiToggles} page="loginPrivateKey"/>
   <Menu App={App} openDevTools={openDevTools} GuiToggles={GuiToggles} page="loginPrivateKey"/>
   <header>
     <nav id="loginPrivateKeyMenuOpen" title="menu" onClick={(e) => showMenu()}>
       <img src="artwork/nav.svg" className="nav-icon dark-hover" onClick={(e) => showMenu()}/>
     </nav>
   </header>
    <div className="flex_center w100pct">
    <img className="flex1 scale-hover" src="artwork/voting-back.svg" height="38px" width="38px" onClick={(e)=> GuiToggles.showLanding()}/>
    <img src="artwork/logonew.svg" height="80px" width="240px" />
    <div className="flex1"></div>
    </div>
    <p className="address-text font_size24 margin_none display_inline_block gradient-font">Enter Private Key</p>
    <textarea className="qraddress-div color_white textarea-placeholder padding_5px" type="text" rows="4" cols="50" id="privateKey" placeholder="Enter Private Key"></textarea>
    <div className="flex_center">
    <button className="proceed-btn scale-hover" onClick={(e)=> usePrivateKey()}>
          <p>Proceed</p>
          </button>
  </div>
<div>
  <p className="gradient-font font_size20 ta_center list-none" >Tips</p>
  <ul className="color_white ta_left">
    <li>Enter your Private Key above.</li>
    <li>Your Private Key is a string of numbers and letters.</li>
    <li>Please use the Mnemonic login if you have your 12 seed words.</li>
    <li>Please take precautions when entering your Private Key, make sure nobody is watching you physically or virtually.</li>
  </ul>
  </div>
  </div>
</div>);
}














//   (
//   <table id="loginPrivateKey" className="bordered w750h520px">
//     <tbody>
//       <tr>
//         <td colSpan="2">
//           <div>Private Key</div>
//         </td>
//       </tr>
//       <tr>
//         <td colSpan="2">
//           <input className="monospace" type="text" size="64" id="privateKey" placeholder="Private Key"></input>
//         </td>
//       </tr>
//       <tr>
//         <td className="ta_left">
//           <div className="bordered bgcolor_black_hover display_inline_block" onClick={(e)=> usePrivateKey()}>Use Private Key</div>
//         </td>
//         <td className="ta_right">
//           <div className="bordered bgcolor_black_hover display_inline_block ta_right" onClick={(e)=> GuiToggles.showLanding()}>Back</div>
//         </td>
//       </tr>
//     </tbody>
//   </table>
//   );
// }
