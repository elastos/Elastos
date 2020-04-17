const React = require('react');

module.exports = (props) => {
  const GuiToggles = props.GuiToggles;
  return (
        <footer>

          <img src="artwork/tw.svg" height="22px" width="22px" />
          <img src="artwork/fb.svg" height="22px" width="22px" />
          <div className="logout-footer">
            <p id="logout-text" onClick={(e) => GuiToggles.showLanding()}>Logout </p>
            <img src="artwork/logout.svg" className="logout-image" onClick={(e) =>
            GuiToggles.showLanding()}/>
          </div>

        </footer>
  )
}

//     <div className="bordered w250px h50px">
//       <table className="w100pct">
//         <tbody>
//           <tr>
//             <td id="facebook" className="w50px h50px ta_center va_bottom bgcolor_black_hover">
//               <a className="exit_link" target="_blank" href="https://www.facebook.com/elastosorg/"><img src="artwork/facebook.svg" /></a>
//             </td>
//             <td id="twitter" className="w50px h50px ta_center va_bottom bgcolor_black_hover">
//               <a className="exit_link" target="_blank" href="https://twitter.com/Elastos_org"><img src="artwork/twitter.svg" /></a>
//             </td>
//             <td id="logout" className="w100px h50px ta_center va_bottom bgcolor_black_hover">

//             <span className="bgcolor_black_hover">
//               <img src="artwork/log-out.svg"  title="Logout" onClick={(e) =>
//             GuiToggles.showLanding()}/>
//             </span>
//             </td>
//           </tr>
//         </tbody>
//       </table>
//     </div>
//   )
// }
