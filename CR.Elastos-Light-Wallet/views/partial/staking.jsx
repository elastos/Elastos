const React = require('react');

module.exports = (props) => {
  const App = props.App;
  const GuiToggles = props.GuiToggles;
  return (

  <div id="staking" className="stakingarea dark-hover2" onClick={(e) => GuiToggles.showVoting()}>
    <p className="stakingtitle cursor_def">staking</p>
    <p className="candidate-total cursor_def">{App.getParsedProducerList().producers.length} candidates total</p>
    <p className="candidate-voted cursor_def">{App.getParsedCandidateVoteList().candidateVotes.length}/36 Active Votes</p>
    <p className="votenow gradient-font cursor_def" onClick={(e) => GuiToggles.showVoting()} >Vote now</p>
    <img src="artwork/arrow.svg" alt="" className="arrow-right" />
  </div>

  )
  }
