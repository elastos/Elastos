const assert = require('chai').assert;
const expect = require('chai').expect;

const keyFixtures = require('./TranscodeKeyTestData.json');
const txFactoryFixtures = require('./TxFactoryTestData.json');

const TxFactory = require('../scripts/TxFactory.js');
const KeyTranscoder = require('../scripts/KeyTranscoder.js');
const AddressTranscoder = require('../scripts/AddressTranscoder.js');
const TxTranscoder = require('../scripts/TxTranscoder.js');

const privateKey = keyFixtures.privateKey;
const publicKey = KeyTranscoder.getPublic(privateKey);
const sendToAddress = AddressTranscoder.getAddressFromPublicKey(publicKey);
const sendAmount = '1';
const feeAmount = '500';
const feeAccount = 'EQNJEA8XhraX8a6SBq98ENU5QSW6nvgSHJ';

const unspentTransactionOutputs = txFactoryFixtures.unspentTransactionOutputs;

unspentTransactionOutputs.forEach((utxo) => {
  TxFactory.updateValueSats(utxo);
});

const encodedSignedTx = txFactoryFixtures.encodedSignedTx;

const encodeDecodeTx1 = txFactoryFixtures.encodeDecodeTx1;

describe('tx-factory', function() {
  it('encoded signed tx matches expected', function() {
    const expectedTx = encodedSignedTx;

    const actualTx = TxFactory.createSignedSendToTx(privateKey, unspentTransactionOutputs, sendToAddress, sendAmount, feeAmount, feeAccount);

    expect(expectedTx).to.deep.equal(actualTx);
  });
  it('signed tx matches expected', function() {
    const expectedTx = TxTranscoder.decodeTx(Buffer.from(encodedSignedTx, 'hex'), true);

    const actualTx = TxTranscoder.decodeTx(Buffer.from(TxFactory.createSignedSendToTx(privateKey, unspentTransactionOutputs, sendToAddress, sendAmount, feeAmount, feeAccount), 'hex'), true);

    expect(expectedTx).to.deep.equal(actualTx);
  });
  it('encode decode 1 works without error', function() {
    const expectedTx = encodeDecodeTx1;

    const actualTx = TxTranscoder.decodeTx(Buffer.from(TxTranscoder.encodeTx(encodeDecodeTx1, true), 'hex'), true);

    expect(expectedTx).to.deep.equal(actualTx);
  });
});
