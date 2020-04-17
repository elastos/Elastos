const React = require('react');

module.exports = (props) => {
  const App = props.App;
  return (
      <div id="balance" className="pricearea">
        <p className="balance">balance</p>
        <p className="usd-head">USD</p>
        <p className="usd-balance">{App.getUSDBalance()}</p>
        <p className="ela-balance gradient-font">{App.getELABalance()} ELA</p>
      </div>

  )
}

//     <div id="balance" className="bordered w250px h90px bgcolor_black_hover position_relative">
//       <table>
//         <tbody>
//           <tr>
//             <td className="w50px">
//               <a className="rotate_n90 exit_link" target="_blank" href="https://api.coingecko.com/api/v3/simple/price?ids=elastos&vs_currencies=usd">Balance</a>
//             </td>
//             <td className="w100px ta_left">
//               <span className="font_size24">USD&nbsp;</span>
//               <span className="font_size24">{App.getUSDBalance()}</span>
//               <br />
//               <span className="color_orange">{App.getELABalance()}</span>
//               <span className="color_orange">&nbsp;ELA</span>
//             </td>
//           </tr>
//         </tbody>
//       </table>
//     </div>
//   )
// }
