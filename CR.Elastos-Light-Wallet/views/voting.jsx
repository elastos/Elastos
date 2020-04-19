const React = require('react');

const Menu = require('./partial/menu.jsx');

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
      return ('Yes')
    } else {
      return ('No')
    }
  }

  return (
    <div id="voting" className="gridback-voting w780h520px">
     <Menu App={App} openDevTools={openDevTools} GuiToggles={GuiToggles} page="voting"/>
      {/* <Menu App={App} openDevTools={openDevTools} GuiToggles={GuiToggles}/> */}
      <div className="logo-info">
      <Branding onClick={(e) => GuiToggles.showHome()}/>
      <header>
        <img src="artwork/system.svg" className="system-icon" />
        <img src="artwork/refreshicon.svg" className="refresh-icon" onClick={(e) => App.refreshBlockchainData()} />
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
        <News App={App}/>
      </div>

      </div>

      <div className="voting-row1">
        <div>
          <img src="artwork/voting-back.svg" onClick={(e) => GuiToggles.showHome()}/>
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
                      return (<tr className="txtable-row" key={index}>
                        <td className="no_border no_padding">{item.n}</td>
                        <td className="no_border no_padding">{item.nickname}</td>
                        <td className="no_border no_padding">{item.active}</td>
                        <td className="no_border no_padding">{item.votes}</td>
                        <td className="white_on_purple_with_hover h20px fake_button" onClick={(e) => App.toggleProducerSelection({index})}>
                          <ProducerSelectionButtonText item={item}/>
                        </td>
                      </tr>)
                    })
                  }
                </tbody>
              </table>
      </div>

      <div className="voting-row3">
        <button  onClick={(e) => App.sendVoteTx()} className="dark-hover">Vote</button>
      </div>

      <div className="voting-row4">
        <p className="display_inline_block active-heading">Active Votes</p>
        <p className="display_inline_block candidate-status status-font">Status: {App.getCandidateVoteListStatus()}</p>
        <p className="display_inline_block status-font">Voted {App.getParsedCandidateVoteList().candidateVotes.length}/36</p>

      </div>

      <div className="voting-row5 overflow_auto scrollbar">
              <table className="w100pct no_border whitespace_nowrap font_size12 txtable">
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
                        <td className="no_border no_padding">{item.votes}</td>
                        <td className="no_border no_padding">{item.state}</td>
                      </tr>)
                    })
                  }
                </tbody>
              </table>
      </div>

      <div>

         <SocialMedia GuiToggles={GuiToggles}/>

      </div>
      </div>
    );
    }

      {/* <table>
      <tbody>
        <tr>

          <td colSpan="2" className="bordered w400px h200px ta_center va_top">
            <div className="bordered padding_5px bgcolor_black_hover" onClick={(e) => GuiToggles.showHome()}>Home</div>
            <div className="display_inline_block">Producer List Status</div>
            <br/> {App.getProducerListStatus()}
            <br/>
            <div className="display_inline_block">
              <span className="padding_2px">{App.getParsedProducerList().totalvotes}</span>
              Votes</div>
            <div className="display_inline_block">
              <span className="padding_2px">{App.getParsedProducerList().totalcounts}</span>
              Counts</div>
            <div className="display_inline_block">
              <span className="padding_2px">{App.getParsedProducerList().producersCandidateCount}</span>
              Selected Candidates</div>
            <div className="display_inline_block">
              Candidates (
              <span className="padding_2px">{App.getParsedProducerList().producers.length}</span>
              total)</div>
            <p></p>
            <div className="h110px overflow_auto">
              <table className="w100pct no_border whitespace_nowrap font_size12 txtable">
                <tbody>
                  <tr>
                    <td className="no_border no_padding">N</td>
                    <td className="no_border no_padding">Nickname</td>
                    <td className="no_border no_padding">Active</td>
                    <td className="no_border no_padding">Votes</td>
                    <td className="no_border no_padding">Select</td>
                  </tr>
                  {
                    App.getParsedProducerList().producers.map((item, index) => {
                      return (<tr className="txtable-row" key={index}>
                        <td className="no_border no_padding">{item.n}</td>
                        <td className="no_border no_padding">{item.nickname}</td>
                        <td className="no_border no_padding">{item.active}</td>
                        <td className="no_border no_padding">{item.votes}</td>
                        <td className="white_on_purple_with_hover h20px fake_button" onClick={(e) => App.toggleProducerSelection({index})}>
                          <ProducerSelectionButtonText item={item}/>
                        </td>
                      </tr>)
                    })
                  }
                </tbody>
              </table>
            </div>
            <div className="bordered padding_5px bgcolor_black_hover" onClick={(e) => App.sendVoteTx()}>Send Votes</div>
          </td>
        </tr>
        <tr>

          <td colSpan="2" className="txtable w400px h200px ta_center va_top">
            <div className="display_inline_block">Candidate List Status</div>
            <br/> {App.getCandidateVoteListStatus()}
            <br/>
            <div className="display_inline_block">
              Candidate Votes (
              <span>{App.getParsedCandidateVoteList().candidateVotes.length}</span>
              total)</div>
            <p></p>
            <div className="h200px overflow_auto">
              <table className="w100pct no_border whitespace_nowrap font_size12 txtable">
                <tbody>
                  <tr>
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
                        <td className="no_border no_padding">{item.votes}</td>
                        <td className="no_border no_padding">{item.state}</td>
                      </tr>)
                    })
                  }
                </tbody>
              </table>
            </div>
          </td>
        </tr>
      </tbody>
    </table> */}



{/* <table>
      <tbody>
        <tr>
          <td className="bordered w250px h200px ta_center va_top">
            <Branding />
            <Balance App={App}/>
          </td>
          <td colSpan="2" className="bordered w400px h200px ta_center va_top">
            <div className="bordered padding_5px bgcolor_black_hover" onClick={(e) => GuiToggles.showHome()}>Home</div>
            <div className="display_inline_block">Producer List Status</div>
            <br/> {App.getProducerListStatus()}
            <br/>
            <div className="display_inline_block">
              <span className="padding_2px">{App.getParsedProducerList().totalvotes}</span>
              Votes</div>
            <div className="display_inline_block">
              <span className="padding_2px">{App.getParsedProducerList().totalcounts}</span>
              Counts</div>
            <div className="display_inline_block">
              <span className="padding_2px">{App.getParsedProducerList().producersCandidateCount}</span>
              Selected Candidates</div>
            <div className="display_inline_block">
              Candidates (
              <span className="padding_2px">{App.getParsedProducerList().producers.length}</span>
              total)</div>
            <p></p>
            <div className="h110px overflow_auto">
              <table className="w100pct no_border whitespace_nowrap font_size12">
                <tbody>
                  <tr>
                    <td className="no_border no_padding">N</td>
                    <td className="no_border no_padding">Nickname</td>
                    <td className="no_border no_padding">Active</td>
                    <td className="no_border no_padding">Votes</td>
                    <td className="no_border no_padding">Select</td>
                  </tr>
                  {
                    App.getParsedProducerList().producers.map((item, index) => {
                      return (<tr key={index}>
                        <td className="no_border no_padding">{item.n}</td>
                        <td className="no_border no_padding">{item.nickname}</td>
                        <td className="no_border no_padding">{item.active}</td>
                        <td className="no_border no_padding">{item.votes}</td>
                        <td className="white_on_purple_with_hover h20px fake_button" onClick={(e) => App.toggleProducerSelection({index})}>
                          <ProducerSelectionButtonText item={item}/>
                        </td>
                      </tr>)
                    })
                  }
                </tbody>
              </table>
            </div>
            <div className="bordered padding_5px bgcolor_black_hover" onClick={(e) => App.sendVoteTx()}>Send Votes</div>
          </td>
        </tr>
        <tr>
          <td className="bordered w250px h200px ta_center va_top">
            <Staking App={App} GuiToggles={GuiToggles}/>
            <News/>
            <SocialMedia GuiToggles={GuiToggles}/>
          </td>
          <td colSpan="2" className="bordered w400px h200px ta_center va_top">
            <div className="display_inline_block">Candidate List Status</div>
            <br/> {App.getCandidateVoteListStatus()}
            <br/>
            <div className="display_inline_block">
              Candidate Votes (
              <span>{App.getParsedCandidateVoteList().candidateVotes.length}</span>
              total)</div>
            <p></p>
            <div className="h200px overflow_auto">
              <table className="w100pct no_border whitespace_nowrap font_size12">
                <tbody>
                  <tr>
                    <td className="no_border no_padding">N</td>
                    <td className="no_border no_padding">Nickname</td>
                    <td className="no_border no_padding">Votes</td>
                    <td className="no_border no_padding">State</td>
                  </tr>
                  {
                    App.getParsedCandidateVoteList().candidateVotes.map((item, index) => {
                      return (<tr key={index}>
                        <td className="no_border no_padding">{item.n}</td>
                        <td className="no_border no_padding">{item.nickname}</td>
                        <td className="no_border no_padding">{item.votes}</td>
                        <td className="no_border no_padding">{item.state}</td>
                      </tr>)
                    })
                  }
                </tbody>
              </table>
            </div>
          </td>
        </tr>
      </tbody>
    </table>  */}
