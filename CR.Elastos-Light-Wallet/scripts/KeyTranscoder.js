'use strict';

const EC = require('elliptic').ec;
const curve = new EC('p256');

const getPublic = (privateKey) => {
  // console.log('getPublic.privateKey', privateKey);
  const rawPrivateKey = Buffer.from(privateKey, 'hex');
  const keypair = curve.keyFromPrivate(rawPrivateKey);
  const publicKey = getPublicEncoded(keypair, true);
  return publicKey.toString('hex');
};

const getPublicKeyEncoded = (publicKey) => {
  if (!Buffer.isBuffer(publicKey)) {
    throw Error('publicKey must be a Buffer');
  }
  // CreateSingleSignatureRedeemScript.pubkeyRaw 33 037D5290F31135FD74E5AAE50EA70E07BC86AC5DBF4C1279D4A29A2BDA30F0C665
  // CreateSingleSignatureRedeemScript.pubkey 037D5290F31135FD74E5AAE50EA70E07BC86AC5DBF4C1279D4A29A2BDA30F0C665
  // CreateSingleSignatureRedeemScript.script 21037D5290F31135FD74E5AAE50EA70E07BC86AC5DBF4C1279D4A29A2BDA30F0C665AC

  if ((publicKey.length == 33)) {
    // console.log('getPublicKeyEncoded.return 1');
    return publicKey;
  }
  let encoded;
  if (publicKey[64] % 2 === 1) {
    // console.log('getPublicKeyEncoded.return 2');
    encoded = '03' + publicKey.slice(1, 33).toString('hex').toUpperCase();
  } else {
    // console.log('getPublicKeyEncoded.return 3');
    encoded = '02' + publicKey.slice(1, 33).toString('hex').toUpperCase();
  }
  return Buffer.from(encoded, 'hex');
};

const getPublicEncoded = (keypair, encode) => {
  //   const unencodedPubKey = keypair.getPublic().encode('hex');
  const unencodedPubKey = Buffer.from(keypair.getPublic().encode());
  if (!Buffer.isBuffer(unencodedPubKey)) {
    throw Error('unencodedPubKey must be a Buffer ' + unencodedPubKey);
  }
  // console.log('getPublicEncoded.unencodedPubKey', unencodedPubKey);
  if (encode) {
    return getPublicKeyEncoded(unencodedPubKey);
  } else {
    return unencodedPubKey;
  }
};

exports.getPublic = getPublic;
exports.getPublicKeyEncoded = getPublicKeyEncoded;
