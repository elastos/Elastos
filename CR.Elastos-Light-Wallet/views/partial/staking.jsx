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
