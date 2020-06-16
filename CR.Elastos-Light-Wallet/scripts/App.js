'use strict';

/** imports */
const bip39 = require('bip39');
const mainConsoleLib = require('console');
const BigNumber = require('bignumber.js');
const crypto = require('crypto');
const Parser = require('rss-parser');

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
const POLL_INTERVAL = 70000;

const JSON_TIMEOUT = 60000;

const LOG_LEDGER_POLLING = false;

const MAX_POLL_DATA_TYPE_IX = 5;

const PRIVATE_KEY_LENGTH = 64;

const EXPLORER = 'https://blockchain.elastos.org';

const RSS_FEED_URL = 'https://news.elastos.org/feed/';

const REST_SERVICES = [{
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

let refreshCandiatesFlag = true;

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

let parsedCandidateVoteList = {
  candidateVotes: [],
  lastVote: [],
};

let unspentTransactionOutputsStatus = 'No UTXOs Requested Yet';

const parsedUnspentTransactionOutputs = [];

let blockchainStatus = 'No Blockchain State Requested Yet';

let blockchainState = {};

let blockchainLastActionHeight = 0;

let parsedRssFeedStatus = 'No Rss Feed Requested Yet';

const parsedRssFeed = [];

let feeStatus = 'No Fee Requested Yet';

let fee = '';

let feeAccountStatus = 'No Fee Account Requested Yet';

let feeAccount = '';

let bannerStatus = '';

let bannerClass = '';

const DECIMAL_REGEX = new RegExp('^[0-9]+([,.][0-9]+)?$');

const TRAILING_ZERO_REGEX = new RegExp('^([0-9].[0-9]+?)0+$');

const mainConsole = new mainConsoleLib.Console(process.stdout, process.stderr);

let GuiToggles;

/** functions */
const init = (_GuiToggles) => {
  sendToAddressStatuses.push('No Send-To Transaction Requested Yet');

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
  const url = `${restService}/api/v3/history/${address}`;
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

const isValidDecimal = (testAmount) => {
  const isValid = DECIMAL_REGEX.test(testAmount);
  // mainConsole.log('isValidDecimal', 'testAmount', testAmount, isValid);
  return isValid;
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
  renderApp();
};

const refreshBlockchainData = () => {
  requestTransactionHistory();
  requestBalance();
  requestUnspentTransactionOutputs();
  requestBlockchainState();
  CoinGecko.requestPriceData();
  requestListOfProducers(true);
  requestListOfCandidateVotes();
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
    bannerStatus = message.message;
    bannerClass = 'bg_red color_white banner-look';
    GuiToggles.showAllBanners();
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
      if (refreshCandiatesFlag) {
        requestListOfProducers(false);
        requestListOfCandidateVotes();
      }
      requestRssFeed();
      requestFee();
      requestFeeAccount();
      pollDataTypeIx++;
      setPollForAllInfoTimer();
      break;
    case MAX_POLL_DATA_TYPE_IX:
      // only check every 10 seconds for a change in device status.
      pollDataTypeIx = 0;
      setTimeout(pollForData, POLL_INTERVAL);
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
  xhttp.onreadystatechange = function () {
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
  if (JSON_TIMEOUT != undefined) {
    xhttp.timeout = JSON_TIMEOUT;
  }
  xhttp.open('POST', url, true);
  xhttp.setRequestHeader('Content-Type', 'application/json');
  xhttp.setRequestHeader('Authorization', 'Basic RWxhVXNlcjpFbGExMjM=');

  // sendToAddressStatuses.push( `XMLHttpRequest: curl ${url} -H "Content-Type: application/json" -d '${jsonString}'` );

  xhttp.send(jsonString);
};

const getRssFeed = async (url, readyCallback, errorCallback) => {
  const parser = new Parser();
  try {
    const feed = await parser.parseURL(url);
    readyCallback(feed);
  } catch (error) {
    errorCallback(error);
  }
};

const getJson = (url, readyCallback, errorCallback) => {
  const xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function () {
    if (this.readyState == 4) {
      if (this.status == 200) {
        readyCallback(JSON.parse(this.response));
      } else {
        errorCallback({
          'status': this.status,
          'statusText': this.statusText,
          'response': this.response,
        });
      }
    }
  };
  if (JSON_TIMEOUT != undefined) {
    xhttp.timeout = JSON_TIMEOUT;
  }
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
  if (refreshCandiatesFlag) {
    requestListOfProducers(false);
    requestListOfCandidateVotes();
  }
};

const getPublicKeyFromMnemonic = () => {
  useLedgerFlag = false;
  isLoggedIn = true;
  const mnemonicElt = document.getElementById('mnemonic');
  const mnemonic = mnemonicElt.value;
  if (!bip39.validateMnemonic(mnemonic)) {
    bannerStatus = `Mnemonic is not valid.`;
    bannerClass = 'bg_red color_white banner-look';
    GuiToggles.showAllBanners();
    renderApp();
    return false;
  }
  const privateKey = Mnemonic.getPrivateKeyFromMnemonic(mnemonic);
  if (privateKey.length != PRIVATE_KEY_LENGTH) {
    bannerStatus = `Mnemonic must create a of length ${PRIVATE_KEY_LENGTH}, not ${privateKey.length}`;
    bannerClass = 'bg_red color_white banner-look';
    GuiToggles.showAllBanners();
    renderApp();
    return false;
  }
  const privateKeyElt = document.getElementById('privateKey');
  privateKeyElt.value = privateKey;
  publicKey = KeyTranscoder.getPublic(privateKey);
  requestBlockchainData();
  return true;
};

const getPublicKeyFromPrivateKey = () => {
  useLedgerFlag = false;
  isLoggedIn = true;
  const privateKeyElt = document.getElementById('privateKey');
  const privateKey = privateKeyElt.value;
  if (privateKey.length != PRIVATE_KEY_LENGTH) {
    bannerStatus = `Private key must be a hex encoded string of length ${PRIVATE_KEY_LENGTH}, not ${privateKey.length}`;
    bannerClass = 'bg_red color_white banner-look';
    GuiToggles.showAllBanners();
    renderApp();
    return false;
  }
  publicKey = KeyTranscoder.getPublic(privateKey);
  requestBlockchainData();
  return true;
};

const sendAmountToAddressErrorCallback = (error) => {
  sendToAddressStatuses.push(JSON.stringify(error));
  renderApp();
};

const sendAmountToAddressReadyCallback = (transactionJson) => {
  // mainConsole.log('sendAmountToAddressReadyCallback ' + JSON.stringify(transactionJson));
  if (transactionJson.status == 400) {
    sendToAddressStatuses.length = 0;
    const message = `Transaction Error.  Status:${transactionJson.status}  Result:${transactionJson.result}`;
    bannerStatus = message;
    bannerClass = 'bg_red color_white banner-look';
    sendToAddressStatuses.push(message);
  } else if (transactionJson.Error != 0) {
    sendToAddressStatuses.length = 0;
    const message = `Transaction Error.  Error:${transactionJson.Error}  Result:${transactionJson.Result}`;
    bannerStatus = message;
    bannerClass = 'bg_red color_white banner-look';
    sendToAddressStatuses.push(message);
  } else {
    sendToAddressStatuses.length = 0;
    const link = getTransactionHistoryLink(transactionJson.result);
    const elt = {};
    elt.txDetailsUrl = link;
    elt.txHash = transactionJson.result;
    sendToAddressStatuses.length = 0;
    const message = 'Transaction Successful.';
    bannerClass = 'bg_green color_white banner-look';
    sendToAddressStatuses.push(message);
    bannerStatus = message;
    sendToAddressLinks.push(elt);
    requestTransactionHistory();
  }
  GuiToggles.showAllBanners();
  setBlockchainLastActionHeight();
  renderApp();
};

const clearSendData = () => {
  // mainConsole.log('STARTED clearSendData');
  GuiUtils.setValue('sendAmount', '');
  GuiUtils.setValue('sendToAddress', '');
  GuiUtils.setValue('feeAmount', fee);
  sendAmount = '';
  feeAmountSats = fee;
  feeAmountEla = '';
  sendToAddressStatuses.length = 0;
  sendToAddressLinks.length = 0;
  // mainConsole.log('SUCCESS clearSendData');
};


const validateInputs = () => {
  sendAmount = GuiUtils.getValue('sendAmount');
  sendToAddress = GuiUtils.getValue('sendToAddress');

  if (sendToAddress.length == 0) {
    bannerStatus = `Address field is blank`;
    bannerClass = 'bg_red color_white banner-look';
    GuiToggles.showAllBanners();
    renderApp();
    return false;
  }

  if (!isValidDecimal(sendAmount)) {
    bannerStatus = `Amount: ${sendAmount} is not a number`;
    bannerClass = 'bg_red color_white banner-look';
    GuiToggles.showAllBanners();
    renderApp();
    return false;
  }
  const sendAmountSatsBn = BigNumber(sendAmount, 10).times(Asset.satoshis);
  const feeAmountSatsBn = BigNumber(feeAmountSats, 10);
  const balanceSatsBn = BigNumber(balance, 10).times(Asset.satoshis);
  if (sendAmountSatsBn.plus(feeAmountSatsBn).isGreaterThanOrEqualTo(balanceSatsBn)) {
    bannerStatus = `Amount: ${sendAmount} + Fees ${feeAmountSats} is greater than balance ${balance}`;
    bannerClass = 'bg_red color_white banner-look';
    GuiToggles.showAllBanners();
    renderApp();
    return false;
  }
  // GuiToggles.hideAllBanners();
  // renderApp();

  feeAmountEla = BigNumber(feeAmountSats, 10).dividedBy(Asset.satoshis).toString();
  // mainConsole.log('SUCCESS updateAmountAndFees');
  return true;
};

const updateAmountAndFees = () => {
  // mainConsole.log('STARTED updateAmountAndFees');

  sendAmount = GuiUtils.getValue('sendAmount');
  sendToAddress = GuiUtils.getValue('sendToAddress');
  feeAmountSats = GuiUtils.getValue('feeAmount');

  // mainConsole.log('INTERIM updateAmountAndFees',
  //   'sendAmount:', sendAmount,
  //   'sendToAddress:', sendToAddress,
  //   'feeAmountSats:', feeAmountSats,
  // );
  if (!isValidDecimal(feeAmountSats)) {
    bannerStatus = `Fees: ${feeAmountSats} is not a number`;
    bannerClass = 'bg_red color_white banner-look';
    GuiToggles.showAllBanners();
    renderApp();
    return false;
  }
  if(parsedTransactionHistory) {
    if(parsedTransactionHistory[0]) {
      if (parsedTransactionHistory[0].type == '*Sending') {
        bannerStatus = `Please wait for previous transaction to confirm.`;
        bannerClass = 'bg_red color_white banner-look';
        GuiToggles.showAllBanners();
        renderApp();
        return false;
      }
    }
  }

  const sendAmountSatsBn = BigNumber(sendAmount, 10).times(Asset.satoshis);
  const feeAmountSatsBn = BigNumber(feeAmountSats, 10);
  const balanceSatsBn = BigNumber(balance, 10).times(Asset.satoshis);
  if (sendAmountSatsBn.plus(feeAmountSatsBn).isGreaterThanOrEqualTo(balanceSatsBn)) {
    bannerStatus = `Amount: ${sendAmount} + Fees ${feeAmountSats} is greater than balance ${balance}`;
    bannerClass = 'bg_red color_white banner-look';
    GuiToggles.showAllBanners();
    renderApp();
    return false;
  }
  // GuiToggles.hideAllBanners();
  // renderApp();

  feeAmountEla = BigNumber(feeAmountSats, 10).dividedBy(Asset.satoshis).toString();
  // mainConsole.log('SUCCESS updateAmountAndFees');
  return true;
};

const showLedgerConfirmBanner = (size) => {
  bannerStatus = `Please review and sign transaction on Ledger (Tx size = ${size} bytes)`;
  mainConsole.log('STARTED showLedgerConfirmBanner', bannerStatus);
  bannerClass = 'landing-btnbg color_white banner-look';
  GuiToggles.showAllBanners();
  renderApp();
};

const hideLedgerConfirmBanner = () => {
  GuiToggles.hideAllBanners();
};

const getTxByteLength = (transactionHex) => {
  const transactionByteLength = Math.ceil(transactionHex.length / 2);
  return transactionByteLength;
};

const sendAmountToAddress = () => {
  const isValid = updateAmountAndFees();
  if (!isValid) {
    return;
  }

  const unspentTransactionOutputs = parsedUnspentTransactionOutputs;
  mainConsole.log('sendAmountToAddress.unspentTransactionOutputs ' + JSON.stringify(unspentTransactionOutputs));

  let encodedTx;

  if (useLedgerFlag) {
    const tx = TxFactory.createUnsignedSendToTx(unspentTransactionOutputs, sendToAddress, sendAmount, publicKey, feeAmountSats, feeAccount);
    const encodedUnsignedTx = TxTranscoder.encodeTx(tx, false);
    showLedgerConfirmBanner(getTxByteLength(encodedUnsignedTx));
    const sendAmountToAddressLedgerCallback = (message) => {
      if (LOG_LEDGER_POLLING) {
        mainConsole.log(`sendAmountToAddressLedgerCallback ${JSON.stringify(message)}`);
      }

      hideLedgerConfirmBanner();
      if (!message.success) {
        bannerStatus = `Send Error: ${message.message}`;
        bannerClass = 'bg_red color_white banner-look';
        GuiToggles.showAllBanners();
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
    const encodedTx = TxFactory.createSignedSendToTx(privateKey, unspentTransactionOutputs, sendToAddress, sendAmount, feeAmountSats, feeAccount);

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
  const txUrl = `${getRestService()}/api/v1/transaction`;
  const jsonString = `{"method": "sendrawtransaction", "data": "${encodedTx}"}`;

  // mainConsole.log('sendAmountToAddress.txUrl ' + txUrl);
  // mainConsole.log('sendAmountToAddress.encodedTx ' + JSON.stringify(encodedTx));

  const decodedTx = TxTranscoder.decodeTx(Buffer.from(encodedTx, 'hex'), true);

  // mainConsole.log('sendAmountToAddress.decodedTx ' + JSON.stringify(decodedTx));

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

const clearParsedProducerList = () => {
  parsedProducerList = {
    totalvotes: '-',
    totalcounts: '-',
    producersCandidateCount: 0,
    producers: [],
  };
}

const requestListOfProducersReadyCallback = (response, force) => {
  if (refreshCandiatesFlag) {
    producerListStatus = 'Producers Received';
  } else {
    if (force) {
      producerListStatus = 'Producers Refreshed';
    } else {
      if (parsedProducerList.producers.length > 0) {
        producerListStatus = 'Producers Refresh Paused';
        return;
      }
    }
  }

  // mainConsole.log('STARTED Producers Callback', response);
  clearParsedProducerList();
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

const requestListOfProducers = (force) => {
  if (force) {
    producerListStatus = 'Refreshing Candidates, Please Wait';
  } else {
    producerListStatus = 'Loading Candidates, Please Wait';
  }
  const txUrl = `${getRestService()}/api/v1/dpos/rank/height/0?state=active`;

  const requestListOfProducersReadyCallbackWrap = (response) => {
    requestListOfProducersReadyCallback(response, force);
  };

  renderApp();
  getJson(txUrl, requestListOfProducersReadyCallbackWrap, requestListOfProducersErrorCallback);
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

const selectActiveVotes = () => {
  if (parsedCandidateVoteList.lastVote.length > 0) {
    let activeVotes = new Set(parsedCandidateVoteList.lastVote);
    // let activeVotes = new Set(parsedCandidateVoteList.candidateVotes.map(e => e.ownerpublickey));
    parsedProducerList.producers.map(e => {
      if (activeVotes.has(e.ownerpublickey)) {
        if (!e.isCandidate) {
          e.isCandidate = true;
          parsedProducerList.producersCandidateCount++;
        }
      }
    });
  } else {
    bannerStatus = `Previous voting record not found.`;
    bannerClass = 'bg_red color_white banner-look';
    GuiToggles.showAllBanners();
  }
  renderApp();
};

const clearSelection = () => {
  parsedProducerList.producers.map(e => {
    if (e.isCandidate) {
      e.isCandidate = false;
      parsedProducerList.producersCandidateCount--;
    }
  });
  renderApp();
};

const requestListOfCandidateVotesErrorCallback = (response) => {
  mainConsole.log('ERRORED Candidate Votes Callback', response);
  const displayRawError = true;
  if (displayRawError) {
    candidateVoteListStatus = `Candidate Votes Error: ${JSON.stringify(response)}`;
  }
  renderApp();
};

const clearParsedCandidateVoteList = () => {
  parsedCandidateVoteList = {};
  parsedCandidateVoteList.candidateVotes = [];
  parsedCandidateVoteList.lastVote = [];
}

const requestListOfCandidateVotesReadyCallback = (response) => {
  candidateVoteListStatus = 'Candidate Votes Received';

  // mainConsole.log('STARTED Candidate Votes Callback', response);
  clearParsedCandidateVoteList();

  if (response.status !== 200) {
    candidateVoteListStatus = `Candidate Votes Error: ${JSON.stringify(response)}`;
  } else {
    if (response.result) {
      parsedCandidateVoteList.lastVote = response.result[0].Vote_Header.Nodes;

      response.result.forEach((candidateVote) => {
        // mainConsole.log('INTERIM Candidate Votes Callback', candidateVote);
        const header = candidateVote.Vote_Header;
        if (header.Is_valid === 'YES') {
          const body = candidateVote.Vote_Body;
          body.forEach((candidateVoteElt) => {
            const parsedCandidateVote = {};
            if (candidateVoteElt.Active == 1) {
              parsedCandidateVote.n = parsedCandidateVoteList.candidateVotes.length + 1;
              parsedCandidateVote.nickname = candidateVoteElt.Nickname;
              parsedCandidateVote.state = candidateVoteElt.State;
              // parsedCandidateVote.votes = candidateVoteElt.Votes;
              parsedCandidateVote.votes = header.Value;
              parsedCandidateVote.ownerpublickey = candidateVoteElt.Ownerpublickey;
              // mainConsole.log('INTERIM Candidate Votes Callback', parsedCandidateVote);
              parsedCandidateVoteList.candidateVotes.push(parsedCandidateVote);
            }
          });
        }
      });
    }
    // mainConsole.log('INTERIM Candidate Votes Callback', response.result);
  }
  // mainConsole.log('SUCCESS Candidate Votes Callback');

  renderApp();
};

const requestListOfCandidateVotes = () => {
  if (address !== undefined) {
    candidateVoteListStatus = 'Loading Votes, Please Wait';

    const txUrl = `${getRestService()}/api/v1/dpos/address/${address}?pageSize=1&pageNum=1`;
    // mainConsole.log('requestListOfCandidateVotes', txUrl);

    renderApp();
    getJson(txUrl, requestListOfCandidateVotesReadyCallback, requestListOfCandidateVotesErrorCallback);
  }
};

const sendVoteTx = () => {
  try {
    updateAmountAndFees();
    const unspentTransactionOutputs = parsedUnspentTransactionOutputs;
    // mainConsole.log('sendVoteTx.unspentTransactionOutputs ' + JSON.stringify(unspentTransactionOutputs));

    if (parsedTransactionHistory[0].type == '*Sending') {
      bannerStatus = `Please wait for previous transaction to confirm.`;
      bannerClass = 'bg_red color_white banner-look';
      GuiToggles.showAllBanners();
      renderApp();
      return;
    }

    if (parsedProducerList.producersCandidateCount === 0) {
      bannerStatus = 'No Candidates Selected.';
      bannerClass = 'bg_red color_white banner-look';
      GuiToggles.showAllBanners();
      renderApp();
      return;
    }

    if (balance == 0) {
      bannerStatus = 'You have insufficient ELA balance to vote.';
      bannerClass = 'bg_red color_white banner-look';
      GuiToggles.showAllBanners();
      renderApp();
      return;
    }

    if (!isValidDecimal(feeAmountSats)) {
      bannerStatus = `feeAmountSats ${feeAmountSats} is not a number`;
      bannerClass = 'bg_red color_white banner-look';
      GuiToggles.showAllBanners();
      renderApp();
      return;
    }

    const candidates = [];
    parsedProducerList.producers.forEach((parsedProducer) => {
      if (parsedProducer.isCandidate) {
        candidates.push(parsedProducer.ownerpublickey);
      }
    });

    // mainConsole.log('sendVoteTx.candidates ' + JSON.stringify(candidates));

    let encodedTx;

    // mainConsole.log('sendVoteTx.useLedgerFlag ' + JSON.stringify(useLedgerFlag));
    // mainConsole.log('sendVoteTx.unspentTransactionOutputs ' + JSON.stringify(unspentTransactionOutputs));
    if (useLedgerFlag) {
      if (unspentTransactionOutputs) {
        const tx = TxFactory.createUnsignedVoteTx(unspentTransactionOutputs, publicKey, feeAmountSats, candidates, feeAccount);
        const encodedUnsignedTx = TxTranscoder.encodeTx(tx, false);
        candidateVoteListStatus = `Voting for ${parsedProducerList.producersCandidateCount} candidates.`;
        showLedgerConfirmBanner(getTxByteLength(encodedUnsignedTx));
        const sendVoteLedgerCallback = (message) => {
          if (LOG_LEDGER_POLLING) {
            mainConsole.log(`sendVoteLedgerCallback ${JSON.stringify(message)}`);
          }
          mainConsole.log(`sendVoteLedgerCallback ${JSON.stringify(message)}`);
          hideLedgerConfirmBanner();
          if (!message.success) {
            candidateVoteListStatus = `Vote Error: ${message.message}`;
            bannerStatus = candidateVoteListStatus;
            bannerClass = 'bg_red color_white banner-look';
            GuiToggles.showAllBanners();
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
      const encodedTx = TxFactory.createSignedVoteTx(privateKey, unspentTransactionOutputs, feeAmountSats, candidates, feeAccount);

      // mainConsole.log('sendVoteTx.encodedTx ' + JSON.stringify(encodedTx));

      if (encodedTx == undefined) {
        return;
      }
      sendVoteCallback(encodedTx);
    }
  } catch (error) {
    mainConsole.trace(error);
  }
  renderApp();
};
// success: success,
// message: lastResponse,
// signature: signature

const sendVoteCallback = (encodedTx) => {
  const txUrl = `${getRestService()}/api/v1/transaction`;
  const jsonString = `{"method": "sendrawtransaction", "data": "${encodedTx}"}`;

  // mainConsole.log('sendVoteCallback.encodedTx ' + JSON.stringify(encodedTx));

  const decodedTx = TxTranscoder.decodeTx(Buffer.from(encodedTx, 'hex'), true);

  // mainConsole.log('sendVoteCallback.decodedTx ' + JSON.stringify(decodedTx));

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
    candidateVoteListStatus = `Vote Error: ${transactionJson.Error} ${transactionJson.Result}`;
    bannerStatus = candidateVoteListStatus;
    bannerClass = 'bg_red color_white banner-look';
  } else {
    candidateVoteListStatus = `Vote Success TX: ${transactionJson.Result}`;
    bannerStatus = candidateVoteListStatus;
    bannerClass = 'bg_green color_white banner-look';
    requestTransactionHistory();
  }
  GuiToggles.showAllBanners();
  renderApp();
};

const getTransactionHistoryErrorCallback = (response) => {
  transactionHistoryStatus = `History Error: ${JSON.stringify(response)}`;
  renderApp();
};

const formatTxValue = (value) => {
  const elaFloat = parseInt(value) / 100000000;
  const elaDisplay = elaFloat.toFixed(8).replace(TRAILING_ZERO_REGEX, '$1');
  return elaDisplay;
}

const getTransactionHistoryReadyCallback = (transactionHistory) => {
  // mainConsole.log('getTransactionHistoryReadyCallback ' + JSON.stringify(transactionHistory));
  transactionHistoryStatus = 'History Received';
  parsedTransactionHistory.length = 0;
  if (transactionHistory.result !== undefined) {
    if (transactionHistory.result.History !== undefined) {
      transactionHistory.result.History.forEach((tx, txIx) => {
        let time = formatDate(new Date(tx.CreateTime * 1000));
        if (tx.CreateTime == 0) {
          time = formatDate(new Date());
        }
        const parsedTransaction = {};
        parsedTransaction.sortTime = tx.CreateTime;
        if (tx.Status == 'pending' && tx.CreateTime == 0) {
          parsedTransaction.sortTime = Math.floor(new Date() / 1000);
        }
        // mainConsole.log('parsedTransaction.sortTime', tx.Status, parsedTransaction.sortTime);
        parsedTransaction.n = txIx;
        parsedTransaction.type = tx.Type;
        if (tx.Type == 'income') {
          parsedTransaction.type = 'Received';
        }
        if (tx.Type == 'spend') {
          parsedTransaction.type = 'Sent';
        }
        if (tx.Type == 'spend' && tx.Status == 'pending') {
          parsedTransaction.type = '*Sending';
        }
        if (tx.Type == 'income' && tx.Status == 'pending') {
          parsedTransaction.type = '*Receiving';
        }
        parsedTransaction.valueSat = tx.Value;
        parsedTransaction.value = formatTxValue(tx.Value);
        parsedTransaction.address = tx.Address;
        parsedTransaction.txHash = tx.Txid;
        parsedTransaction.txHashWithEllipsis = tx.Txid;
        if (parsedTransaction.txHashWithEllipsis.length > 20) {
          parsedTransaction.txHashWithEllipsis = parsedTransaction.txHashWithEllipsis.substring(0, 20) + '...';
        }
        parsedTransaction.txDetailsUrl = getTransactionHistoryLink(tx.Txid);
        parsedTransaction.time = time;
        parsedTransactionHistory.push(parsedTransaction);
      });
    }
  }

  parsedTransactionHistory.sort((a, b) => {
    return b.sortTime - a.sortTime;
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

const copyAddressToClipboard = () => {
  appClipboard.writeText(address);
  bannerStatus = `Copied to clipboard:\n${address}`;
  bannerClass = 'bg_green color_white banner-look';
  GuiToggles.showAllBanners();
  renderApp();
};

const copyMnemonicToClipboard = () => {
  appClipboard.writeText(generatedMnemonic);
  bannerStatus = `copied to clipboard:\n${generatedMnemonic}`;
  bannerClass = 'bg_green color_white banner-look';
  GuiToggles.showAllBanners();
  renderApp();
};

const copyPrivateKeyToClipboard = () => {
  appClipboard.writeText(generatedPrivateKeyHex);
  bannerStatus = `copied to clipboard:\n${generatedPrivateKeyHex}`;
  bannerClass = 'bg_green color_white banner-look';
  GuiToggles.showAllBanners();
  renderApp();
};

const verifyLedgerBanner = () => {
  if (useLedgerFlag) {
    bannerStatus = `Please verify that this address:\n${address} corresponds to the address on your Ledger device`;
    bannerClass = 'landing-btnbg color_white banner-look';
  } else {
    bannerStatus = `No Ledger Device Connected`;
    bannerClass = 'landing-btnbg color_white banner-look';
  }

  GuiToggles.showAllBanners();
  renderApp();
};

const clearGlobalData = () => {
  // mainConsole.log('STARTED clearGlobalData');
  GuiUtils.setValue('privateKey', '');
  GuiUtils.setValue('mnemonic', '');
  GuiUtils.setValue('feeAmount', fee);
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

  parsedRssFeed.length = 0;
  parsedRssFeedStatus = 'No Rss Feed Requested Yet';

  feeStatus = 'No Fee Requested Yet';
  fee = '';
  feeAccountStatus = 'No Fee Account Requested Yet';
  feeAccount = '';

  bannerStatus = '';
  bannerClass = '';

  clearParsedProducerList();
  clearParsedCandidateVoteList();

  renderApp();
  // mainConsole.log('SUCCESS clearGlobalData');
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

const setRefreshCandiatesFlag = (_refreshCandiatesFlag) => {
  refreshCandiatesFlag = _refreshCandiatesFlag;
  // mainConsole.log('refreshCandiatesFlag', refreshCandiatesFlag);
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
};

const getCandidateVoteListStatus = () => {
  return candidateVoteListStatus;
};

const getParsedCandidateVoteList = () => {
  return parsedCandidateVoteList;
};

const getAddressOrBlank = () => {
  const address = getAddress();
  if (address != undefined) {
    return address;
  }
  return '';
};


const requestRssFeed = async () => {
  parsedRssFeedStatus = 'Rss Feed Requested';
  getRssFeed(RSS_FEED_URL, getRssFeedReadyCallback, getRssFeedErrorCallback);
};

const getRssFeedErrorCallback = (error) => {
  mainConsole.log('getRssFeedErrorCallback ', error);
  parsedRssFeedStatus = `Rss Feed Error ${error.message}`;

  renderApp();
};

const getRssFeedReadyCallback = (response) => {
  parsedRssFeedStatus = 'Rss Feed Received';
  parsedRssFeed.length = 0;

  // mainConsole.log('getRssFeedReadyCallback ', response);

  if ((response.items != undefined) && (response.items != null)) {
    response.items.forEach((item, itemIx) => {
      parsedRssFeed.push(item);
    });
  }

  renderApp();
};

const requestFee = async () => {
  const feeUrl = `${getRestService()}/api/v1/fee`;
  feeStatus = 'Fee Requested';
  getJson(feeUrl, getFeeReadyCallback, getFeeErrorCallback);
};

const getFeeErrorCallback = (error) => {
  // mainConsole.log('getFeeErrorCallback ', error);
  feeStatus = `Rss Feed Error ${error.message}`;
  fee = '';
  renderApp();
};

const getFeeReadyCallback = (response) => {
  feeStatus = 'Fee Received';
  fee = response.result.toString();
  // mainConsole.log('getFeeReadyCallback ', response, fee);
  renderApp();
};


const requestFeeAccount = async () => {
  const feeAccountUrl = `${getRestService()}/api/v1/node/reward/address`;
  feeAccountStatus = 'Fee Account Requested';
  // mainConsole.log('requestFeeAccount ', feeAccountStatus);
  getJson(feeAccountUrl, getFeeAccountReadyCallback, getFeeAccountErrorCallback);
};

const getFeeAccountErrorCallback = (error) => {
  // mainConsole.log('getFeeErrorCallback ', error);
  feeAccountStatus = `Rss Account Feed Error ${error.message}`;
  feeAccount = '';
  renderApp();
};

const getFeeAccountReadyCallback = (response) => {
  feeAccountStatus = 'Fee Account Received';
  feeAccount = response.result;
  // mainConsole.log('getFeeAccountReadyCallback ', response, feeAccount);
  renderApp();
};

const getParsedRssFeed = () => {
  return parsedRssFeed;
};

const getBannerStatus = () => {
  return bannerStatus;
};

const getBannerClass = () => {
  return bannerClass;
};

const getFee = () => {
  return fee;
};

const generatePrivateKeyHex = () => {
  generatedPrivateKeyHex = crypto.randomBytes(32).toString('hex');
};

const getGeneratedPrivateKeyHex = () => {
  return generatedPrivateKeyHex;
};

const generateMnemonic = () => {
  generatedMnemonic = bip39.entropyToMnemonic(crypto.randomBytes(16).toString('hex'));
};

const getGeneratedMnemonic = () => {
  return generatedMnemonic;
};

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
exports.changeNodeUrl = changeNodeUrl;
exports.resetNodeUrl = resetNodeUrl;
exports.getCandidateVoteListStatus = getCandidateVoteListStatus;
exports.getParsedCandidateVoteList = getParsedCandidateVoteList;
exports.toggleProducerSelection = toggleProducerSelection;
exports.sendVoteTx = sendVoteTx;
exports.getAddressOrBlank = getAddressOrBlank;
exports.getParsedRssFeed = getParsedRssFeed;
exports.getBannerStatus = getBannerStatus;
exports.getBannerClass = getBannerClass;
exports.getFee = getFee;
exports.generateMnemonic = generateMnemonic;
exports.getGeneratedMnemonic = getGeneratedMnemonic;
exports.copyMnemonicToClipboard = copyMnemonicToClipboard;
exports.generatePrivateKeyHex = generatePrivateKeyHex;
exports.getGeneratedPrivateKeyHex = getGeneratedPrivateKeyHex;
exports.copyPrivateKeyToClipboard = copyPrivateKeyToClipboard;
exports.copyAddressToClipboard = copyAddressToClipboard;
exports.setRefreshCandiatesFlag = setRefreshCandiatesFlag;
exports.requestListOfProducers = requestListOfProducers;
exports.requestListOfCandidateVotes = requestListOfCandidateVotes;
exports.verifyLedgerBanner = verifyLedgerBanner;
exports.formatTxValue = formatTxValue;
exports.selectActiveVotes = selectActiveVotes;
exports.clearSelection = clearSelection;
exports.validateInputs = validateInputs;
