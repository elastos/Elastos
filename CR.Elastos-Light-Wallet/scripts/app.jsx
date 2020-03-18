"use strict";

/** imports */
const React = require('react');
const ReactDOM = require('react-dom');

const electron = require('electron');
const shell = electron.shell;
const remote = electron.remote;
const clipboard = electron.clipboard;

const Row = require('react-bootstrap').Row;
const Col = require('react-bootstrap').Col;
const Grid = require('react-bootstrap').Grid;
const Table = require('react-bootstrap').Table;

const BigNumber = require('bignumber.js');

const crypto = require('crypto');

/** modules */

const LedgerComm = require('./LedgerComm.js');
const AddressTranscoder = require('./AddressTranscoder.js')
const KeyTranscoder = require('./KeyTranscoder.js')
const TxTranscoder = require('./TxTranscoder.js')
const TxSigner = require('./TxSigner.js')
const Asset = require('./Asset.js')
const TxFactory = require('./TxFactory.js')

/** global constants */

const LOG_LEDGER_POLLING = true;

const MAX_POLL_DATA_TYPE_IX = 4;

const PRIVATE_KEY_LENGTH = 64;

const DEFAULT_FEE_SATS = '500';

/** networks */
const EXPLORER = 'https://blockchain.elastos.org';

const REST_SERVICES =  [
  {
    name:'api-wallet',
    url:'https://api-wallet-ela.elastos.org'
  },
  {
    name:'node1',
    url:'https://node1.elaphant.app'
  },
  {
    name:'node3',
    url:'https://node3.elaphant.app'
  }
]
var currentNetworkIx = 0;

var restService;

const getRestService = () => {
  return restService;
}

const setRestService = (ix) => {
  currentNetworkIx = ix;
  restService = REST_SERVICES[ix].url;
  get('nodeUrl').value = restService;
};

const getTransactionHistoryUrl = (address) => {
  const url = `${EXPLORER}/api/v1/txs/?pageNum=0&address=${address}`;
  // mainConsole.log('getTransactionHistoryUrl',url);
  return url;
}

const getTransactionHistoryLink = (txid) => {
  const url = `${EXPLORER}/tx/${txid}`;
  // mainConsole.log('getTransactionHistoryLink',url);
  return url;
}

const getBalanceUrl = (address) => {
  const url = `${getRestService()}/api/v1/asset/balances/${address}`;
  // mainConsole.log('getBalanceUrl',url);
  return url;
}

const getUnspentTransactionOutputsUrl = (address) => {
  const url = `${getRestService()}/api/v1/asset/utxo/${address}/${Asset.elaAssetId}`;
  // mainConsole.log('getUnspentTransactionOutputsUrl',url);
  return url;
}

const getStateUrl = () => {
  const url = `${getRestService()}/api/v1/node/state`;
  // mainConsole.log('getStateUrl',url);
  return url;
}

/** global variables */

var ledgerDeviceInfo = undefined;

var publicKey = undefined;

var address = undefined;

var pollDataTypeIx = 0;

var balance = undefined;

var sendAmount = '';

var feeAmountSats = '';

var feeAmountEla = '';

var isLoggedIn = false;

var useLedgerFlag = false;

var generatedPrivateKeyHex = undefined;

const sendToAddressStatuses = [];
sendToAddressStatuses.push('No Send-To Transaction Requested Yet');
const sendToAddressLinks = [];

var balanceStatus = 'No Balance Requested Yet';

var transactionHistoryStatus = 'No History Requested Yet';

const parsedTransactionHistory = [];

var producerListStatus = 'No Producers Requested Yet';

var parsedProducerList = {};
parsedProducerList.totalvotes = '-';
parsedProducerList.totalcounts = '-';
parsedProducerList.producersCandidateCount = 0;
parsedProducerList.producers = [];

var candidateVoteListStatus = 'No Candidate Votes Requested Yet';

var parsedCandidateVoteList = {};
parsedCandidateVoteList.candidateVotes = [];

var unspentTransactionOutputsStatus = 'No UTXOs Requested Yet';

const parsedUnspentTransactionOutputs = [];

var blockchainStatus = 'No Blockchain State Requested Yet';

var blockchainState = {};

var blockchainLastActionHeight = 0;

/** functions */
const formatDate = (date) => {
  let month = (date.getMonth() + 1).toString();
  let day = date.getDate().toString();
  const year = date.getFullYear();

  if (month.length < 2) {
    month = '0' + month
  };
  if (day.length < 2) {
    day = '0' + day
  };

  return [year, month, day].join('-');
}

const changeNetwork = (event) => {
  currentNetworkIx = event.target.value;
  setRestService(currentNetworkIx);
  refreshBlockchainData();
}

const resetNodeUrl = () => {
  setRestService(currentNetworkIx);
  renderApp();
}

const changeNodeUrl = () => {
  restService = get('nodeUrl').value;
  showLogin();
}

const refreshBlockchainData = () => {
  requestTransactionHistory();
  requestBalance();
  requestUnspentTransactionOutputs();
  requestBlockchainState();
  renderApp();
}

const publicKeyCallback = (message) => {
  if (LOG_LEDGER_POLLING) {
    mainConsole.log(`publicKeyCallback ${JSON.stringify(message)}`);
  }
  if (message.success) {
    publicKey = message.publicKey;
    requestBlockchainDataAndShowHome();
  } else {
    ledgerDeviceInfo.error = true;
    ledgerDeviceInfo.message = message.message;
    renderApp();
  }
}

const pollForDataCallback = (message) => {
  if (LOG_LEDGER_POLLING) {
    mainConsole.log(`pollForDataCallback ${JSON.stringify(message)}`);
  }
  ledgerDeviceInfo = message;
  renderApp();
  pollDataTypeIx++;
  setPollForAllInfoTimer();
}

var mainConsoleLib = require('console');
var mainConsole = new mainConsoleLib.Console(process.stdout, process.stderr);
mainConsole.log('Consone Logging Enabled.');

const pollForData = () => {
  if (LOG_LEDGER_POLLING) {
    mainConsole.log('getAllLedgerInfo ' + pollDataTypeIx);
  }
  var resetPollIndex = false;
  switch (pollDataTypeIx) {
    case 0:
      pollForDataCallback('Polling...');
      break;
    case 1:
      LedgerComm.getLedgerDeviceInfo(pollForDataCallback);
      break;
    case 2:
      if (useLedgerFlag) {
        LedgerComm.getPublicKey(publicKeyCallback);
      }
      break;
    case 3:
      if (address != undefined) {
        requestTransactionHistory();
        requestBalance();
        requestUnspentTransactionOutputs();
        requestBlockchainState();
      }
    case 4:
      requestListOfProducers();
    case MAX_POLL_DATA_TYPE_IX:
      // only check every 10 seconds for a change in device status.
      pollDataTypeIx = 0;
      setTimeout(pollForData, 10000);
      break;
    default:
      throw Error('poll data index reset failed.');
  }
};

const setPollForAllInfoTimer = () => {
  setTimeout(pollForData, 1);
}

const postJson = (url, jsonString, readyCallback, errorCallback) => {
  var xmlhttp = new XMLHttpRequest(); // new HttpRequest instance

  const xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4) {
      // sendToAddressStatuses.push( `XMLHttpRequest: status:${this.status} response:'${this.response}'` );
      if (this.status == 200) {
        readyCallback(JSON.parse(this.response));
      } else {
        errorCallback(this.response);
      }
    }
  }
  xhttp.responseType = 'text';
  xhttp.open('POST', url, true);
  xhttp.setRequestHeader('Content-Type', 'application/json');
  xhttp.setRequestHeader('Authorization', 'Basic RWxhVXNlcjpFbGExMjM=');

  // sendToAddressStatuses.push( `XMLHttpRequest: curl ${url} -H "Content-Type: application/json" -d '${jsonString}'` );

  xhttp.send(jsonString);
}

const getJson = (url, readyCallback, errorCallback) => {
  const xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4) {
      if (this.status == 200) {
        readyCallback(JSON.parse(this.response));
      } else {
        errorCallback({'status': this.status, 'statusText': this.statusText, 'response': this.response});
      }
    }
  }
  xhttp.responseType = 'text';
  xhttp.open('GET', url, true);
  xhttp.send();
}

const requestUnspentTransactionOutputs = () => {
  unspentTransactionOutputsStatus = 'UTXOs Requested';
  const unspentTransactionOutputsUrl = getUnspentTransactionOutputsUrl(address);

  // mainConsole.log( 'unspentTransactionOutputsUrl ' + unspentTransactionOutputsUrl );

  getJson(unspentTransactionOutputsUrl, getUnspentTransactionOutputsReadyCallback, getUnspentTransactionOutputsErrorCallback);
};

const getUnspentTransactionOutputsErrorCallback = (response) => {
  unspentTransactionOutputsStatus = `UTXOs Error ${JSON.stringify(response)}`;

  renderApp();
}

const getUnspentTransactionOutputsReadyCallback = (response) => {
  unspentTransactionOutputsStatus = 'UTXOs Received';
  parsedUnspentTransactionOutputs.length = 0;

  mainConsole.log('getUnspentTransactionOutputsCallback ' + JSON.stringify(response), response.Result);

  if ((response.Result != undefined) && (response.Result != null) && (response.Error == 0)) {
    response.Result.forEach((utxo, utxoIx) => {
      TxFactory.updateValueSats(utxo, utxoIx);
      parsedUnspentTransactionOutputs.push(utxo);
    });
  }

  renderApp();
}

const get = (id) => {
  const elt = document.getElementById(id);
  if (elt == null) {
    throw new Error('elt is null:' + id);
  }
  return elt;
}

const hide = (id) => {
  get(id).style = 'display:none;';
}

const show = (id) => {
  get(id).style = 'display:default;';
}

const getPublicKeyFromLedger = () => {
  useLedgerFlag = true;
  isLoggedIn = true;
  LedgerComm.getPublicKey(publicKeyCallback);
}

const requestBlockchainDataAndShowHome = () => {
  if (publicKey === undefined) {
    return;
  }
  address = AddressTranscoder.getAddressFromPublicKey(publicKey);
  requestTransactionHistory();
  requestBalance();
  requestUnspentTransactionOutputs();
  requestBlockchainState();
  showHome();
}

const getPublicKeyFromPrivateKey = () => {
  useLedgerFlag = false;
  isLoggedIn = true;
  show('privateKey');
  const privateKeyElt = document.getElementById('privateKey');
  const privateKey = privateKeyElt.value;
  if (privateKey.length != PRIVATE_KEY_LENGTH) {
    alert(`private key must be a hex encoded string of length ${PRIVATE_KEY_LENGTH}, not ${privateKey.length}`);
    return;
  }
  publicKey = KeyTranscoder.getPublic(privateKey);
  requestBlockchainDataAndShowHome();
}

const sendAmountToAddressErrorCallback = (error) => {
  sendToAddressStatuses.push(JSON.stringify(error));
  renderApp();
}

const sendAmountToAddressReadyCallback = (transactionJson) => {
  mainConsole.log('sendAmountToAddressReadyCallback ' + JSON.stringify(transactionJson));
  if (transactionJson.Error) {
    sendToAddressStatuses.push(`${transactionJson.Error} ${transactionJson.Result}`);
  } else {
    sendToAddressStatuses.length = 0;
    const link = getTransactionHistoryLink(transactionJson.result);
    const elt = {};
    elt.txDetailsUrl = link;
    elt.txHash = transactionJson.result;
    sendToAddressStatuses.length = 0;
    sendToAddressStatuses.push('Transaction Successful.');
    sendToAddressLinks.push(elt);
  }
  showCompletedTransaction();
  setBlockchainLastActionHeight();
  renderApp();
}

const clearSendData = () => {
  mainConsole.log('STARTED clearSendData');
  const sendAmountElt = document.getElementById('sendAmount');
  const sendToAddressElt = document.getElementById('sendToAddress');
  const feeAmountElt = document.getElementById('feeAmount');
  sendAmountElt.value = '';
  sendToAddressElt.value = '';
  feeAmountElt.value = DEFAULT_FEE_SATS;
  sendAmount = '';
  feeAmountSats = '';
  feeAmountEla = '';
  sendToAddressStatuses.length = 0;
  sendToAddressLinks.length = 0;
  mainConsole.log('SUCCESS clearSendData');
}

const updateAmountAndFees = () => {
  const sendAmountElt = document.getElementById('sendAmount');
  const feeAmountElt = document.getElementById('feeAmount');

  sendAmount = sendAmountElt.value;
  feeAmountSats = feeAmountElt.value;
  if (Number.isNaN(sendAmount)) {
    throw new Error(`sendAmount ${sendAmount} is not a number`);
  }
  if (Number.isNaN(feeAmountSats)) {
    throw new Error(`feeAmountSats ${feeAmountSats} is not a number`);
  }
  feeAmountEla = BigNumber(feeAmountSats, 10).dividedBy(Asset.satoshis).toString();
}

const updateAmountAndFeesAndRenderApp = () => {
  updateAmountAndFees();
  renderApp();
}

const sendAmountToAddress = () => {
  updateAmountAndFees();

  const sendToAddressElt = document.getElementById('sendToAddress');

  const sendToAddress = sendToAddressElt.value;

  const unspentTransactionOutputs = parsedUnspentTransactionOutputs;
  mainConsole.log('sendAmountToAddress.unspentTransactionOutputs ' + JSON.stringify(unspentTransactionOutputs));

  if (Number.isNaN(sendAmount)) {
    throw new Error(`sendAmount ${sendAmount} is not a number`);
  }
  if (Number.isNaN(feeAmountSats)) {
    throw new Error(`feeAmountSats ${feeAmountSats} is not a number`);
  }

  var encodedTx;

  if (useLedgerFlag) {
    const tx = TxFactory.createUnsignedSendToTx(unspentTransactionOutputs, sendToAddress, sendAmount, publicKey, feeAmountSats);
    const encodedUnsignedTx = TxTranscoder.encodeTx(tx, false);
    const sendAmountToAddressLedgerCallback = (message) => {
      if (LOG_LEDGER_POLLING) {
        mainConsole.log(`sendAmountToAddressLedgerCallback ${JSON.stringify(message)}`);
      }
      if (!message.success) {
        sendToAddressStatuses.length = 0;
        sendToAddressLinks.length = 0;
        sendToAddressStatuses.push(JSON.stringify(message));
        renderApp();
        return;
      }
      const signature = Buffer.from(message.signature, 'hex');
      const encodedTx = TxSigner.addSignatureToTx(tx, publicKey, signature);
      sendAmountToAddressCallback(encodedTx);
    }
    LedgerComm.sign(encodedUnsignedTx, sendAmountToAddressLedgerCallback);
  } else {
    const privateKeyElt = document.getElementById('privateKey');
    const privateKey = privateKeyElt.value;
    const encodedTx = TxFactory.createSignedSendToTx(privateKey, unspentTransactionOutputs, sendToAddress, sendAmount, feeAmountSats);

    if (encodedTx == undefined) {
      return;
    }
    sendAmountToAddressCallback(encodedTx);
  }
  renderApp();
}
// success: success,
// message: lastResponse,
// signature: signature

// https://walletservice.readthedocs.io/en/latest/api_guide.html#post--api-1-sendRawTx
const sendAmountToAddressCallback = (encodedTx) => {
  const txUrl = `${getRestService()}/api/v1/sendRawTx`;

  const jsonString = `{"data": "${encodedTx}"}`;

  mainConsole.log('sendAmountToAddress.txUrl ' + txUrl);
  mainConsole.log('sendAmountToAddress.encodedTx ' + JSON.stringify(encodedTx));

  const decodedTx = TxTranscoder.decodeTx(Buffer.from(encodedTx, 'hex'), true);

  mainConsole.log('sendAmountToAddress.decodedTx ' + JSON.stringify(decodedTx));

  sendToAddressStatuses.length = 0;
  sendToAddressLinks.length = 0;
  sendToAddressStatuses.push(JSON.stringify(encodedTx));
  sendToAddressStatuses.push(JSON.stringify(decodedTx));
  sendToAddressStatuses.push(`Transaction Requested: curl ${txUrl} -H "Content-Type: application/json" -d '${jsonString}'`);
  renderApp();
  postJson(txUrl, jsonString, sendAmountToAddressReadyCallback, sendAmountToAddressErrorCallback);
}

const requestListOfProducersErrorCallback = (response) => {
  producerListStatus = `Producers Error: ${JSON.stringify(response)}`;
  renderApp();
}

const requestListOfProducersReadyCallback = (response) => {
  producerListStatus = 'Producers Received';

  // mainConsole.log('STARTED Producers Callback', response);
  parsedProducerList = {};
  parsedProducerList.producersCandidateCount = 0;
  parsedProducerList.producers = [];
  if (response.status !== 200) {
    producerListStatus = `Producers Error: ${JSON.stringify(response)}`;
  } else {
    parsedProducerList.totalvotes = 0;
    parsedProducerList.totalcounts = 0;
    response.result.forEach((producer) => {
      const parsedProducer = {};
      // mainConsole.log('INTERIM Producers Callback', producer);
      parsedProducer.n = parsedProducerList.producers.length + 1;
      parsedProducer.nickname = producer.Nickname;
      parsedProducer.active = producer.Active.toString();
      parsedProducer.votes = producer.Votes;
      parsedProducer.ownerpublickey = producer.Ownerpublickey;
      parsedProducer.isCandidate = false;
      // mainConsole.log('INTERIM Producers Callback', parsedProducer);
      parsedProducerList.producers.push(parsedProducer);
    });
    // mainConsole.log('INTERIM Producers Callback', response.result.producers[0]);
  }
  // mainConsole.log('SUCCESS Producers Callback');

  renderApp();
}

const requestListOfProducers = () => {
  producerListStatus = 'Producers Requested';
  const txUrl = `${getRestService()}/api/v1/dpos/rank/height/0`

  renderApp();
  getJson(txUrl, requestListOfProducersReadyCallback, requestListOfProducersErrorCallback);
}

const toggleProducerSelection = (item) => {
  // mainConsole.log('INTERIM toggleProducerSelection item', item);
  const index = item.index;
  // mainConsole.log('INTERIM toggleProducerSelection index', index);
  // mainConsole.log('INTERIM toggleProducerSelection length', parsedProducerList.producers.length);
  const parsedProducer = parsedProducerList.producers[index];
  // mainConsole.log('INTERIM[1] toggleProducerSelection parsedProducer', parsedProducer);
  // mainConsole.log('INTERIM[1] toggleProducerSelection isCandidate', parsedProducer.isCandidate);
  parsedProducer.isCandidate = !parsedProducer.isCandidate;
  // mainConsole.log('INTERIM[2] toggleProducerSelection isCandidate', parsedProducer.isCandidate);

  parsedProducerList.producersCandidateCount = 0;
  parsedProducerList.producers.forEach((parsedProducerElt) => {
    if(parsedProducerElt.isCandidate) {
      parsedProducerList.producersCandidateCount++;
    }
  })

  renderApp();
}

const requestListOfCandidateVotesErrorCallback = (response) => {
  let displayRawError = true;
  if(displayRawError) {
    candidateVoteListStatus = `Candidate Votes Error: ${JSON.stringify(response)}`;
  }
  renderApp();
}

const requestListOfCandidateVotesReadyCallback = (response) => {
  candidateVoteListStatus = 'Candidate Votes Received';

  mainConsole.log('STARTED Candidate Votes Callback', response);
  parsedCandidateVoteList = {};
  parsedCandidateVoteList.candidateVotes = [];
  if (response.status !== 200) {
    candidateVoteListStatus = `Candidate Votes Error: ${JSON.stringify(response)}`;
  } else {
    response.result.forEach((candidateVote) => {
       mainConsole.log('INTERIM Candidate Votes Callback', candidateVote);
     const body = candidateVote.Vote_Body;
     body.forEach((candidateVoteElt) => {
       const parsedCandidateVote = {};
        parsedCandidateVote.n = parsedCandidateVoteList.candidateVotes.length + 1;
        parsedCandidateVote.nickname = candidateVoteElt.Nickname;
        parsedCandidateVote.active = candidateVoteElt.Active.toString();
        parsedCandidateVote.votes = candidateVoteElt.Votes;
        parsedCandidateVote.ownerpublickey = candidateVoteElt.Ownerpublickey;
        mainConsole.log('INTERIM Candidate Votes Callback', parsedCandidateVote);
        parsedCandidateVoteList.candidateVotes.push(parsedCandidateVote);
     })
    });
    mainConsole.log('INTERIM Candidate Votes Callback', response.result);
  }
  mainConsole.log('SUCCESS Candidate Votes Callback');

  renderApp();
}

const requestListOfCandidateVotes = () => {
  candidateVoteListStatus = 'Candidate Votes Requested';

  const txUrl = `${getRestService()}/api/v1/dpos/address/${address}?pageSize=5000&pageNum=1`;

  renderApp();
  getJson(txUrl, requestListOfCandidateVotesReadyCallback, requestListOfCandidateVotesErrorCallback);
}

const sendVoteTx = () => {
  updateAmountAndFees();
  const unspentTransactionOutputs = parsedUnspentTransactionOutputs;
  mainConsole.log('sendVoteTx.unspentTransactionOutputs ' + JSON.stringify(unspentTransactionOutputs));

  if (Number.isNaN(feeAmountSats)) {
    throw new Error(`feeAmountSats ${feeAmountSats} is not a number`);
  }

  const candidates = [];
  parsedProducerList.producers.forEach((parsedProducer) => {
    if (parsedProducer.isCandidate) {
      candidates.push(parsedProducer.ownerpublickey);
    }
  })

  mainConsole.log('sendVoteTx.candidates ' + JSON.stringify(candidates));

  var encodedTx;

  mainConsole.log('sendVoteTx.useLedgerFlag ' + JSON.stringify(useLedgerFlag));
  mainConsole.log('sendVoteTx.unspentTransactionOutputs ' + JSON.stringify(unspentTransactionOutputs));
  candidateVoteListStatus = `Voting for ${parsedProducerList.producersCandidateCount} candidates.`;
  if (useLedgerFlag) {
    if(unspentTransactionOutputs) {
      const tx = TxFactory.createUnsignedVoteTx(unspentTransactionOutputs, publicKey, feeAmountSats, candidates);
      const encodedUnsignedTx = TxTranscoder.encodeTx(tx, false);
      const sendVoteLedgerCallback = (message) => {
        if (LOG_LEDGER_POLLING) {
          mainConsole.log(`sendVoteLedgerCallback ${JSON.stringify(message)}`);
        }
        if (!message.success) {
          // sendToAddressStatuses.length = 0;
          // sendToAddressLinks.length = 0;
          // sendToAddressStatuses.push(JSON.stringify(message));
          renderApp();
          return;
        }
        const signature = Buffer.from(message.signature, 'hex');
        const encodedTx = TxSigner.addSignatureToTx(tx, publicKey, signature);
        sendVoteCallback(encodedTx);
      }
      candidateVoteListStatus += ' please confirm tx on ledger.';
      LedgerComm.sign(encodedUnsignedTx, sendVoteLedgerCallback);
    } else {
      alert('please wait, UTXOs have not been retrieved yet.');
    }
  } else {
    const privateKeyElt = document.getElementById('privateKey');
    const privateKey = privateKeyElt.value;
    const encodedTx = TxFactory.createSignedVoteTx(privateKey, unspentTransactionOutputs, feeAmountSats, candidates);

    mainConsole.log('sendVoteTx.encodedTx ' + JSON.stringify(encodedTx));

    if (encodedTx == undefined) {
      return;
    }
    sendVoteCallback(encodedTx);
  }
  renderApp();
}
// success: success,
// message: lastResponse,
// signature: signature

const sendVoteCallback = (encodedTx) => {
  const txUrl = `${getRestService()}/api/v1/sendRawTx`;

  const jsonString = `{"data": "${encodedTx}"}`;

  mainConsole.log('sendVoteCallback.encodedTx ' + JSON.stringify(encodedTx));

  const decodedTx = TxTranscoder.decodeTx(Buffer.from(encodedTx, 'hex'), true);

  mainConsole.log('sendVoteCallback.decodedTx ' + JSON.stringify(decodedTx));

  // sendToAddressStatuses.length = 0;
  // sendToAddressLinks.length = 0;
  // sendToAddressStatuses.push(JSON.stringify(encodedTx));
  // sendToAddressStatuses.push(JSON.stringify(decodedTx));
  // sendToAddressStatuses.push(`Transaction Requested: curl ${txUrl} -H "Content-Type: application/json" -d '${jsonString}'`);
  renderApp();
  postJson(txUrl, jsonString, sendVoteReadyCallback, senVoteErrorCallback);
}

const senVoteErrorCallback = (error) => {
  mainConsole.log('senVoteErrorCallback ' + JSON.stringify(error));
  // sendToAddressStatuses.push(JSON.stringify(error));
  renderApp();
}

const sendVoteReadyCallback = (transactionJson) => {
  mainConsole.log('sendVoteReadyCallback ' + JSON.stringify(transactionJson));
  if (transactionJson.Error) {
    candidateVoteListStatus(`Vote Error: ${transactionJson.Error} ${transactionJson.Result}`);
  } else {
    candidateVoteListStatus = `Vote Success TX: ${transactionJson.result}`;
  }
  renderApp();
}

const getTransactionHistoryErrorCallback = (response) => {
  transactionHistoryStatus = `History Error: ${JSON.stringify(response)}`;
  renderApp();
}

const getTransactionHistoryReadyCallback = (transactionHistory) => {
  transactionHistoryStatus = 'History Received';
  parsedTransactionHistory.length = 0;
  transactionHistory.txs.forEach((tx, txIx) => {
    const time = formatDate(new Date(tx.time * 1000));
    // tx.vin.forEach((vinElt) => {
    //   const parsedTransaction = {};
    //   parsedTransaction.n = txIx;
    //   parsedTransaction.type = 'input';
    //   parsedTransaction.value = vinElt.value;
    //   parsedTransaction.valueSat = vinElt.valueSat;
    //   parsedTransaction.address = vinElt.addr;
    //   parsedTransaction.txHash = tx.txid;
    //   parsedTransaction.txDetailsUrl = getTransactionHistoryLink(tx.txid);
    //   parsedTransaction.time = time;
    //   parsedTransactionHistory.push(parsedTransaction);
    // });
    tx.vout.forEach((voutElt) => {
      voutElt.scriptPubKey.addresses.forEach((voutAddress) => {
        const parsedTransaction = {};
        parsedTransaction.n = txIx;
        if(voutAddress == address) {
          parsedTransaction.type = 'input';
        } else {
          parsedTransaction.type = 'output';
        }
        parsedTransaction.value = voutElt.value;
        parsedTransaction.valueSat = voutElt.valueSat;
        parsedTransaction.address = voutAddress;
        parsedTransaction.txHash = tx.txid;
        parsedTransaction.txDetailsUrl = getTransactionHistoryLink(tx.txid);
        parsedTransaction.time = time;
        parsedTransactionHistory.push(parsedTransaction);
      });
    });
  });

  renderApp();
}

const requestTransactionHistory = () => {
  transactionHistoryStatus = 'History Requested';
  const transactionHistoryUrl = getTransactionHistoryUrl(address);
  //mainConsole.log('requestTransactionHistory ' + transactionHistoryUrl);
  getJson(transactionHistoryUrl, getTransactionHistoryReadyCallback, getTransactionHistoryErrorCallback);
};

const getBalanceErrorCallback = (response) => {
  balanceStatus = `Balance Error:${JSON.stringify(response)} `;
}

const getBalanceReadyCallback = (balanceResponse) => {
  if(balanceResponse.Error == 0) {
    balanceStatus = `Balance Received.`;
  } else {
    balanceStatus = `Balance Received Error:${balanceResponse.Error}`;
  }
  balance = balanceResponse.Result;

  renderApp();
}

const requestBalance = () => {
  const balanceUrl = getBalanceUrl(address);
  balanceStatus = `Balance Requested ${balanceUrl}`;
  getJson(balanceUrl, getBalanceReadyCallback, getBalanceErrorCallback);
};

const getBlockchainStateErrorCallback = (response) => {
  balanceStatus = `Blockchain State Error:${JSON.stringify(response)} `;
}

const getBlockchainStateReadyCallback = (blockchainStateResponse) => {
  blockchainStatus = `Blockchain State Received:${blockchainStateResponse.Desc} ${blockchainStateResponse.Error} `;
  blockchainState = blockchainStateResponse.Result;

  renderApp();
}

const requestBlockchainState = () => {
  const stateUrl = getStateUrl();
  blockchainState = {};
  blockchainStatus = `Blockchain State Requested ${stateUrl}`;
  getJson(stateUrl, getBlockchainStateReadyCallback, getBlockchainStateErrorCallback);
};

const getConfirmations = () => {
  if(blockchainState.height) {
    return blockchainState.height - blockchainLastActionHeight;
  } else {
    return 0;
  }
}

const setBlockchainLastActionHeight = () => {
  if(blockchainState.height) {
    blockchainLastActionHeight = blockchainState.height;
  }
}

const removeClass = (id, cl) => {
  get(id).classList.remove(cl);
}

const addClass = (id, cl) => {
  get(id).classList.add(cl);
}

const selectButton = (id) => {
  addClass(id, 'white_on_light_purple');
  removeClass(id, 'white_on_purple_with_hover');
}

const clearButtonSelection = (id) => {
  removeClass(id, 'white_on_light_purple');
  addClass(id, 'white_on_purple_with_hover');
}

const hideEverything = () => {
  clearButtonSelection('send');
  clearButtonSelection('home');
  clearButtonSelection('receive');
  clearButtonSelection('transactions');
  clearButtonSelection('voting');
  hide('change-node');
  hide('private-key-entry');
  hide('cancel-confirm-transaction');
  hide('completed-transaction');
  hide('fees');
  hide('confirm-and-see-fees');
  hide('to-address');
  hide('send-amount');
  hide('from-address');
  hide('balance');
  hide('transaction-more-info');
  hide('transaction-list-small');
  hide('transaction-list-large');
  hide('your-address');
  hide('private-key-login');
  hide('ledger-login');
  hide('elastos-branding');
  hide('send-spacer-01');
  hide('private-key-generate');
  hide('private-key-generator');
  hide('candidate-list');
  hide('candidate-vote-button');
  hide('candidate-vote-list');
}

const openDevTools = () => {
  try {
    const window = remote.getCurrentWindow();
    window.webContents.openDevTools();
  } catch (e) {
    alert(`error:${e}`)
  }
}

const copyToClipboard = () => {
  clipboard.writeText(generatedPrivateKeyHex);
  alert(`copied to clipboard:\n${generatedPrivateKeyHex}`)
}

const showChangeNode = () => {
  clearGlobalData();
  hideEverything();
  clearSendData();
  show('change-node');
}

const showLogin = () => {
  clearGlobalData();
  hideEverything();
  clearSendData();
  show('private-key-login');
  show('ledger-login');
  show('elastos-branding');
  show('private-key-generate');
}

const showHome = () => {
  if (!isLoggedIn) {
    return;
  }
  hideEverything();
  clearSendData();
  show('transaction-more-info');
  show('transaction-list-small');
  show('your-address');
  show('elastos-branding');
  selectButton('home');
}

const showSend = () => {
  if (!isLoggedIn) {
    return;
  }
  hideEverything();
  clearSendData();
  show('from-address');
  show('balance');
  show('send-amount');
  show('to-address');
  show('confirm-and-see-fees');
  selectButton('send');
}

const cancelSend = () => {
  const sendToAddressElt = document.getElementById('sendToAddress');
  const sendAmountElt = document.getElementById('sendAmount');
  const feeAmountElt = document.getElementById('feeAmount');

  sendToAddressElt.value = '';
  sendAmountElt.value = '';
  feeAmountElt.value = DEFAULT_FEE_SATS;

  sendAmount = '';
  feeAmountSats = '';
  feeAmountEla = '';

  showSend();
}

const showConfirmAndSeeFees = () => {
  hideEverything();
  show('fees');
  show('cancel-confirm-transaction');
  show('send-spacer-01');
  selectButton('send');
  updateAmountAndFeesAndRenderApp();
}

const showCompletedTransaction = () => {
  hideEverything();
  show('fees');
  show('completed-transaction');
  show('send-spacer-01');
  selectButton('send');
  updateAmountAndFeesAndRenderApp();
}

const showReceive = () => {
  if (!isLoggedIn) {
    return;
  }
  hideEverything();
  clearSendData();
  show('your-address');
  selectButton('receive');
}

const showTransactions = () => {
  if (!isLoggedIn) {
    return;
  }
  hideEverything();
  clearSendData();
  show('transaction-more-info');
  show('transaction-list-large');
  selectButton('transactions');
}

const showVoting = () => {
  if (!isLoggedIn) {
    return;
  }
  hideEverything();
  clearSendData();
  requestListOfProducers();
  requestListOfCandidateVotes();
  show('candidate-list');
  show('candidate-vote-button');
  show('candidate-vote-list');
  selectButton('voting');
}

const showPrivateKeyEntry = () => {
  hideEverything();
  clearSendData();
  show('private-key-entry');
}

const showGenerateNewPrivateKey = () => {
  hideEverything();
  clearSendData();
  show('private-key-generator');
  generatedPrivateKeyHex = crypto.randomBytes(32).toString('hex');
  renderApp();
}

const clearGlobalData = () => {
  get('privateKey').value = '';
  get('feeAmount').value = DEFAULT_FEE_SATS;
  get('nodeUrl').value = '';

  useLedgerFlag = false;
  publicKey = undefined;
  address = undefined;
  balance = undefined;
  generatedPrivateKeyHex = undefined;

  sendAmount = '';
  feeAmountSats = '';
  feeAmountEla = '';

  sendToAddressStatuses.length = 0;
  sendToAddressLinks.length = 0;
  sendToAddressStatuses.push('No Send-To Transaction Requested Yet');

  balanceStatus = 'No Balance Requested Yet';

  transactionHistoryStatus = 'No History Requested Yet';
  parsedTransactionHistory.length = 0;

  unspentTransactionOutputsStatus = 'No UTXOs Requested Yet';
  parsedUnspentTransactionOutputs.length = 0;
  renderApp();
}

const Version = () => {
  return remote.app.getVersion();
}

const LedgerMessage = () => {
  let message = '';
  if (LOG_LEDGER_POLLING) {
    mainConsole.log('LedgerMessage', ledgerDeviceInfo);
  }
  if (ledgerDeviceInfo) {
    if (ledgerDeviceInfo.error) {
      message += 'Error:';
      if (ledgerDeviceInfo.message) {
        message += ledgerDeviceInfo.message;
      }
    } else {
      if (ledgerDeviceInfo.message) {
        message += ledgerDeviceInfo.message;
      }
    }
  }
  return message;
}

const UseLedgerButton = () => {
  if (
    ledgerDeviceInfo
    ? ledgerDeviceInfo.enabled
    : false) {
    return (<div className="white_on_gray bordered display_inline_block float_right fake_button rounded padding_5px" onClick={(e) => getPublicKeyFromLedger()}>Use Ledger</div>);
  } else {
    return (<div className="white_on_pink bordered display_inline_block float_right fake_button rounded padding_5px">Use Ledger</div>);
  }
  return (<div/>);
}

const TransactionHistoryElementIcon = (props) => {
  const item = props.item;
  if (item.type == 'input') {
    return (<img src="artwork/received-ela.svg"/>);
  }
  if (item.type == 'output') {
    return (<img src="artwork/sent-ela.svg"/>);
  }
  return (<div/>);
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

const onLinkClick = (event) => {
  event.preventDefault();
  shell.openExternal(event.currentTarget.href);
}

class App extends React.Component {
  render() {
    return (<div>
      <table className="w800h600px no_padding no_border">
        <tbody>
          <tr className="no_padding">
            <td className="valign_top white_on_purple no_border" style={{
                width: '150px'
              }}>
              <table className="w100pct no_border">
                <tbody>
                  <tr>
                    <td className="black_on_offwhite h20px no_border user_select_none">
                      <img className="valign_middle" src="artwork/elastos-black-small.svg"></img>
                      Elastos <Version/>
                      </td>
                  </tr>
                  <tr>
                    <td className="white_on_purple h20px no_border"></td>
                  </tr>
                  <tr>
                    <td id='home' className="white_on_purple_with_hover h20px fake_button" onClick={(e) => showHome()}>
                    Network
                    <select value={currentNetworkIx} name="network" onChange={(e) => changeNetwork(e)}>
                      <option value="0">{REST_SERVICES[0].name}</option>
                      <option value="1">{REST_SERVICES[1].name}</option>
                      <option value="2">{REST_SERVICES[2].name}</option>
                    </select>
                    </td>
                  </tr>
                  <tr>
                    <td className="white_on_purple_with_hover h20px fake_button" onClick={(e) => showChangeNode()}>
                      <div className="tooltip">Change Node<span className="tooltiptext">{restService}</span>
                      </div>
                    </td>
                  </tr>
                  <tr>
                    <td className="white_on_purple h20px no_border"></td>
                  </tr>
                  <tr>
                    <td id='home' className="white_on_purple_with_hover h20px fake_button" onClick={(e) => showHome()}>
                      <img className="valign_middle" src="artwork/home.svg"></img>
                      Home</td>
                  </tr>
                  <tr>
                    <td id='send' className="white_on_purple_with_hover h20px fake_button" onClick={(e) => showSend()}>
                      <img className="valign_middle" src="artwork/send.svg"></img>
                      Send</td>
                  </tr>
                  <tr>
                    <td id='receive' className="white_on_purple_with_hover h20px fake_button" onClick={(e) => showReceive()}>
                      <img className="valign_middle" src="artwork/receive.svg"></img>
                      Receive</td>
                  </tr>
                  <tr>
                    <td id='transactions' className="white_on_purple_with_hover h20px fake_button" onClick={(e) => showTransactions()}>
                      <img className="valign_middle" src="artwork/transactions.svg"></img>
                      Transactions</td>
                  </tr>
                  <tr>
                    <td id='voting' className="white_on_purple_with_hover h20px fake_button" onClick={(e) => showVoting()}>
                      <img className="valign_middle" src="artwork/voting.svg"></img>
                      Voting</td>
                  </tr>
                  <tr>
                    <td id='refresh' className="white_on_purple_with_hover h20px fake_button" onClick={(e) => refreshBlockchainData()}>
                      <img className="valign_middle" src="artwork/refresh.svg"></img>
                      Refresh</td>
                  </tr>
                  <tr>
                    <td className="white_on_purple h250px no_border"></td>
                  </tr>
                  <tr>
                    <td className="white_on_purple_with_hover h20px fake_button" onClick={(e) => showLogin()}>Logout</td>
                  </tr>
                  <tr>
                    <td className="white_on_purple_with_hover h20px fake_button" onClick={(e) => openDevTools()}>Show Dev Tools</td>
                  </tr>
                </tbody>
              </table>
            </td>
            <td className="valign_top black_on_offwhite no_border no_padding">
              <table className="w626px black_on_offwhite no_border no_padding">
                <tbody>
                  <tr id="elastos-branding" className="no_border no_padding">
                    <td className="h325px w595px no_border no_padding">
                      <div className="branding_container">
                        <a href="https://elastos.org" onClick={(e) => onLinkClick(e)}>
                          <img className="branding_image" style={{
                              left: '175px',
                              top: '10px'
                            }} src="artwork/elastos-branding.svg"></img>
                          <img style={{
                              left: '380px',
                              top: '130px'
                            }} className="branding_image" src="artwork/elastos-white-large.svg"></img>
                        </a>
                      </div>
                    </td>
                  </tr>
                  <tr id="ledger-login">
                    <td className="black_on_white h20px darkgray_border">
                      <div className="gray_on_white">Ledger Status</div>
                      <p><LedgerMessage/>
                      </p>
                      <UseLedgerButton/>
                    </td>
                  </tr>
                  <tr id="change-node">
                    <td className="black_on_white h20px darkgray_border">
                      <div className="gray_on_white">Node URL</div>
                      <br/>
                      <input style={{
                          fontFamily: 'monospace'
                        }} type="text" size="64" id="nodeUrl" placeholder={restService}></input>
                      <br/>
                      <br/>
                      <br/>
                      <div className="white_on_gray bordered display_inline_block fake_button rounded padding_5px" onClick={(e) => showLogin()}>Cancel</div>
                      <div className="white_on_gray bordered display_inline_block float_right fake_button rounded padding_5px" onClick={(e) => changeNodeUrl()}>Change Node URL</div>
                      <div className="white_on_gray bordered display_inline_block float_right fake_button rounded padding_5px" onClick={(e) => resetNodeUrl()}>Reset Node URL TO Default</div>
                    </td>
                  </tr>
                  <tr id="private-key-login">
                    <td className="black_on_white h20px darkgray_border">
                      <div className="gray_on_white">Private Key</div>
                      <p>Enter private key manually.</p>
                      <div className="white_on_gray bordered display_inline_block float_right fake_button rounded padding_5px" onClick={(e) => showPrivateKeyEntry()}>Enter Key</div>
                    </td>
                  </tr>
                  <tr id="private-key-entry">
                    <td className="black_on_white h20px darkgray_border">
                      <div className="gray_on_white">Private Key</div>
                      <br/>
                      <input style={{
                          fontFamily: 'monospace'
                        }} type="text" size="64" id="privateKey" placeholder="Private Key"></input>
                      <br/>
                      <div className="white_on_gray bordered display_inline_block float_right fake_button rounded padding_5px" onClick={(e) => getPublicKeyFromPrivateKey()}>Use Private Key</div>
                    </td>
                  </tr>
                  <tr id="private-key-generate">
                    <td className="black_on_white h20px darkgray_border">
                      <div className="gray_on_white">Generate New Private Key</div>
                      <p>Generate new private key.</p>
                      <div className="white_on_gray bordered display_inline_block float_right fake_button rounded padding_5px" onClick={(e) => showGenerateNewPrivateKey()}>Generate Key</div>
                    </td>
                  </tr>
                  <tr id="private-key-generator">
                    <td className="black_on_white h20px darkgray_border">
                      <div className="gray_on_white">New Private Key</div>
                      <br/>
                      <br/>
                      {generatedPrivateKeyHex}
                      <br/>
                      <br/>
                      <hr/>
                      <strong>
                      Reminder: Save this private key.
                      <br/>
                      If you lose this key, there will be no way to recover your coins.
                      <br/>
                      Keep a backup of it in a safe place.
                      <br/>
                      To use this key, copy it (you can use the convenient copy button), and use to log in to the wallet.
                      <br/>
                      </strong>
                      <br/>
                      <div className="white_on_gray bordered display_inline_block float_left fake_button rounded padding_5px" onClick={(e) => copyToClipboard()}>Copy</div>
                      <br/>
                      <div className="white_on_gray bordered display_inline_block float_right fake_button rounded padding_5px" onClick={(e) => showLogin()}>Done</div>
                      <br/>
                    </td>
                  </tr>
                  <tr id="your-address">
                    <td className="black_on_white h20px darkgray_border">
                      <div className="gray_on_white">Your Address</div>
                      <br/>{address}
                    </td>
                  </tr>
                  <tr id="transaction-list-small">
                    <td className="black_on_white h20px darkgray_border">
                      <div className="gray_on_white ` display_inline_block">Previous Transactions ({parsedTransactionHistory.length}
                        total)</div>
                      <div className="gray_on_white float_right display_inline_block">&nbsp;{getConfirmations()}&nbsp;
                        Confirmations</div>
                      <div className="gray_on_white float_right display_inline_block">&nbsp;{blockchainState.height}&nbsp;
                        Blocks</div>
                      <br/>
                      <table className="w100pct black_on_offwhite no_border whitespace_nowrap">
                        <tbody>
                          <tr>
                            <td className="no_border no_padding">Nbr</td>
                            <td className="no_border no_padding">Icon</td>
                            <td className="no_border no_padding">Value</td>
                            <td className="no_border no_padding">TX</td>
                            <td className="no_border no_padding">Time</td>
                          </tr>
                          {
                            parsedTransactionHistory.map((item, index) => {
                              if (index > 2) {
                                return undefined;
                              }
                              return (<tr key={index}>
                                <td className="no_border no_padding">{item.n}</td>
                                <td className="no_border no_padding">
                                  <TransactionHistoryElementIcon item={item}/>{/* item.type */}
                                </td>
                                <td className="no_border no_padding">{item.value}
                                  ELA</td>
                                <td className="no_border no_padding">
                                  <a href={item.txDetailsUrl} onClick={(e) => onLinkClick(e)}>{item.txHash}</a>
                                </td>
                                <td className="no_border no_padding">
                                  {item.time}
                                </td>
                              </tr>)
                            })
                          }
                        </tbody>
                      </table>
                    </td>
                  </tr>
                  <tr id="transaction-list-large">
                    <td className="black_on_white h20px darkgray_border">
                      <div className="gray_on_white">Transaction List Status</div>
                      <br/> {transactionHistoryStatus}
                      <div className="gray_on_white">Blockchain Status</div>
                      <br/> {blockchainStatus}
                      <br/>
                      <div className="gray_on_white ` display_inline_block">Previous Transactions ({parsedTransactionHistory.length}
                        total)</div>
                      <div className="gray_on_white float_right display_inline_block">&nbsp;{getConfirmations()}&nbsp;
                        Confirmations</div>
                      <div className="gray_on_white float_right display_inline_block">&nbsp;{blockchainState.height}&nbsp;
                        Blocks</div>
                      <p></p>
                      <div className="h420px overflow_auto">
                        <table className="w100pct black_on_offwhite no_border whitespace_nowrap">
                          <tbody>
                            <tr>
                              <td className="no_border no_padding">Nbr</td>
                              <td className="no_border no_padding">Icon</td>
                              <td className="no_border no_padding">Value</td>
                              <td className="no_border no_padding">TX</td>
                              <td className="no_border no_padding">Time</td>
                            </tr>
                            {
                              parsedTransactionHistory.map((item, index) => {
                                return (<tr key={index}>
                                  <td className="no_border no_padding">{item.n}</td>
                                  <td className="no_border no_padding">
                                    <TransactionHistoryElementIcon item={item}/>{/* item.type */}
                                  </td>
                                  <td className="no_border no_padding">{item.value}
                                    ELA</td>
                                  <td className="no_border no_padding">
                                    <a href={item.txDetailsUrl} onClick={(e) => onLinkClick(e)}>{item.txHash}</a>
                                  </td>
                                  <td className="no_border no_padding">
                                    {item.time}
                                  </td>
                                </tr>)
                              })
                            }
                          </tbody>
                        </table>
                      </div>
                    </td>
                  </tr>
                  <tr id="transaction-more-info">
                    <td className="black_on_white h20px darkgray_border">
                      <div className="gray_on_white">More Info</div>
                      <br/>Tap on the transaction ID to view further details or visit http://blockchain.elastos.org
                      <br/>Or https://blockchain-beta.elastos.org/tx/
                    </td>
                  </tr>
                  <tr id="from-address">
                    <td className="black_on_white h20px darkgray_border">
                      <div className="gray_on_white">From Address</div>
                      <p>{address}</p>
                    </td>
                  </tr>
                  <tr id="balance">
                    <td className="black_on_white h20px darkgray_border">
                      <div className="gray_on_white">Balance</div>
                      <p>{balance}</p>
                    </td>
                  </tr>
                  <tr id="send-amount">
                    <td className="black_on_white h20px darkgray_border">
                      <div className="gray_on_white">Send Amount</div>
                      <br/>
                      <input style={{
                          fontFamily: 'monospace'
                        }} type="text" size="64" id="sendAmount" placeholder="Send Amount"></input>
                    </td>
                  </tr>
                  <tr id="to-address">
                    <td className="black_on_white h20px darkgray_border">
                      <div className="gray_on_white">To Address</div>
                      <br/>
                      <input style={{
                          fontFamily: 'monospace'
                        }} type="text" size="64" id="sendToAddress" placeholder="Send To Address"></input>
                    </td>
                  </tr>
                  <tr id="send-spacer-01">
                    <td className="black_on_white h200px no_border">
                      <div className="gray_on_white">Balance Status</div>
                      <br/> {balanceStatus}
                      <br/>
                      <div className="gray_on_white">Send Status</div>
                      <br/>
                      <div className="h100px w600px overflow_auto">
                        <table>
                          <tbody>
                            {
                              sendToAddressStatuses.map((sendToAddressStatus, index) => {
                                return (<tr key={index}>
                                  <td>{sendToAddressStatus}</td>
                                </tr>)
                              })
                            }
                            {
                              sendToAddressLinks.map((item, index) => {
                                return (<tr key={index}>
                                  <td>
                                    <a href={item.txDetailsUrl} onClick={(e) => onLinkClick(e)}>{item.txHash}</a>
                                  </td>
                                </tr>)
                              })
                            }
                          </tbody>
                        </table>
                      </div>
                    </td>
                  </tr>
                  <tr id="confirm-and-see-fees">
                    <td className="black_on_white h20px darkgray_border">
                      <div className="gray_on_white">Confirm</div>
                      <p>Tap ‘Next’ to confirm the fees for your transaction.</p>
                      <p></p>
                      <div className="lightgray_border white_on_black bordered display_inline_block float_right fake_button rounded padding_5px" onClick={(e) => showConfirmAndSeeFees()}>Next</div>
                    </td>
                  </tr>
                  <tr id="fees">
                    <td className="black_on_white h20px darkgray_border">
                      <div className="gray_on_white display_inline_block">Fees (in Satoshis)</div>
                      <br/>
                      <input style={{
                          fontFamily: 'monospace'
                        }} type="text" size="64" id="feeAmount" placeholder="Fees"></input>
                      <p></p>
                      <div className="white_on_black lightgray_border bordered display_inline_block float_right fake_button rounded padding_5px" onClick={(e) => updateAmountAndFeesAndRenderApp()}>Estimated New Balance</div>
                    </td>
                  </tr>
                  <tr id="cancel-confirm-transaction">
                    <td className="black_on_white h20px darkgray_border">
                      <div className="gray_on_white">Confirm</div>
                      <p>Your balance will be deducted {sendAmount}
                        ELA + {feeAmountEla}
                        ELA in fees.</p>
                      <p></p>
                      <div className="white_on_black lightgray_border bordered display_inline_block float_right fake_button rounded padding_5px" onClick={(e) => sendAmountToAddress()}>Confirm</div>
                      <div className="white_on_gray darkgray_border bordered display_inline_block float_right fake_button rounded padding_5px" onClick={(e) => cancelSend()}>Cancel</div>
                    </td>
                  </tr>
                  <tr id="completed-transaction">
                    <td className="black_on_white h20px darkgray_border">
                      <div className="white_on_black lightgray_border bordered display_inline_block float_right fake_button rounded padding_5px" onClick={(e) => showTransactions()}>Show Transactions</div>
                    </td>
                  </tr>
                  <tr id="candidate-list">
                    <td className="black_on_white h20px darkgray_border">
                      <div className="gray_on_white">Producer List Status</div>
                      <br/> {producerListStatus}
                      <br/>
                      <div className="gray_on_white float_right display_inline_block">
                        <span className="padding_2px">{parsedProducerList.totalvotes}</span>
                        Votes</div>
                      <div className="gray_on_white float_right display_inline_block">
                        <span className="padding_2px">{parsedProducerList.totalcounts}</span>
                        Counts</div>
                      <div className="gray_on_white float_right display_inline_block">
                        <span className="padding_2px">{parsedProducerList.producersCandidateCount}</span>
                        Selected Candidates</div>
                      <div className="gray_on_white display_inline_block">
                        Candidates (
                        <span className="padding_2px">{parsedProducerList.producers.length}</span>
                        total)</div>
                      <p></p>
                      <div className="h200px overflow_auto">
                        <table className="w100pct black_on_offwhite no_border whitespace_nowrap">
                          <tbody>
                            <tr>
                              <td className="no_border no_padding">N</td>
                              <td className="no_border no_padding">Nickname</td>
                              <td className="no_border no_padding">Active</td>
                              <td className="no_border no_padding">Votes</td>
                              <td className="no_border no_padding">Select</td>
                            </tr>
                            {
                              parsedProducerList.producers.map((item, index) => {
                                return (<tr key={index}>
                                  <td className="no_border no_padding">{item.n}</td>
                                  <td className="no_border no_padding">{item.nickname}</td>
                                  <td className="no_border no_padding">{item.active}</td>
                                  <td className="no_border no_padding">{item.votes}</td>
                                  <td className="white_on_purple_with_hover h20px fake_button" onClick={(e) => toggleProducerSelection({index})}>
                                    <ProducerSelectionButtonText item={item}/>
                                  </td>
                                </tr>)
                              })
                            }
                          </tbody>
                        </table>
                      </div>
                    </td>
                  </tr>
                  <tr id="candidate-vote-button">
                    <td className="black_on_white h20px darkgray_border">
                      <div className="lightgray_border white_on_black bordered display_inline_block float_right fake_button rounded padding_5px" onClick={(e) => sendVoteTx()}>Send Votes</div>
                    </td>
                  </tr>
                  <tr id="candidate-vote-list">
                    <td className="black_on_white h20px darkgray_border">
                      <div className="gray_on_white">Candidate List Status</div>
                      <br/> {candidateVoteListStatus}
                      <br/>
                      <div className="gray_on_white ` display_inline_block">
                        Candidate Votes (
                        <span>{parsedCandidateVoteList.candidateVotes.length}</span>
                        total)</div>
                      <p></p>
                      <div className="h200px overflow_auto">
                        <table className="w100pct black_on_offwhite no_border whitespace_nowrap">
                          <tbody>
                            <tr>
                              <td className="no_border no_padding">N</td>
                              <td className="no_border no_padding">Nickname</td>
                              <td className="no_border no_padding">Votes</td>
                            </tr>
                            {
                              parsedCandidateVoteList.candidateVotes.map((item, index) => {
                                return (<tr key={index}>
                                  <td className="no_border no_padding">{item.n}</td>
                                  <td className="no_border no_padding">{item.nickname}</td>
                                  <td className="no_border no_padding">{item.votes}</td>
                                </tr>)
                              })
                            }
                          </tbody>
                        </table>
                      </div>
                    </td>
                  </tr>
                </tbody>
              </table>
            </td>
          </tr>
        </tbody>
      </table>
    </div>)
  }
}
const renderApp = () => {
  ReactDOM.render(<App/>, document.getElementById('main'));
};
const onLoad = () => {
  setRestService(0);
  showLogin();
}

/** call initialization functions */
window.onload = onLoad;

setPollForAllInfoTimer();

renderApp();
