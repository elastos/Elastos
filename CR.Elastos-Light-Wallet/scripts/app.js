'use strict';

/** imports */
const bip39 = require('bip39');
const mainConsoleLib = require('console');
const BigNumber = require('bignumber.js');
const crypto = require('crypto');

/* modules */
const LedgerComm = require('./LedgerComm.js');
const AddressTranscoder = require('./AddressTranscoder.js');
const KeyTranscoder = require('./KeyTranscoder.js');
const TxTranscoder = require('./TxTranscoder.js');
const TxSigner = require('./TxSigner.js');
const Asset = require('./Asset.js');
const TxFactory = require('./TxFactory.js');
const Mnemonic = require('./Mnemonic.js');
const GuiUtils = require('./GuiUtils.js');
const CoinGecko = require('./CoinGecko.js');

/** global constants */
const LOG_LEDGER_POLLING = true;

const MAX_POLL_DATA_TYPE_IX = 4;

const PRIVATE_KEY_LENGTH = 64;

const DEFAULT_FEE_SATS = '500';

const EXPLORER = 'https://blockchain.elastos.org';

const REST_SERVICES = [
  {
    name: 'api-wallet',
    url: 'https://api-wallet-ela.elastos.org',
  },
  {
    name: 'node1',
    url: 'https://node1.elaphant.app',
  },
  {
    name: 'node3',
    url: 'https://node3.elaphant.app',
  },
];

/** global variables */
let currentNetworkIx = 0;

let restService;

let ledgerDeviceInfo = undefined;

let publicKey = undefined;

let address = undefined;

let pollDataTypeIx = 0;

let balance = undefined;

let sendAmount = '';

let feeAmountSats = '';

let feeAmountEla = '';

let isLoggedIn = false;

let useLedgerFlag = false;

let generatedPrivateKeyHex = undefined;

let generatedMnemonic = undefined;

let appClipboard = undefined;

let appDocument = undefined;

let renderApp = undefined;

const sendToAddressStatuses = [];
const sendToAddressLinks = [];

let balanceStatus = 'No Balance Requested Yet';

let transactionHistoryStatus = 'No History Requested Yet';

const parsedTransactionHistory = [];

let producerListStatus = 'No Producers Requested Yet';

let parsedProducerList = {};

let candidateVoteListStatus = 'No Candidate Votes Requested Yet';

let parsedCandidateVoteList = {};

let unspentTransactionOutputsStatus = 'No UTXOs Requested Yet';

const parsedUnspentTransactionOutputs = [];

let blockchainStatus = 'No Blockchain State Requested Yet';

let blockchainState = {};

let blockchainLastActionHeight = 0;

const mainConsole = new mainConsoleLib.Console(process.stdout, process.stderr);

let GuiToggles;

/** functions */
const init = (_GuiToggles) => {
  sendToAddressStatuses.push('No Send-To Transaction Requested Yet');
  parsedProducerList.totalvotes = '-';
  parsedProducerList.totalcounts = '-';
  parsedProducerList.producersCandidateCount = 0;
  parsedProducerList.producers = [];
  parsedCandidateVoteList.candidateVotes = [];

  GuiToggles = _GuiToggles;

  setRestService(0);

  mainConsole.log('Consone Logging Enabled.');
};

const setAppClipboard = (clipboard) => {
  appClipboard = clipboard;
};

const setAppDocument = (_document) => {
  appDocument = _document;
};

const setRenderApp = (_renderApp) => {
  renderApp = _renderApp;
};

const getRestService = () => {
  return restService;
};

const setRestService = (ix) => {
  currentNetworkIx = ix;
  restService = REST_SERVICES[ix].url;
  get('nodeUrl').value = restService;
};

const getTransactionHistoryUrl = (address) => {
  const url = `${EXPLORER}/api/v1/txs/?pageNum=0&address=${address}`;
  return url;
};

const getTransactionHistoryLink = (txid) => {
  const url = `${EXPLORER}/tx/${txid}`;
  return url;
};

const getBalanceUrl = (address) => {
  const url = `${getRestService()}/api/v1/asset/balances/${address}`;
  mainConsole.log('getBalanceUrl', url);
  return url;
};

const getUnspentTransactionOutputsUrl = (address) => {
  const url = `${getRestService()}/api/v1/asset/utxo/${address}/${Asset.elaAssetId}`;
  return url;
};

const getStateUrl = () => {
  const url = `${getRestService()}/api/v1/node/state`;
  return url;
};


const formatDate = (date) => {
  let month = (date.getMonth() + 1).toString();
  let day = date.getDate().toString();
  const year = date.getFullYear();

  if (month.length < 2) {
    month = '0' + month;
  };
  if (day.length < 2) {
    day = '0' + day;
  };

  return [year, month, day].join('-');
};

const changeNetwork = (event) => {
  currentNetworkIx = event.target.value;
  setRestService(currentNetworkIx);
  refreshBlockchainData();
};

const resetNodeUrl = () => {
  setRestService(currentNetworkIx);
  renderApp();
};

const changeNodeUrl = () => {
  restService = get('nodeUrl').value;
  showLogin();
};

const refreshBlockchainData = () => {
  requestTransactionHistory();
  requestBalance();
  requestUnspentTransactionOutputs();
  requestBlockchainState();
  CoinGecko.requestPriceData();
  renderApp();
};

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
};

const pollForDataCallback = (message) => {
  if (LOG_LEDGER_POLLING) {
    mainConsole.log(`pollForDataCallback ${JSON.stringify(message)}`);
  }
  ledgerDeviceInfo = message;
  renderApp();
  pollDataTypeIx++;
  setPollForAllInfoTimer();
};

const pollForData = () => {
  if (LOG_LEDGER_POLLING) {
    mainConsole.log('getAllLedgerInfo ' + pollDataTypeIx);
  }
  const resetPollIndex = false;
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
};

const postJson = (url, jsonString, readyCallback, errorCallback) => {
  const xmlhttp = new XMLHttpRequest(); // new HttpRequest instance

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
  };
  xhttp.responseType = 'text';
  xhttp.open('POST', url, true);
  xhttp.setRequestHeader('Content-Type', 'application/json');
  xhttp.setRequestHeader('Authorization', 'Basic RWxhVXNlcjpFbGExMjM=');

  // sendToAddressStatuses.push( `XMLHttpRequest: curl ${url} -H "Content-Type: application/json" -d '${jsonString}'` );

  xhttp.send(jsonString);
};

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
  };
  xhttp.responseType = 'text';
  xhttp.open('GET', url, true);
  xhttp.send();
};

const requestUnspentTransactionOutputs = () => {
  unspentTransactionOutputsStatus = 'UTXOs Requested';
  const unspentTransactionOutputsUrl = getUnspentTransactionOutputsUrl(address);

  // mainConsole.log( 'unspentTransactionOutputsUrl ' + unspentTransactionOutputsUrl );

  getJson(unspentTransactionOutputsUrl, getUnspentTransactionOutputsReadyCallback, getUnspentTransactionOutputsErrorCallback);
};

const getUnspentTransactionOutputsErrorCallback = (response) => {
  unspentTransactionOutputsStatus = `UTXOs Error ${JSON.stringify(response)}`;

  renderApp();
};

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
};

const get = (id) => {
  const elt = document.getElementById(id);
  if (elt == null) {
    throw new Error('elt is null:' + id);
  }
  return elt;
};

const hide = (id) => {
  get(id).style = 'display:none;';
};

const show = (id) => {
  get(id).style = 'display:default;';
};

const getPublicKeyFromLedger = () => {
  useLedgerFlag = true;
  isLoggedIn = true;
  LedgerComm.getPublicKey(publicKeyCallback);
};

const requestBlockchainDataAndShowHome = () => {
  if (publicKey === undefined) {
    return;
  }
  address = AddressTranscoder.getAddressFromPublicKey(publicKey);
  requestTransactionHistory();
  requestBalance();
  requestUnspentTransactionOutputs();
  requestBlockchainState();

  if (isLoggedIn) {
    GuiToggles.showHome();
  }
};

const getPublicKeyFromMnemonic = () => {
  useLedgerFlag = false;
  isLoggedIn = true;
  show('mnemonic');
  const mnemonicElt = document.getElementById('mnemonic');
  const mnemonic = mnemonicElt.value;
  if (!bip39.validateMnemonic(mnemonic)) {
    alert(`mnemonic is not valid.`);
    return;
  }
  const privateKey = Mnemonic.getPrivateKeyFromMnemonic(mnemonic);
  if (privateKey.length != PRIVATE_KEY_LENGTH) {
    alert(`mnemonic must create a of length ${PRIVATE_KEY_LENGTH}, not ${privateKey.length}`);
    return;
  }
  publicKey = KeyTranscoder.getPublic(privateKey);
  requestBlockchainDataAndShowHome();
};

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
};

const sendAmountToAddressErrorCallback = (error) => {
  sendToAddressStatuses.push(JSON.stringify(error));
  renderApp();
};

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
};

const clearSendData = () => {
  mainConsole.log('STARTED clearSendData');
  GuiUtils.setValue('sendAmount', '');
  GuiUtils.setValue('sendToAddress', '');
  GuiUtils.setValue('feeAmount', DEFAULT_FEE_SATS);
  sendAmount = '';
  feeAmountSats = '';
  feeAmountEla = '';
  sendToAddressStatuses.length = 0;
  sendToAddressLinks.length = 0;
  mainConsole.log('SUCCESS clearSendData');
};

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
};

const updateAmountAndFeesAndRenderApp = () => {
  updateAmountAndFees();
  renderApp();
};

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

  let encodedTx;

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
    };
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
};
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
};

const requestListOfProducersErrorCallback = (response) => {
  producerListStatus = `Producers Error: ${JSON.stringify(response)}`;
  renderApp();
};

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


    response.result.sort((producer0, producer1) => {
      return parseFloat(producer1.Votes) - parseFloat(producer0.Votes);
    });

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
};

const requestListOfProducers = () => {
  producerListStatus = 'Producers Requested';
  const txUrl = `${getRestService()}/api/v1/dpos/rank/height/0?state=active`;

  renderApp();
  getJson(txUrl, requestListOfProducersReadyCallback, requestListOfProducersErrorCallback);
};

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
    if (parsedProducerElt.isCandidate) {
      parsedProducerList.producersCandidateCount++;
    }
  });

  renderApp();
};

const requestListOfCandidateVotesErrorCallback = (response) => {
  const displayRawError = true;
  if (displayRawError) {
    candidateVoteListStatus = `Candidate Votes Error: ${JSON.stringify(response)}`;
  }
  renderApp();
};

const requestListOfCandidateVotesReadyCallback = (response) => {
  candidateVoteListStatus = 'Candidate Votes Received';

  // mainConsole.log('STARTED Candidate Votes Callback', response);
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
      });
    });
    mainConsole.log('INTERIM Candidate Votes Callback', response.result);
  }
  mainConsole.log('SUCCESS Candidate Votes Callback');

  renderApp();
};

const requestListOfCandidateVotes = () => {
  candidateVoteListStatus = 'Candidate Votes Requested';

  const txUrl = `${getRestService()}/api/v1/dpos/address/${address}?pageSize=5000&pageNum=1`;

  renderApp();
  getJson(txUrl, requestListOfCandidateVotesReadyCallback, requestListOfCandidateVotesErrorCallback);
};

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
  });

  mainConsole.log('sendVoteTx.candidates ' + JSON.stringify(candidates));

  let encodedTx;

  mainConsole.log('sendVoteTx.useLedgerFlag ' + JSON.stringify(useLedgerFlag));
  mainConsole.log('sendVoteTx.unspentTransactionOutputs ' + JSON.stringify(unspentTransactionOutputs));
  candidateVoteListStatus = `Voting for ${parsedProducerList.producersCandidateCount} candidates.`;
  if (useLedgerFlag) {
    if (unspentTransactionOutputs) {
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
      };
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
};
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
};

const senVoteErrorCallback = (error) => {
  mainConsole.log('senVoteErrorCallback ' + JSON.stringify(error));
  // sendToAddressStatuses.push(JSON.stringify(error));
  renderApp();
};

const sendVoteReadyCallback = (transactionJson) => {
  mainConsole.log('sendVoteReadyCallback ' + JSON.stringify(transactionJson));
  if (transactionJson.Error) {
    candidateVoteListStatus(`Vote Error: ${transactionJson.Error} ${transactionJson.Result}`);
  } else {
    candidateVoteListStatus = `Vote Success TX: ${transactionJson.result}`;
  }
  renderApp();
};

const getTransactionHistoryErrorCallback = (response) => {
  transactionHistoryStatus = `History Error: ${JSON.stringify(response)}`;
  renderApp();
};

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
        if (voutAddress == address) {
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
};

const requestTransactionHistory = () => {
  transactionHistoryStatus = 'History Requested';
  const transactionHistoryUrl = getTransactionHistoryUrl(address);
  // mainConsole.log('requestTransactionHistory ' + transactionHistoryUrl);
  getJson(transactionHistoryUrl, getTransactionHistoryReadyCallback, getTransactionHistoryErrorCallback);
};

const getBalanceErrorCallback = (response) => {
  balanceStatus = `Balance Error:${JSON.stringify(response)} `;
  balance = undefined;
};

const getBalanceReadyCallback = (balanceResponse) => {
  if (balanceResponse.Error == 0) {
    balanceStatus = `Balance Received.`;
    balance = balanceResponse.Result;
  } else {
    balanceStatus = `Balance Received Error:${balanceResponse.Error}`;
    balance = undefined;
  }
  mainConsole.log('getBalanceReadyCallback ' + JSON.stringify(balanceResponse));

  renderApp();
};

const requestBalance = () => {
  const balanceUrl = getBalanceUrl(address);
  balanceStatus = `Balance Requested ${balanceUrl}`;
  getJson(balanceUrl, getBalanceReadyCallback, getBalanceErrorCallback);
};

const getBlockchainStateErrorCallback = (response) => {
  balanceStatus = `Blockchain State Error:${JSON.stringify(response)} `;
};

const getBlockchainStateReadyCallback = (blockchainStateResponse) => {
  blockchainStatus = `Blockchain State Received:${blockchainStateResponse.Desc} ${blockchainStateResponse.Error} `;
  blockchainState = blockchainStateResponse.Result;

  renderApp();
};

const requestBlockchainState = () => {
  const stateUrl = getStateUrl();
  blockchainState = {};
  blockchainStatus = `Blockchain State Requested ${stateUrl}`;
  getJson(stateUrl, getBlockchainStateReadyCallback, getBlockchainStateErrorCallback);
};

const getConfirmations = () => {
  if (blockchainState.height) {
    return blockchainState.height - blockchainLastActionHeight;
  } else {
    return 0;
  }
};

const setBlockchainLastActionHeight = () => {
  if (blockchainState.height) {
    blockchainLastActionHeight = blockchainState.height;
  }
};

const removeClass = (id, cl) => {
  get(id).classList.remove(cl);
};

const addClass = (id, cl) => {
  get(id).classList.add(cl);
};

const selectButton = (id) => {
  addClass(id, 'white_on_light_purple');
  removeClass(id, 'white_on_purple_with_hover');
};

const clearButtonSelection = (id) => {
  removeClass(id, 'white_on_light_purple');
  addClass(id, 'white_on_purple_with_hover');
};

const hideEverything = () => {
  clearButtonSelection('send');
  clearButtonSelection('home');
  clearButtonSelection('receive');
  clearButtonSelection('transactions');
  clearButtonSelection('voting');
  hide('change-node');
  hide('private-key-entry');
  hide('mnemonic-entry');
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
  hide('mnemonic-login');
  hide('ledger-login');
  hide('elastos-branding');
  hide('send-spacer-01');
  hide('private-key-generate');
  hide('private-key-generator');
  hide('mnemonic-generate');
  hide('mnemonic-generator');
  hide('candidate-list');
  hide('candidate-vote-button');
  hide('candidate-vote-list');
};

const copyMnemonicToClipboard = () => {
  clipboard.writeText(generatedMnemonic);
  alert(`copied to clipboard:\n${generatedMnemonic}`);
};

const copyToPrivateKeyClipboard = () => {
  clipboard.writeText(generatedPrivateKeyHex);
  alert(`copied to clipboard:\n${generatedPrivateKeyHex}`);
};

const showChangeNode = () => {
  clearGlobalData();
  hideEverything();
  clearSendData();
  show('change-node');
};

const showLogin = () => {
  clearGlobalData();
  hideEverything();
  clearSendData();
  show('private-key-login');
  show('mnemonic-login');
  show('ledger-login');
  show('elastos-branding');
  show('mnemonic-generate');
  show('private-key-generate');
};

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
};

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
};

const showConfirmAndSeeFees = () => {
  hideEverything();
  show('fees');
  show('cancel-confirm-transaction');
  show('send-spacer-01');
  selectButton('send');
  updateAmountAndFeesAndRenderApp();
};

const showCompletedTransaction = () => {
  hideEverything();
  show('fees');
  show('completed-transaction');
  show('send-spacer-01');
  selectButton('send');
  updateAmountAndFeesAndRenderApp();
};

const showReceive = () => {
  if (!isLoggedIn) {
    return;
  }
  hideEverything();
  clearSendData();
  show('your-address');
  selectButton('receive');
};

const showTransactions = () => {
  if (!isLoggedIn) {
    return;
  }
  hideEverything();
  clearSendData();
  show('transaction-more-info');
  show('transaction-list-large');
  selectButton('transactions');
};

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
};

const showPrivateKeyEntry = () => {
  hideEverything();
  clearSendData();
  show('private-key-entry');
};

const showMnemonicEntry = () => {
  hideEverything();
  clearSendData();
  show('mnemonic-entry');
};

const showGenerateNewMnemonic = () => {
  hideEverything();
  clearSendData();
  show('mnemonic-generator');
  generatedMnemonic = bip39.entropyToMnemonic(crypto.randomBytes(32).toString('hex'));
  renderApp();
};

const showGenerateNewPrivateKey = () => {
  hideEverything();
  clearSendData();
  show('private-key-generator');
  generatedPrivateKeyHex = crypto.randomBytes(32).toString('hex');
  renderApp();
};

const clearGlobalData = () => {
  get('privateKey').value = '';
  get('mnemonic').value = '';
  get('feeAmount').value = DEFAULT_FEE_SATS;
  get('nodeUrl').value = '';

  useLedgerFlag = false;
  publicKey = undefined;
  address = undefined;
  balance = undefined;
  generatedPrivateKeyHex = undefined;
  generatedMnemonic = undefined;

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
};

const getLedgerDeviceInfo = () => {
  return ledgerDeviceInfo;
};

const getMainConsole = () => {
  return mainConsole;
};

const getELABalance = () => {
  if (balance) {
    return balance;
  }
  // mainConsole.log('getELABalance', balanceStatus);
  return '?';
};

const getUSDBalance = () => {
  const data = CoinGecko.getPriceData();
  if (data) {
    const elastos = data.elastos;
    if (elastos) {
      const usd = elastos.usd;

      mainConsole.log('getUSDBalance', usd, balance, balanceStatus);
      if (balance) {
        return (parseFloat(usd) * parseFloat(balance)).toFixed(3);
      }
    }
  }
  return '?';
};

const getAddress = () => {
  return address;
};

exports.init = init;
exports.trace = mainConsole.trace;
exports.setAppClipboard = setAppClipboard;
exports.setAppDocument = setAppDocument;
exports.setRenderApp = setRenderApp;
exports.getLedgerDeviceInfo = getLedgerDeviceInfo;
exports.getMainConsole = getMainConsole;
exports.getPublicKeyFromLedger = getPublicKeyFromLedger;
exports.refreshBlockchainData = refreshBlockchainData;
exports.clearSendData = clearSendData;
exports.setPollForAllInfoTimer = setPollForAllInfoTimer;
exports.getPublicKeyFromMnemonic = getPublicKeyFromMnemonic;
exports.getPublicKeyFromPrivateKey = getPublicKeyFromPrivateKey;
exports.getAddress = getAddress;
exports.getELABalance = getELABalance;
exports.getUSDBalance = getUSDBalance;
