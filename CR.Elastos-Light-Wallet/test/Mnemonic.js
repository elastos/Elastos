const assert = require('chai').assert;
const expect = require('chai').expect;

const bitcoreTools = require('../libraries/bitcore-tools.js');

const mnemonic = 'abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about';

const passphrase = '';

const privateKey = '9007fb0e9149e8940559a6c69f53c276d94cf94956d8e7b10ef6c2b2e5237d1a';

// const bip44path = `m/44'/2305'/0'/0/0`;

const index = 0;

function calcAddressForELA(seed, coin, account, change, index) {
  const publicKey = bitcoreTools.generateSubPublicKey(bitcoreTools.getMasterPublicKey(seed), change, index);
  return {
    privateKey: bitcoreTools.generateSubPrivateKey(seed, coin, change, index).toString('hex'),
    publicKey: publicKey,
    // address: getAddress(publicKey.toString('hex')),
    // did: getDid(publicKey.toString('hex')),
  };
}

describe('Mnemonic.sign', function() {
  it.only('Mnemonic.tx mnemonic to seed', function() {
    const actualSeedRaw = bitcoreTools.getSeedFromMnemonic(mnemonic);
    const actualSeed = Buffer.from(actualSeedRaw).toString('hex');
    bitcoreTools.bitcore.crypto.Point.setCurve('p256');
    const elaAddress = calcAddressForELA(actualSeed, 0, 0, 0, index);
    const expectedPrivateKey = elaAddress.privateKey;
    const actualPrivateKey = privateKey;
    // address = elaAddress.address;
    // privkey = elaAddress.privateKey;
    // pubkey = elaAddress.publicKey;
    expect(expectedPrivateKey).to.equal(actualPrivateKey);
  });
});
