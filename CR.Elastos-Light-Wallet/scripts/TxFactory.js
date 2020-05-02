'use strict';

const KeyTranscoder = require('./KeyTranscoder.js');
const TxSigner = require('./TxSigner.js');
const AddressTranscoder = require('./AddressTranscoder.js');
const Asset = require('./Asset.js');
const BigNumber = require('bignumber.js');
const mainConsoleLib = require('console');

const mainConsole = new mainConsoleLib.Console(process.stdout, process.stderr);

/* eslint-disable */
const ZERO = BigNumber(0, 10);

const FEE_SATS = BigNumber(100, 10);
/* eslint-enable */

const createSignedSendToTx = (privateKey, unspentTransactionOutputs, sendToAddress, sendAmount, feeAmountSats, feeAccount) => {
  if (Number.isNaN(sendAmount)) {
    throw new Error(`sendAmount ${sendAmount} is not a number`);
  }
  if (Number.isNaN(feeAmountSats)) {
    throw new Error(`feeAmountSats ${feeAmountSats} is not a number`);
  }
  const publicKey = KeyTranscoder.getPublic(privateKey);
  const tx = createUnsignedSendToTx(unspentTransactionOutputs, sendToAddress, sendAmount, publicKey, feeAmountSats, feeAccount);
  const signature = TxSigner.getSignature(tx, privateKey);
  const encodedSignedTx = TxSigner.addSignatureToTx(tx, publicKey, signature);

  // console.log('createSignedSendToTx.signedTx ' + JSON.stringify(tx));

  // console.log('createSignedSendToTx.encodedSignedTx ' + JSON.stringify(encodedSignedTx));
  return encodedSignedTx;
};

const createUnsignedSendToTx = (unspentTransactionOutputs, sendToAddress, sendAmount, publicKey, feeAmountSats, feeAccount) => {
  if (unspentTransactionOutputs == undefined) {
    throw new Error(`unspentTransactionOutputs is undefined`);
  }
  if (sendToAddress == undefined) {
    throw new Error(`sendToAddress is undefined`);
  }
  if (sendAmount == undefined) {
    throw new Error(`sendAmount is undefined`);
  }
  if (publicKey == undefined) {
    throw new Error(`publicKey is undefined`);
  }
  if (feeAmountSats == undefined) {
    throw new Error(`feeAmount is undefined`);
  }
  if (feeAccount == undefined) {
    throw new Error(`feeAccount is undefined`);
  }
  /* eslint-disable */
  const sendAmountSats = BigNumber(sendAmount, 10).times(Asset.satoshis);
  /* eslint-enable */
  return createUnsignedSendToTxSats(unspentTransactionOutputs, sendToAddress, sendAmountSats, publicKey, feeAmountSats, feeAccount);
};

const createUnsignedSendToTxSats = (unspentTransactionOutputs, sendToAddress, sendAmountSats, publicKey, feeAmountStr, feeAccount) => {
  // mainConsole.log('STARTED createUnsignedSendToTxSats');
  if (unspentTransactionOutputs == undefined) {
    throw new Error(`unspentTransactionOutputs is undefined`);
  }
  if (sendToAddress == undefined) {
    throw new Error(`sendToAddress is undefined`);
  }
  if (sendAmountSats == undefined) {
    throw new Error(`sendAmount is undefined`);
  }
  if (publicKey == undefined) {
    throw new Error(`publicKey is undefined`);
  }
  if (feeAmountStr == undefined) {
    throw new Error(`feeAmountStr is undefined`);
  }
  if (feeAccount == undefined) {
    throw new Error(`feeAccount is undefined`);
  }
  // mainConsole.log('createUnsignedSendToTxSats.unspentTransactionOutputs ' + JSON.stringify(unspentTransactionOutputs));
  // console.log('createUnsignedSendToTx.sendToAddress ' + JSON.stringify(sendToAddress));
  // console.log('createUnsignedSendToTx.sendAmount ' + JSON.stringify(sendAmount));

  // console.log('createUnsignedSendToTx.publicKey ' + JSON.stringify(publicKey));
  const address = AddressTranscoder.getAddressFromPublicKey(publicKey);
  // console.log('createUnsignedSendToTx.address ' + JSON.stringify(address));

  const tx = {};
  tx.TxType = 2;
  tx.LockTime = 0;
  tx.PayloadVersion = 0;
  tx.TxAttributes = [];
  tx.UTXOInputs = [];
  tx.Outputs = [];
  tx.Programs = [];

  {
    const txAttribute = {};
    txAttribute.Usage = 0;
    txAttribute.Data = '30';
    tx.TxAttributes.push(txAttribute);
  }

  /* eslint-disable */
  const feeAmountSats = BigNumber(feeAmountStr, 10);

  /* eslint-enable */

  mainConsole.log(`createUnsignedSendToTx.inputValueSats[${sendAmountSats}]`);

  const sendAmountAndFeeAmountSats = sendAmountSats.plus(feeAmountSats).plus(FEE_SATS);

  mainConsole.log(`createUnsignedSendToTx.FEE_SATS[${FEE_SATS}]`);

  mainConsole.log(`createUnsignedSendToTx.feeAmountSats[${feeAmountSats}]`);

  // console.log(`createUnsignedSendToTx.sendAmountAndFeeAmountSats[${sendAmountAndFeeAmountSats}]`);

  /* eslint-disable */
  let inputValueSats = BigNumber(0, 10);
  /* eslint-enable */
  const usedUtxos = new Set();

  unspentTransactionOutputs.forEach((utxo) => {
    if (inputValueSats.isLessThan(sendAmountAndFeeAmountSats)) {
      if (utxo.valueSats.isGreaterThan(ZERO)) {
        const utxoInput = {};
        utxoInput.TxId = utxo.Txid.toUpperCase();
        utxoInput.ReferTxOutputIndex = utxo.Index;

        // console.log(`createUnsignedSendToTx.utxoInput[${tx.UTXOInputs.length}] ${JSON.stringify(utxo)}`);

        const utxoInputStr = JSON.stringify(utxoInput);
        if (!usedUtxos.has(utxoInputStr)) {
          utxoInput.Sequence = tx.UTXOInputs.length;
          tx.UTXOInputs.push(utxoInput);
          inputValueSats = inputValueSats.plus(utxo.valueSats);
          usedUtxos.add(utxoInputStr);
        }
      }
    }
  });

  mainConsole.log(`createUnsignedSendToTx.sendAmountAndFeeAmountSats[${sendAmountAndFeeAmountSats}]`);

  mainConsole.log(`createUnsignedSendToTx.inputValueSats[${inputValueSats}]`);

  const changeValueSats = inputValueSats.minus(sendAmountAndFeeAmountSats);

  mainConsole.log(`createUnsignedSendToTx.changeValueSats[${changeValueSats}]`);

  mainConsole.log(`createUnsignedSendToTx.feeAmountSats[${feeAmountSats}]`);

  {
    const sendOutput = {};
    sendOutput.AssetID = Asset.elaAssetId;
    sendOutput.Value = sendAmountSats.toString(10);
    sendOutput.OutputLock = 0;
    sendOutput.Address = sendToAddress;
    tx.Outputs.push(sendOutput);
  }
  {
    const changeOutput = {};
    changeOutput.AssetID = Asset.elaAssetId;
    changeOutput.Value = changeValueSats.toString(10);
    changeOutput.OutputLock = 0;
    changeOutput.Address = address;
    tx.Outputs.push(changeOutput);
  }
  {
    const feeOutput = {};
    feeOutput.AssetID = Asset.elaAssetId;
    feeOutput.Value = feeAmountSats.toString(10);
    feeOutput.OutputLock = 0;
    feeOutput.Address = feeAccount;
    tx.Outputs.push(feeOutput);
  }

  if (changeValueSats.isLessThan(ZERO)) {
    mainConsole.log('FAILURE createUnsignedSendToTxSats', changeValueSats, tx);
    return undefined;
  }

  tx.Programs = [];

  // mainConsole.log('SUCCESS createUnsignedSendToTxSats', tx);

  return tx;
};

const updateValueSats = (utxo, utxoIx) => {
  /* eslint-disable */
  const valueBigNumber = BigNumber(utxo.Value, 10);
  /* eslint-enable */
  utxo.utxoIx = utxoIx;
  utxo.valueSats = valueBigNumber.times(Asset.satoshis);
};

const createUnsignedVoteTx = (unspentTransactionOutputs, publicKey, feeAmountSats, candidates, feeAccount) => {
  if (unspentTransactionOutputs == undefined) {
    throw new Error(`unspentTransactionOutputs is undefined`);
  }
  if (publicKey == undefined) {
    throw new Error(`publicKey is undefined`);
  }
  if (feeAmountSats == undefined) {
    throw new Error(`feeAmountSats is undefined`);
  }
  if (candidates == undefined) {
    throw new Error(`candidates is undefined`);
  }
  if (feeAccount == undefined) {
    throw new Error(`feeAccount is undefined`);
  }
  const sendToAddress = AddressTranscoder.getAddressFromPublicKey(publicKey);
  /* eslint-disable */
  let sendAmountSats = BigNumber(0, 10);
  /* eslint-enable */

  const usedUtxos = new Set();
  unspentTransactionOutputs.forEach((utxo) => {
    if (utxo.valueSats.isGreaterThan(ZERO)) {
      const utxoInput = {};
      utxoInput.TxId = utxo.Txid.toUpperCase();
      utxoInput.ReferTxOutputIndex = utxo.Index;
      const utxoInputStr = JSON.stringify(utxoInput);
      if (!usedUtxos.has(utxoInputStr)) {
        sendAmountSats = sendAmountSats.plus(utxo.valueSats);
        usedUtxos.add(utxoInputStr);
      }
    }
  });
  sendAmountSats = sendAmountSats.minus(feeAmountSats).minus(FEE_SATS);

  // mainConsole.log('createUnsignedSendToTxSats', unspentTransactionOutputs, sendToAddress, sendAmountSats, publicKey, feeAmountSats, feeAccount);

  const tx = createUnsignedSendToTxSats(unspentTransactionOutputs, sendToAddress, sendAmountSats, publicKey, feeAmountSats, feeAccount);
  tx.Version = 9;
  const voteOutput = tx.Outputs[0];
  voteOutput.Type = 1;
  voteOutput.Payload = {};
  voteOutput.Payload.Version = 0;
  voteOutput.Payload.Contents = [];

  const Content = {};
  voteOutput.Payload.Contents.push(Content);
  Content.Votetype = 0;
  Content.Candidates = [];

  candidates.forEach((candidate) => {
    Content.Candidates.push(candidate);
  });
  return tx;
};

const createSignedVoteTx = (privateKey, unspentTransactionOutputs, feeAmountSats, candidates, feeAccount) => {
  const publicKey = KeyTranscoder.getPublic(privateKey);
  const tx = createUnsignedVoteTx(unspentTransactionOutputs, publicKey, feeAmountSats, candidates, feeAccount);
  const signature = TxSigner.getSignature(tx, privateKey);
  const encodedSignedTx = TxSigner.addSignatureToTx(tx, publicKey, signature);

  // console.log('createSignedSendToTx.signedTx ' + JSON.stringify(tx));

  // console.log('createSignedSendToTx.encodedSignedTx ' + JSON.stringify(encodedSignedTx));
  return encodedSignedTx;
};

exports.createUnsignedSendToTx = createUnsignedSendToTx;
exports.createSignedSendToTx = createSignedSendToTx;
exports.updateValueSats = updateValueSats;
exports.createUnsignedVoteTx = createUnsignedVoteTx;
exports.createSignedVoteTx = createSignedVoteTx;
