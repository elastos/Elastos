const assert = require('chai').assert;
const expect = require('chai').expect;

const TxSigner = require('../scripts/TxSigner.js');
const BufferSigner = require('../scripts/BufferSigner.js');

const LedgerCommTestData = require('./LedgerCommTestData.json');
const TxTranscoder = require('../scripts/TxTranscoder.js');

const getActualSignature = () => {
  const tx = TxTranscoder.decodeTx(Buffer.from(LedgerCommTestData.tx, 'hex'), false);
  const actualSignature = TxSigner.getSignature(tx, LedgerCommTestData.privateKey).toString('hex').toUpperCase();
  return actualSignature;
};

const getActualTxRawHash = () => {
  const hash = BufferSigner.getHash(LedgerCommTestData.tx);
  return hash.toString('hex').toUpperCase();
};

const encodeDecodeTx = () => {
  const tx = TxTranscoder.decodeTx(Buffer.from(LedgerCommTestData.tx, 'hex'), false);
  const encodedTxHex = TxTranscoder.encodeTx(tx, false);
  return encodedTxHex;
};

const getActualTxHash = () => {
  const encodedTxHex = encodeDecodeTx();
  const hash = BufferSigner.getHash(encodedTxHex);
  return hash.toString('hex').toUpperCase();
};

describe('LedgerComm.sign', function() {
  it('LedgerComm.tx encode decode works', function() {
    const expectedTx = LedgerCommTestData.tx;
    const actualTx = encodeDecodeTx();
    expect(expectedTx).to.equal(actualTx);
  });
  it('LedgerComm.sign tx hash matches expected', function() {
    const expectedTxHash = LedgerCommTestData.txHash;
    const actualTxHash = getActualTxHash();
    expect(expectedTxHash).to.equal(actualTxHash);
  });
  it('LedgerComm.sign signature length matches expected', function() {
    const expectedSignature = LedgerCommTestData.signature;
    const actualSignature = getActualSignature();
    expect(expectedSignature.length).to.equal(actualSignature.length);
  });
  it('LedgerComm.sign signature matches expected', function() {
    const expectedSignature = LedgerCommTestData.signature;
    const actualSignature = getActualSignature();
    expect(expectedSignature).to.equal(actualSignature);
  });
});
