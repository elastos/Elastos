'use strict';

const SmartBuffer = require('smart-buffer').SmartBuffer;

const TxTranscoder = require('./TxTranscoder.js');
const KeyTranscoder = require('./KeyTranscoder.js');
const AddressTranscoder = require('./AddressTranscoder.js');
const BufferSigner = require('./BufferSigner.js');

const getSignature = (tx, privateKey) => {
  // console.log('getSignature.tx',tx);
  // console.log('getSignature.privateKey',privateKey);
  const encodedTxHex = TxTranscoder.encodeTx(tx, false);

  // console.log('getSignature.encodedTxHex',encodedTxHex);

  const signatureHex = BufferSigner.sign(encodedTxHex, privateKey);

  const signature = Buffer.from(signatureHex, 'hex');
  return signature;
};

const addSignatureToTx = (tx, publicKey, signature) => {
  const signatureParameter = new SmartBuffer();
  signatureParameter.writeInt8(signature.length);
  signatureParameter.writeBuffer(signature);
  const signatureParameterHex = signatureParameter.toString('hex').toUpperCase();

  const publicKeyRaw = Buffer.from(publicKey, 'hex');

  const code = AddressTranscoder.getSingleSignatureRedeemScript(publicKeyRaw, 1);

  const Program = {};
  Program.Code = code;
  Program.Parameter = signatureParameterHex;

  tx.Programs = [];
  tx.Programs.push(Program);

  // console.log('signTx.tx[1]',tx);

  return TxTranscoder.encodeTx(tx, true);
};

exports.addSignatureToTx = addSignatureToTx;
exports.getSignature = getSignature;

const signTx = (tx, privateKey) => {
  // console.log('signTx.tx[0]',tx);
  const signature = getSignature(tx, privateKey);
  const publicKey = KeyTranscoder.getPublic(privateKey);
  return addSignatureToTx(tx, publicKey, signature);
};

exports.signTx = signTx;
