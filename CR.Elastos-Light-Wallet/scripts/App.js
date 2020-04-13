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
const LOG_LEDGER_POLLING = false;

const MAX_POLL_DATA_TYPE_IX = 5;

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

let sendHasFocus = false;

let sendAmount = '';

let feeAmountSats = '';

let feeAmountEla = '';

let sendToAddress = '';

let sendStep = 1;

let isLoggedIn = false;

let useLedgerFlag = false;

let generatedPrivateKeyHex = undefined;

let generatedMnemonic = undefined;

let appClipboard = undefined;

let appDocument = undefined;

let renderApp = () => {};

const sendToAddressStatuses = [];
const sendToAddressLinks = [];

let balanceStatus = 'No Balance Requested Yet';

let transactionHistoryStatus = 'No History Requested Yet';

const parsedTransactionHistory = [];

let producerListStatus = 'No Producers Requested Yet';

let parsedProducerList = {
  totalvotes: '-',
  totalcounts: '-',
  producersCandidateCount: 0,
  producers: [],
};

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
  parsedCandidateVoteList.candidateVotes = [];

  GuiToggles = _GuiToggles;

  setRestService(0);

  mainConsole.log('Console Logging Enabled.');
};

const setAppClipboard = (clipboard) => {
  appClipboard = clipboard;
};

const setAppDocument = (_document) => {
  appDocument = _document;
};

const setRenderApp = (_renderApp) => {
  renderApp = () => {
    // mainConsole.log('renderApp', 'sendHasFocus', sendHasFocus);
    if (!sendHasFocus) {
      _renderApp();
    }
  };
};

const getRestService = () => {
  return restService;
};

const setRestService = (ix) => {
  currentNetworkIx = ix;
  restService = REST_SERVICES[ix].url;
  GuiUtils.setValue('nodeUrl', restService);
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
  // mainConsole.log('getBalanceUrl', url);
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
  restService = GuiUtils.getValue('nodeUrl');
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
    requestBlockchainData();
  } else {
    ledgerDeviceInfo.error = true;
    ledgerDeviceInfo.message = message.message;
    renderApp();
  }
  pollDataTypeIx++;
  setPollForAllInfoTimer();
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
  // if (LOG_LEDGER_POLLING) {
  // mainConsole.log('pollForData', pollDataTypeIx);
  // }
  try {
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
        } else {
          pollDataTypeIx++;
          setPollForAllInfoTimer();
        }
        break;
      case 3:
        if (address != undefined) {
          requestTransactionHistory();
          requestBalance();
          requestUnspentTransactionOutputs();
          requestBlockchainState();
        }
        pollDataTypeIx++;
        setPollForAllInfoTimer();
        break;
      case 4:
        requestListOfProducers();
        requestListOfCandidateVotes();
        pollDataTypeIx++;
        setPollForAllInfoTimer();
        break;
      case MAX_POLL_DATA_TYPE_IX:
        // only check every 10 seconds for a change in device status.
        pollDataTypeIx = 0;
        setTimeout(pollForData, 10000);
        break;
      default:
        throw Error('poll data index reset failed.');
    }
  } catch (error) {
    mainConsole.trace('pollForData', pollDataTypeIx, error);
  }
};

const setPollForAllInfoTimer = () => {
  // mainConsole.trace('setPollForAllInfoTimer', pollDataTypeIx);
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

  // mainConsole.log('getUnspentTransactionOutputsCallback ' + JSON.stringify(response), response.Result);

  if ((response.Result != undefined) && (response.Result != null) && (response.Error == 0)) {
    response.Result.forEach((utxo, utxoIx) => {
      TxFactory.updateValueSats(utxo, utxoIx);
      parsedUnspentTransactionOutputs.push(utxo);
    });
  }

  renderApp();
};

const getPublicKeyFromLedger = () => {
  useLedgerFlag = true;
  isLoggedIn = true;
  LedgerComm.getPublicKey(publicKeyCallback);
};

const requestBlockchainData = () => {
  if (publicKey === undefined) {
    return;
  }
  address = AddressTranscoder.getAddressFromPublicKey(publicKey);
  requestTransactionHistory();
  requestBalance();
  requestUnspentTransactionOutputs();
  requestBlockchainState();
};

const getPublicKeyFromMnemonic = () => {
  useLedgerFlag = false;
  isLoggedIn = true;
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
  requestBlockchainData();
};

const getPublicKeyFromPrivateKey = () => {
  useLedgerFlag = false;
  isLoggedIn = true;
  const privateKeyElt = document.getElementById('privateKey');
  const privateKey = privateKeyElt.value;
  if (privateKey.length != PRIVATE_KEY_LENGTH) {
    alert(`private key must be a hex encoded string of length ${PRIVATE_KEY_LENGTH}, not ${privateKey.length}`);
    return;
  }
  publicKey = KeyTranscoder.getPublic(privateKey);
  requestBlockchainData();
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
  setBlockchainLastActionHeight();
  renderApp();
};

const clearSendData = () => {
  mainConsole.log('STARTED clearSendData');
  GuiUtils.setValue('sendAmount', '');
  GuiUtils.setValue('sendToAddress', '');
  GuiUtils.setValue('feeAmount', DEFAULT_FEE_SATS);
  sendAmount = '';
  feeAmountSats = DEFAULT_FEE_SATS;
  feeAmountEla = '';
  sendToAddressStatuses.length = 0;
  sendToAddressLinks.length = 0;
  mainConsole.log('SUCCESS clearSendData');
};

const updateAmountAndFees = () => {
  // mainConsole.log('STARTED updateAmountAndFees');

  sendAmount = GuiUtils.getValue('sendAmount');
  sendToAddress = GuiUtils.getValue('sendToAddress');
  feeAmountSats = GuiUtils.getValue('feeAmount');

  // mainConsole.log('INTERIM updateAmountAndFees',
  //     'sendAmount:', sendAmount,
  //     'sendToAddress:', sendToAddress,
  //     'feeAmountSats:', feeAmountSats,
  // );

  if (Number.isNaN(sendAmount)) {
    throw new Error(`sendAmount ${sendAmount} is not a number`);
  }
  if (Number.isNaN(feeAmountSats)) {
    throw new Error(`feeAmountSats ${feeAmountSats} is not a number`);
  }
  feeAmountEla = BigNumber(feeAmountSats, 10).dividedBy(Asset.satoshis).toString();
  // mainConsole.log('SUCCESS updateAmountAndFees');
};

const sendAmountToAddress = () => {
  updateAmountAndFees();

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
  producerListStatus = `Producers Error, Retrying`;
  renderApp();
};

const requestListOfProducersReadyCallback = (response) => {
  producerListStatus = 'Producers Received';

  // mainConsole.log('STARTED Producers Callback', response);
  parsedProducerList = {
    totalvotes: '-',
    totalcounts: '-',
    producersCandidateCount: 0,
    producers: [],
  };
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
      // mainConsole.log('INTERIM Candidate Votes Callback', candidateVote);
      const body = candidateVote.Vote_Body;
      body.forEach((candidateVoteElt) => {
        const parsedCandidateVote = {};
        parsedCandidateVote.n = parsedCandidateVoteList.candidateVotes.length + 1;
        parsedCandidateVote.nickname = candidateVoteElt.Nickname;
        parsedCandidateVote.active = candidateVoteElt.Active.toString();
        parsedCandidateVote.votes = candidateVoteElt.Votes;
        parsedCandidateVote.ownerpublickey = candidateVoteElt.Ownerpublickey;
        // mainConsole.log('INTERIM Candidate Votes Callback', parsedCandidateVote);
        parsedCandidateVoteList.candidateVotes.push(parsedCandidateVote);
      });
    });
    // mainConsole.log('INTERIM Candidate Votes Callback', response.result);
  }
  // mainConsole.log('SUCCESS Candidate Votes Callback');

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
    tx.vout.forEach((voutElt) => {
      voutElt.scriptPubKey.addresses.forEach((voutAddress) => {
        const parsedTransaction = {};
        parsedTransaction.n = txIx;
        if (voutAddress == address) {
          parsedTransaction.type = 'Receiving';
        } else {
          parsedTransaction.type = 'Was sent';
        }
        parsedTransaction.value = voutElt.value;
        parsedTransaction.valueSat = voutElt.valueSat;
        parsedTransaction.address = voutAddress;
        parsedTransaction.txHash = tx.txid;
        parsedTransaction.txHashWithEllipsis = tx.txid;
        if (parsedTransaction.txHashWithEllipsis.length > 30) {
          parsedTransaction.txHashWithEllipsis = parsedTransaction.txHashWithEllipsis.substring(0, 30) + '...';
        }
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
  // mainConsole.log('getBalanceReadyCallback ' + JSON.stringify(balanceResponse));

  renderApp();
};

const requestBalance = () => {
  if (address != undefined) {
    const balanceUrl = getBalanceUrl(address);
    balanceStatus = `Balance Requested ${balanceUrl}`;
    getJson(balanceUrl, getBalanceReadyCallback, getBalanceErrorCallback);
  }
};

const getBlockchainStateErrorCallback = (blockchainStateResponse) => {
  balanceStatus = 'Blockchain State Error ' + blockchainStateResponse.Error;
  blockchainState = blockchainStateResponse.Result;
};

const getBlockchainStateReadyCallback = (blockchainStateResponse) => {
  // mainConsole.log('getBlockchainStateReadyCallback ', blockchainStateResponse);
  if (blockchainStateResponse.Error == 0) {
    blockchainStatus = `Blockchain State Received`;
    blockchainState = blockchainStateResponse.Result;
  } else {
    balanceStatus = 'Blockchain State Error ' + blockchainStateResponse.Error;
    blockchainState = blockchainStateResponse.Result;
  }

  renderApp();
};

const requestBlockchainState = () => {
  const stateUrl = getStateUrl();
  blockchainState = {};
  blockchainStatus = `Blockchain State Requested`;
  getJson(stateUrl, getBlockchainStateReadyCallback, getBlockchainStateErrorCallback);
};

const getConfirmations = () => {
  if (blockchainState) {
    if (blockchainState.height) {
      return blockchainState.height - blockchainLastActionHeight;
    } else {
      return 0;
    }
  }
};

const setBlockchainLastActionHeight = () => {
  if (blockchainState.height) {
    blockchainLastActionHeight = blockchainState.height;
  }
};

const copyMnemonicToClipboard = () => {
  clipboard.writeText(generatedMnemonic);
  alert(`copied to clipboard:\n${generatedMnemonic}`);
};

const copyToPrivateKeyClipboard = () => {
  clipboard.writeText(generatedPrivateKeyHex);
  alert(`copied to clipboard:\n${generatedPrivateKeyHex}`);
};

const clearGlobalData = () => {
  mainConsole.log('STARTED clearGlobalData');
  GuiUtils.setValue('privateKey', '');
  GuiUtils.setValue('mnemonic', '');
  GuiUtils.setValue('feeAmount', DEFAULT_FEE_SATS);
  GuiUtils.setValue('nodeUrl', '');

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
  mainConsole.log('SUCCESS clearGlobalData');
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

      // mainConsole.log('getUSDBalance', usd, balance, balanceStatus);
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

const getParsedProducerList = () => {
  return parsedProducerList;
};

const getProducerListStatus = () => {
  return producerListStatus;
};

const getParsedTransactionHistory = () => {
  return parsedTransactionHistory;
};

const getBlockchainState = () => {
  if (blockchainState) {
    return blockchainState;
  }
  return {};
};

const getBlockchainStatus = () => {
  return blockchainStatus;
};

const getTransactionHistoryStatus = () => {
  return transactionHistoryStatus;
};

const getSendToAddressStatuses = () => {
  return sendToAddressStatuses;
};

const getSendToAddressLinks = () => {
  return sendToAddressLinks;
};

const getSendAmount = () => {
  return sendAmount;
};

const getFeeAmountEla = () => {
  return feeAmountEla;
};

const getSendToAddress = () => {
  return sendToAddress;
};

const getFeeAmountSats = () => {
  return feeAmountSats;
};

const getSendHasFocus = () => {
  return sendHasFocus;
};

const setSendHasFocus = (_sendHasFocus) => {
  sendHasFocus = _sendHasFocus;
  // mainConsole.log('setSendHasFocus', sendHasFocus);
};

const getSendStep = () => {
  return sendStep;
};

const setSendStep = (_sendStep) => {
  sendStep = _sendStep;
};

const renderAppWrapper = () => {
  renderApp();
};

const getCurrentNetworkIx = () => {
  return currentNetworkIx;
}

exports.DEFAULT_FEE_SATS = DEFAULT_FEE_SATS;
exports.REST_SERVICES = REST_SERVICES;
exports.init = init;
exports.log = mainConsole.log;
exports.trace = mainConsole.trace;
exports.renderApp = renderAppWrapper;
exports.setAppClipboard = setAppClipboard;
exports.setAppDocument = setAppDocument;
exports.setRenderApp = setRenderApp;
exports.getLedgerDeviceInfo = getLedgerDeviceInfo;
exports.getMainConsole = getMainConsole;
exports.getPublicKeyFromLedger = getPublicKeyFromLedger;
exports.refreshBlockchainData = refreshBlockchainData;
exports.clearSendData = clearSendData;
exports.clearGlobalData = clearGlobalData;
exports.setPollForAllInfoTimer = setPollForAllInfoTimer;
exports.getPublicKeyFromMnemonic = getPublicKeyFromMnemonic;
exports.getPublicKeyFromPrivateKey = getPublicKeyFromPrivateKey;
exports.getAddress = getAddress;
exports.getELABalance = getELABalance;
exports.getUSDBalance = getUSDBalance;
exports.getParsedProducerList = getParsedProducerList;
exports.getProducerListStatus = getProducerListStatus;
exports.getParsedTransactionHistory = getParsedTransactionHistory;
exports.getBlockchainState = getBlockchainState;
exports.getConfirmations = getConfirmations;
exports.getBlockchainStatus = getBlockchainStatus;
exports.getTransactionHistoryStatus = getTransactionHistoryStatus;
exports.updateAmountAndFees = updateAmountAndFees;
exports.getSendToAddressStatuses = getSendToAddressStatuses;
exports.getSendToAddressLinks = getSendToAddressLinks;
exports.getSendAmount = getSendAmount;
exports.getFeeAmountEla = getFeeAmountEla;
exports.getSendToAddress = getSendToAddress;
exports.getFeeAmountSats = getFeeAmountSats;
exports.getSendHasFocus = getSendHasFocus;
exports.setSendHasFocus = setSendHasFocus;
exports.getSendStep = getSendStep;
exports.setSendStep = setSendStep;
exports.sendAmountToAddress = sendAmountToAddress;
exports.getRestService = getRestService;
exports.setRestService = setRestService;
exports.getCurrentNetworkIx = getCurrentNetworkIx;
