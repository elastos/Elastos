"use strict";

/** imports */
const React = require('react');
const ReactDOM = require('react-dom');

const electron = require('electron');
const shell = electron.shell;
const remote = electron.remote;
const clipboard = electron.clipboard;

/** views */
const Home = require('./home.jsx');
const Landing = require('./landing.jsx');
const LoginMnemonic = require('./login-mnemonic.jsx');
const LoginPrivateKey = require('./login-private-key.jsx');
const Temp = require('./temp.jsx');

/** modules */
const app = require('../scripts/app.js');
const guiToggles = require('../scripts/guiToggles.js');

/** functions */

// const Version = () => {
//   return remote.app.getVersion();
// }
//

//
// const UseLedgerButton = () => {
//   if (
//     app.getLedgerDeviceInfo()
//     ? app.getLedgerDeviceInfo().enabled
//     : false) {
//     return (<div className="white_on_gray bordered display_inline_block float_right fake_button rounded padding_5px" onClick={(e) => app.getPublicKeyFromLedger()}>Use Ledger</div>);
//   } else {
//     return (<div className="white_on_pink bordered display_inline_block float_right fake_button rounded padding_5px">Use Ledger</div>);
//   }
//   return (<div/>);
// }
//
// const TransactionHistoryElementIcon = (props) => {
//   const item = props.item;
//   if (item.type == 'input') {
//     return (<img src="artwork/received-ela.svg"/>);
//   }
//   if (item.type == 'output') {
//     return (<img src="artwork/sent-ela.svg"/>);
//   }
//   return (<div/>);
// }
//
// const ProducerSelectionButtonText = (props) => {
//   const item = props.item;
//   const isCandidate = item.isCandidate;
//   if (isCandidate) {
//     return ('Yes')
//   } else {
//     return ('No')
//   }
// }

const onLinkClick = (event) => {
  event.preventDefault();
  shell.openExternal(event.currentTarget.href);
}

const openDevTools = () => {
  try {
    const window = remote.getCurrentWindow();
    window.webContents.openDevTools();
  } catch (e) {
    alert(`error:${e}`)
  }
}

/* actions */
// <Version/>
// showHome
// showSend
// showReceive
// showTransactions
// showVoting
// showPrivateKeyEntry
// getPublicKeyFromPrivateKey
// showMnemonicEntry
// getPublicKeyFromMnemonic
// showGenerateNewPrivateKey
// showGenerateNewMnemonic
// showLogin
// showConfirmAndSeeFees
// updateAmountAndFeesAndRenderApp
// sendAmountToAddress
// cancelSend
// showTransactions
// sendVoteTx

// getConfirmations
// <br/> {transactionHistoryStatus}
// <br/> {blockchainStatus}
// parsedTransactionHistory
    // <tbody>
    //   <tr>
    //     <td className="no_border no_padding">Nbr</td>
    //     <td className="no_border no_padding">Icon</td>
    //     <td className="no_border no_padding">Value</td>
    //     <td className="no_border no_padding">TX</td>
    //     <td className="no_border no_padding">Time</td>
    //   </tr>
    //   {
    //     parsedTransactionHistory.map((item, index) => {
    //       if (index > 2) {
    //         return undefined;
    //       }
    //       return (<tr key={index}>
    //         <td className="no_border no_padding">{item.n}</td>
    //         <td className="no_border no_padding">
    //           <TransactionHistoryElementIcon item={item}/>{/* item.type */}
    //         </td>
    //         <td className="no_border no_padding">{item.value}
    //           ELA</td>
    //         <td className="no_border no_padding">
    //           <a href={item.txDetailsUrl} onClick={(e) => onLinkClick(e)}>{item.txHash}</a>
    //         </td>
    //         <td className="no_border no_padding">
    //           {item.time}
    //         </td>
    //       </tr>)
    //     })
    //   }
    // </tbody>
// getConfirmations()
// blockchainState.height
// address
// balance
//  balanceStatus

// send
  // <tr id="send-amount">
  //   <td className="black_on_white h20px darkgray_border">
  //     <div className="gray_on_white">Send Amount</div>
  //     <br/>
  //     <input style={{
  //         fontFamily: 'monospace'
  //       }} type="text" size="64" id="sendAmount" placeholder="Send Amount"></input>
  //   </td>
  // </tr>
  // <tr id="to-address">
  //   <td className="black_on_white h20px darkgray_border">
  //     <div className="gray_on_white">To Address</div>
  //     <br/>
  //     <input style={{
  //         fontFamily: 'monospace'
  //       }} type="text" size="64" id="sendToAddress" placeholder="Send To Address"></input>
  //   </td>
  // </tr>
  // <table>
  //   <tbody>
  //     {
  //       sendToAddressStatuses.map((sendToAddressStatus, index) => {
  //         return (<tr key={index}>
  //           <td>{sendToAddressStatus}</td>
  //         </tr>)
  //       })
  //     }
  //     {
  //       sendToAddressLinks.map((item, index) => {
  //         return (<tr key={index}>
  //           <td>
  //             <a href={item.txDetailsUrl} onClick={(e) => onLinkClick(e)}>{item.txHash}</a>
  //           </td>
  //         </tr>)
  //       })
  //     }
  //   </tbody>
  // </table>

// changeNetwork
// changeNodeUrl
// resetNodeUrl
// copyPrivateKeyToClipboard
// copyMnemonicToClipboard

// <select value={currentNetworkIx} name="network" onChange={(e) => changeNetwork(e)}>
//   <option value="0">{REST_SERVICES[0].name}</option>
//   <option value="1">{REST_SERVICES[1].name}</option>
//   <option value="2">{REST_SERVICES[2].name}</option>
// </select>
// <td className="white_on_purple_with_hover h20px fake_button" onClick={(e) => showChangeNode()}>
//   <div className="tooltip">Change Node<span className="tooltiptext">{restService}</span>
//   </div>
// </td>
// <div className="gray_on_white">Ledger Status</div>
// <p><LedgerMessage/>
// </p>
// <UseLedgerButton/>
//
// vote
//   parsedProducerList
// parsedProducerList.totalvotes}</span>
// {parsedProducerList.totalcounts}</span>
// {parsedProducerList.producersCandidateCount}</span>
// parsedProducerList.producers.length
// vote
// <table className="w100pct black_on_offwhite no_border whitespace_nowrap">
//   <tbody>
//     <tr>
//       <td className="no_border no_padding">N</td>
//       <td className="no_border no_padding">Nickname</td>
//       <td className="no_border no_padding">Active</td>
//       <td className="no_border no_padding">Votes</td>
//       <td className="no_border no_padding">Select</td>
//     </tr>
//     {
//       parsedProducerList.producers.map((item, index) => {
//         return (<tr key={index}>
//           <td className="no_border no_padding">{item.n}</td>
//           <td className="no_border no_padding">{item.nickname}</td>
//           <td className="no_border no_padding">{item.active}</td>
//           <td className="no_border no_padding">{item.votes}</td>
//           <td className="white_on_purple_with_hover h20px fake_button" onClick={(e) => toggleProducerSelection({index})}>
//             <ProducerSelectionButtonText item={item}/>
//           </td>
//         </tr>)
//       })
//     }
//   </tbody>
// </table>
// candidateVoteListStatus
// parsedCandidateVoteList.candidateVotes.length
// <table className="w100pct black_on_offwhite no_border whitespace_nowrap">
//   <tbody>
//     <tr>
//       <td className="no_border no_padding">N</td>
//       <td className="no_border no_padding">Nickname</td>
//       <td className="no_border no_padding">Votes</td>
//     </tr>
//     {
//       parsedCandidateVoteList.candidateVotes.map((item, index) => {
//         return (<tr key={index}>
//           <td className="no_border no_padding">{item.n}</td>
//           <td className="no_border no_padding">{item.nickname}</td>
//           <td className="no_border no_padding">{item.votes}</td>
//         </tr>)
//       })
//     }
//   </tbody>
// </table>

class App extends React.Component {
  render() {
    return (<div className="display_inline_block ta_center va_top body">
      <Home app={app} openDevTools={openDevTools} guiToggles={guiToggles}/>
      <Landing app={app} openDevTools={openDevTools} guiToggles={guiToggles}/>
      <LoginMnemonic app={app} openDevTools={openDevTools} guiToggles={guiToggles}/>
      <LoginPrivateKey app={app} openDevTools={openDevTools} guiToggles={guiToggles}/>
      <Temp app={app} openDevTools={openDevTools} guiToggles={guiToggles}/>
    </div>)
  }
}
const renderApp = () => {
  ReactDOM.render(<App/>, document.getElementById('main'));
};

const onLoad = () => {
  app.init();
  guiToggles.init(app);
  app.setAppClipboard(clipboard);
  app.setAppDocument(document);
  app.setRenderApp(renderApp);
  renderApp();
  guiToggles.showLanding();
  app.setPollForAllInfoTimer();
}

/** call initialization functions */
window.onload = onLoad;


renderApp();
