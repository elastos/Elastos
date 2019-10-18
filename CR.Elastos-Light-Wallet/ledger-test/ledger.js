require('babel-polyfill');

const assert = require('chai').assert;
const expect = require('chai').expect;

const txFixtures = require('../test/TranscodeSignedTxTestData.json')

const LedgerComm = require('../scripts/LedgerComm.js')

const TxSigner = require('../scripts/TxSigner.js')

const AddressTranscoder = require('../scripts/AddressTranscoder.js')

describe('ledger communication', function() {
  it('ledger getLedgerDeviceInfo works', function(done) {
    const callback = (msg) => {
      console.log(`LedgerComm.getLedgerDeviceInfo ${JSON.stringify(msg)}`);
      done();
    }
    LedgerComm.getLedgerDeviceInfo(callback);
  });
    it('ledger getPublicKey works', function(done) {
      const callback = (msg) => {
        console.log(`LedgerComm.getPublicKey ${JSON.stringify(msg)}`);
        if(msg.success) {
          const publicKey = msg.publicKey;
          const address = AddressTranscoder.getAddressFromPublicKey(publicKey);
          console.log(`address ${address}`);
        }
        done();
      }
      LedgerComm.getPublicKey(callback);
    });
    it('ledger sign works', function(done) {
      const rawDecodedTxForSigning = JSON.parse(JSON.stringify(txFixtures.decodedTxForSigning));
      const callback = (msg) => {
        console.log(`LedgerComm.sign ${JSON.stringify(msg)}`);
        if(msg.success) {
          const signature = msg.signature;
          console.log(`signature ${signature}`);
          const actualTx = TxSigner.addSignatureToTx(rawDecodedTxForSigning, publicKey, signature);
          console.log(`actualTx ${actualTx}`);
        }
        done();
      }
      LedgerComm.sign(rawDecodedTxForSigning,callback);
    });
});
