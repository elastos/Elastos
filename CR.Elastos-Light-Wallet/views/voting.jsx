const React = require('react');

const Menu = require('./partial/menu.jsx');

const Banner = require('./partial/banner.jsx');

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
  const onLinkClick = props.onLinkClick;

  const showMenu = () => {
    GuiToggles.showMenu('voting');
  }

  const ProducerSelectionButtonText = (props) => {
    // mainConsole.log('INTERIM ProducerSelectionButtonText props', props);
    // mainConsole.log('INTERIM ProducerSelectionButtonText item', props.item);
    // mainConsole.log('INTERIM ProducerSelectionButtonText isCandidate', props.item.isCandidate);
    const item = props.item;
    const isCandidate = item.isCandidate;
    if (isCandidate) {
      return (<img src="artwork/check-square.svg" />)
    } else {
      return (<img src="artwork/square.svg" />)
    }
  }

  return (
    <div id="voting" className="gridback-voting w780h520px">
     <Banner App={App} GuiToggles={GuiToggles} page="voting"/>
     <Menu App={App} openDevTools={openDevTools} GuiToggles={GuiToggles} page="voting"/>
      {/* <Menu App={App} openDevTools={openDevTools} GuiToggles={GuiToggles}/> */}
      <div className="logo-info">
      <Branding onClick={(e) => GuiToggles.showHome()}/>
      <header>
        <img src="artwork/refreshicon.svg" className="refresh-icon" title="Refresh" onClick={(e) => App.refreshBlockchainData()} />
        {/* Change to menu below */}
        <nav id="votingMenuOpen" title="menu" onClick={(e) => showMenu()}>
          <img src="artwork/nav.svg" className="nav-icon dark-hover" onClick={(e) => showMenu()}/>
        </nav>
      </header>
      <div className="pricearea">
       <Balance App={App}/>
      </div>

      <div className="stakingarea">
       <Staking App={App} GuiToggles={GuiToggles}/>
      </div>


      <div id="scroll-radio">

      </div>

      <div>
        <News App={App} onLinkClick={onLinkClick}/>
      </div>

      </div>

      <div className="voting-row1">
        <div>
          <img src="artwork/voting-back.svg" className="scale-hover" width="33px" height="33px" onClick={(e) => GuiToggles.showHome()}/>
          <p className="display_inline_block votes-header">Votes</p>
          <p className="display_inline_block candidate-status status-font">Status: {App.getProducerListStatus()} </p>
          <p className="display_inline_block status-font">Candidates: {App.getParsedProducerList().producers.length} </p>
          <p className="display_inline_block status-font">Selected: {App.getParsedProducerList().producersCandidateCount}/36 </p>
          </div>
      </div>

      <div className="voting-row2 overflow_auto scrollbar">
      <table className="w100pct no_border whitespace_nowrap txtable">
                <tbody>
                  <tr className="txtable-headrow">
                    <td className="no_border no_padding">N</td>
                    <td className="no_border no_padding">Nickname</td>
                    <td className="no_border no_padding">Active</td>
                    <td className="no_border no_padding">Votes</td>
                    <td className="no_border no_padding">Select</td>
                  </tr>
                  {
                    App.getParsedProducerList().producers.map((item, index) => {
                      return (<tr className={item.isCandidate ? 'txtable-row voting-selected ': 'txtable-row voting-hover'} key={index} onClick={(e) => App.toggleProducerSelection({index})}>
                        <td className="no_border no_padding">{item.n}</td>
                        <td className="no_border no_padding">{item.nickname}</td>
                        {/* <td className="no_border no_padding">{item.active}</td> */}
                        <td className="no_border no_padding">
                        {Number(item.active) ? (<img src="artwork/greenstatus.svg" />) : (<img src="artwork/redstatus.svg" />)
                        } </td>
                        <td className="no_border no_padding">{item.votes}</td>
                        <td className="white_on_purple_with_hover h20px fake_button">
                          <ProducerSelectionButtonText item={item}/>
                        </td>
                      </tr>)
                    })
                  }
                </tbody>
              </table>
      </div>

      <div className="voting-row3">
        <button className='votingselect-button scale-hover' title="Select previous voting list" onClick={() => App.selectActiveVotes()} >Select Previous</button>
        <button className='votingselect-button marginright_auto scale-hover' title='Clear Selection' onClick={() => App.clearSelection()}>Clear Selection</button>
        <button  onClick={(e) => App.sendVoteTx()} className="scale-hover voting-button">Vote</button>
      </div>

      <div className="voting-row4">
        <p className="display_inline_block active-heading">Active Votes</p>
        {/*
        <p className="display_inline_block candidatevote-status status-font flex_center">Status: {App.getCandidateVoteListStatus()}</p>
         */}
        <p className="display_inline_block status-font">Voted {App.getParsedCandidateVoteList().candidateVotes.length}/36</p>

      </div>

      <div className="voting-row5 overflow_auto scrollbar">
              <table className="w100pct no_border whitespace_nowrap font_size16 txtable">
                <tbody>
                  <tr className="txtable-headrow">
                    <td className="no_border no_padding">N</td>
                    <td className="no_border no_padding">Nickname</td>
                    <td className="no_border no_padding">Votes</td>
                    <td className="no_border no_padding">State</td>
                  </tr>
                  {
                    App.getParsedCandidateVoteList().candidateVotes.map((item, index) => {
                      return (<tr className="txtable-row" key={index}>
                        <td className="no_border no_padding">{item.n}</td>
                        <td className="no_border no_padding">{item.nickname}</td>
                        <td className="no_border no_padding">{item.votes} ELA</td>
                        <td className="no_border no_padding">{item.state}</td>
                      </tr>)
                    })
                  }
                </tbody>
              </table>
      </div>

      <div>

      <SocialMedia GuiToggles={GuiToggles}  onLinkClick={onLinkClick}/>

      </div>
      </div>
    );
    }
