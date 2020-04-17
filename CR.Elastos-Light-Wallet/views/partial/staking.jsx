const React = require('react');

module.exports = (props) => {
  const App = props.App;
  const GuiToggles = props.GuiToggles;
  return (

  <div id="staking" className="stakingarea dark-hover" onClick={(e) => GuiToggles.showVoting()}>
    <p className="stakingtitle">staking</p>
    <p className="candidate-total">{App.getParsedProducerList().producers.length} candidates total</p>
    <p className="candidate-voted">{App.getParsedCandidateVoteList().candidateVotes.length}/36 Active Votes</p>
    <p className="votenow gradient-font" onClick={(e) => GuiToggles.showVoting()} >Vote now</p>
    <img src="" alt="" className="arrow-right" />
  </div>

  )
  }

//     <div id="staking" className="bordered w250px h110px bgcolor_black position_relative">
//       <table>
//         <tbody>
//           <tr>
//             <td className="w50px">
//               <div className="rotate_n90">Staking</div>
//             </td>
//             <td className="w150px ta_left">
//               <span className="font_size12">{App.getProducerListStatus()}</span>
//               <br/>
//               <span className="font_size12">{App.getParsedProducerList().totalvotes}</span>
//               <span className="font_size12">&nbsp;Votes</span>
//               <br/>
//               <span className="font_size12">{App.getParsedProducerList().totalcounts}</span>
//               <span className="font_size12">&nbsp;Counts</span>
//               <br/>
//               <span className="font_size12">{App.getParsedProducerList().producersCandidateCount}</span>
//               <span className="font_size12">&nbsp;Selected Candidates</span>
//               <br/>
//               <span className="font_size12">{App.getParsedProducerList().producers.length}</span>
//               <span className="font_size12">&nbsp;Candidates Total</span>
//               <div className=""></div>
//               <div className="font_size24 bordered display_inline bgcolor_black_hover" onClick={(e) => GuiToggles.showVoting()}>Vote Now</div>
//             </td>
//           </tr>
//         </tbody>
//       </table>
//     </div>
//   )
// }
