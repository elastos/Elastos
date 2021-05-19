const assert = require('chai').assert;
const expect = require('chai').expect;

const fixtures = require('./TranscodeUnsignedTxTestData.json');

const TxTranscoder = require('../scripts/TxTranscoder.js');
const TxSigner = require('../scripts/TxSigner.js');

const encodedTx = fixtures.encodedTx;
const decodedTx = fixtures.decodedTx;

const rawEncodedTx = Buffer.from(encodedTx, 'hex');
const rawDecodedTx = JSON.parse(JSON.stringify(decodedTx));
// console.log('expectedTx', expectedTx);

const lineSplit32 = (str) => {
  return str.toUpperCase();
  // return str.match(/.{1,32}/g).join('\n').toUpperCase();
};

describe('transcode-unsigned-tx', function() {
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
    expect(expectedTx).to.deep.equal(actualTx);
  });
  it('decodedTx encodes to encodedTx', function() {
    const expectedTx = encodedTx;
    // console.log('expectedTx', expectedTx);
    const actualTx = TxTranscoder.encodeTx(rawDecodedTx, true);
    // console.log('actualTx', actualTx);

    // split so it's not all one long line of hex.
    expect(lineSplit32(expectedTx)).to.deep.equal(lineSplit32(actualTx));
  });
});
