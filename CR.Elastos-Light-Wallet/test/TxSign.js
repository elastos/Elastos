const assert = require('chai').assert;
const expect = require('chai').expect;

const txFixtures = require('./TranscodeSignedTxTestData.json');
const keyFixtures = require('./TranscodeKeyTestData.json');

const TxTranscoder = require('../scripts/TxTranscoder.js');
const TxSigner = require('../scripts/TxSigner.js');
const KeyTranscoder = require('../scripts/KeyTranscoder.js');

const decodedTx = txFixtures.decodedSignedTx;
const decodedTxForSigning = txFixtures.decodedTxForSigning;
const encodedSignedTx = txFixtures.encodedSignedTx;
const encodedTxForSigning = txFixtures.encodedTxForSigning;
const rawEncodedSignedTx = Buffer.from(encodedSignedTx, 'hex');

const rawDecodedTx = JSON.parse(JSON.stringify(decodedTx));
const rawDecodedTxForSigning = JSON.parse(JSON.stringify(decodedTxForSigning));
// console.log('expectedTx', expectedTx);
const privateKey = keyFixtures.privateKey;

const lineSplit32 = (str) => {
  // return str.toUpperCase();
  return str.match(/.{1,32}/g).join('\n').toUpperCase();
};

describe('tx-sign', function() {
  it('decodedSignedTx.length encodes to encodedSignedTx.length', function() {
    const expectedTx = encodedSignedTx;
    const tx = TxTranscoder.decodeTx(rawEncodedSignedTx, true);
    const publicKey = KeyTranscoder.getPublic(privateKey);
    const signature = TxSigner.getSignature(tx, privateKey);
    const actualTx = TxSigner.addSignatureToTx(rawDecodedTxForSigning, publicKey, signature);
    expect(expectedTx.length).to.equal(actualTx.length);
  });
  it('HEX OLD decodedSignedTx encodes to encodedSignedTx', function() {
    const expectedTx = encodedSignedTx;
    const actualTx = TxSigner.signTx(rawDecodedTxForSigning, privateKey);
    // split so it's not all one long line of hex.
    expect(lineSplit32(expectedTx)).to.deep.equal(lineSplit32(actualTx));
  });
  it('HEX decodedSignedTx encodes to encodedSignedTx', function() {
    const expectedTx = encodedSignedTx;
    const publicKey = KeyTranscoder.getPublic(privateKey);
    const signature = TxSigner.getSignature(rawDecodedTxForSigning, privateKey);
    const actualTx = TxSigner.addSignatureToTx(rawDecodedTxForSigning, publicKey, signature);

    // split so it's not all one long line of hex.
    expect(lineSplit32(expectedTx)).to.deep.equal(lineSplit32(actualTx));
  });
  it('JSON decodedSignedTx encodes to encodedSignedTx', function() {
    const expectedTx = decodedTx;
    const publicKey = KeyTranscoder.getPublic(privateKey);
    const signature = TxSigner.getSignature(expectedTx, privateKey);
    const actualSignedTx = Buffer.from(TxSigner.addSignatureToTx(rawDecodedTx, publicKey, signature), 'hex');
    const actualTx = TxTranscoder.decodeTx(actualSignedTx, true);
    // console.log('actualTx', actualTx.Programs);

    // split so it's not all one long line of hex.
    expect(expectedTx).to.deep.equal(actualTx);
  });
});
