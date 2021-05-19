'use strict';

const bitcoreTools = require('../libraries/bitcore-tools.js');

// coin used by elephant wallet
// const bip44path = `m/44'/0'/0'/0/0`;
const BITCOIN_COIN = 0;

// coin used by ledger nano s.
// const bip44path = `m/44'/2305'/0'/0/0`;
const ELASTOS_COIN = 2305;

const change = 0;

const index = 0;

function getPrivateKeyFromMnemonic(mnemonic) {
  const seedBytes = bitcoreTools.getSeedFromMnemonic(mnemonic);
  const seed = Buffer.from(seedBytes).toString('hex');
  bitcoreTools.bitcore.crypto.Point.setCurve('p256');
  const privateKey = bitcoreTools.generateSubPrivateKey(seed, BITCOIN_COIN, change, index).toString('hex');
  return privateKey;
}

exports.getPrivateKeyFromMnemonic = getPrivateKeyFromMnemonic;
