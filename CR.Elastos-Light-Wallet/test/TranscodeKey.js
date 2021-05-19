const assert = require('chai').assert;
const KeyTranscoder = require('../scripts/KeyTranscoder.js');
const AddressTranscoder = require('../scripts/AddressTranscoder.js');

const wif = require('wif');
const fixtures = require('./TranscodeKeyTestData.json');

const hexPrivateKey = fixtures.privateKey;
const expectedPublicKey = fixtures.publicKey;
const expectedAddress = fixtures.address;
const hexScriptHash = fixtures.scriptHash;

describe('transcode-key', function() {
  it('private ECDSA derives expected public', function() {
    const actualPublicKey = KeyTranscoder.getPublic(hexPrivateKey);
    assert.equal(actualPublicKey, expectedPublicKey, 'PublicKey must match expected');
  });
  it('public ECDSA derives expected address', function() {
    const actualAddress = AddressTranscoder.getAddressFromPublicKey(expectedPublicKey);
    assert.equal(actualAddress, expectedAddress, 'Address must match expected');
  });
  it('scripthash derives expected address', function() {
    const rawScriptHash = Buffer.from(hexScriptHash, 'hex');
    const actualAddress = AddressTranscoder.getAddressFromProgramHash(rawScriptHash);
    assert.equal(actualAddress, expectedAddress, 'Address must match expected');
  });
  it('ledger WIF derives expected private', function() {
    const ledgerWif = fixtures.ledgerWif;
    const ledgerPrivateKey = fixtures.ledgerPrivateKey;
    const privateKey = wif.decode(ledgerWif).privateKey.toString('hex');
    assert.equal(privateKey, ledgerPrivateKey, 'PrivateKey must match expected');
  });
  it('ledger private derives expected address', function() {
    const privateKey = fixtures.ledgerPrivateKey;
    const publicKey = KeyTranscoder.getPublic(privateKey);
    const actualAddress = AddressTranscoder.getAddressFromPublicKey(publicKey);
    const ledgerAddress = fixtures.ledgerAddress;
    assert.equal(actualAddress, ledgerAddress, 'PublicKey must match expected');
  });
  it('private WIF derives expected address', function() {
    const ledgerWif = fixtures.ledgerWif;
    const ledgerAddress = fixtures.ledgerAddress;
    const privateKey = wif.decode(ledgerWif).privateKey.toString('hex');
    const publicKey = KeyTranscoder.getPublic(privateKey);
    const actualAddress = AddressTranscoder.getAddressFromPublicKey(publicKey);
    assert.equal(actualAddress, ledgerAddress, 'PublicKey must match expected');
  });
  it('extendedPublicKey1 derives expected address1', function() {
    const publicKey = fixtures.extendedPublicKey1;
    const actualAddress = AddressTranscoder.getAddressFromPublicKey(publicKey);
    const expectedAddress = fixtures.extendedPublicKeyAddress1;
    assert.equal(actualAddress, expectedAddress, 'Address must match expected');
  });
  it('extendedPublicKey1 derives expected publicKey', function() {
    const extendedPublicKey = Buffer.from(fixtures.extendedPublicKey1, 'hex');
    const actualPublicKey = KeyTranscoder.getPublicKeyEncoded(extendedPublicKey).toString('hex').toUpperCase();
    const expectedPublicKey = fixtures.publicKey1;
    assert.equal(actualPublicKey, expectedPublicKey, 'PublicKey must match expected');
  });
  it('extendedPublicKey2 derives expected address2', function() {
    const publicKey = fixtures.extendedPublicKey2;
    const actualAddress = AddressTranscoder.getAddressFromPublicKey(publicKey);
    const expectedAddress = fixtures.extendedPublicKeyAddress2;
    assert.equal(actualAddress, expectedAddress, 'Address must match expected');
  });
});
