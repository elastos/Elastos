const assert = require('chai').assert;
const expect = require('chai').expect;

/**
curl elastos.coranos.io:20336 -H 'Content-Type: application/json' -H 'Accept:application/json' --data '{"method": "getrawtransaction", "params": {"txid":"88a261753512227b928caec4df29476c0ed7adda4518064acad0b03d62bd0df1","verbose":true}}' > voting/verbose-true-tx.json;
curl elastos.coranos.io:20336 -H 'Content-Type: application/json' -H 'Accept:application/json' --data '{"method": "getrawtransaction", "params": {"txid":"88a261753512227b928caec4df29476c0ed7adda4518064acad0b03d62bd0df1","verbose":false}}' > voting/verbose-false-tx.json;
*/

const fixtures = require('./TranscodeSignedVoteTxTestData.json');

const TxTranscoder = require('../scripts/TxTranscoder.js');
const TxSigner = require('../scripts/TxSigner.js');

const encodedTx = fixtures.encodedSignedTx;
const decodedTx = fixtures.decodedSignedTx;

const rawEncodedTx = Buffer.from(encodedTx, 'hex');
const rawDecodedTx = JSON.parse(JSON.stringify(decodedTx));
// console.log('expectedTx', expectedTx);

const lineSplit32 = (str) => {
  // return str.toUpperCase();
  return str.match(/.{1,32}/g).join('\n').toUpperCase();
};

describe('transcode-signed-vote-tx', function() {
  it('decodeTx detects non Buffer', function() {
    expect(() => TxTranscoder.decodeTx(encodedTx)).to.throw('encodedTx must be a Buffer');
  });
  it('encodeTx detects null includePrograms flag', function() {
    expect(() => TxTranscoder.encodeTx(rawDecodedTx)).to.throw('includePrograms is a required parameter.');
  });
  it('encodedTx decodes to decodedTx', function() {
    const expectedTx = decodedTx;
    // console.log('expectedTx', expectedTx);
    const actualTx = TxTranscoder.decodeTx(rawEncodedTx, true);
    // console.log('actualTx', actualTx);
    expect(actualTx).to.deep.equal(expectedTx);
  });
  it('decodedTx encodes to encodedTx', function() {
    const expectedTx = encodedTx;
    // console.log('expectedTx', expectedTx);
    const actualTx = TxTranscoder.encodeTx(rawDecodedTx, true);
    // console.log('actualTx', actualTx);

    // split so it's not all one long line of hex.
    if (actualTx.length < expectedTx.length) {
      const truncatedExpectedTx = expectedTx.slice(0, actualTx.length);
      expect(lineSplit32(actualTx)).to.deep.equal(lineSplit32(truncatedExpectedTx));
    }
    expect(lineSplit32(actualTx)).to.deep.equal(lineSplit32(expectedTx));
  });
});
