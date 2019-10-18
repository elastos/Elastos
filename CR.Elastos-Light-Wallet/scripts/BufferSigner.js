const EC = require('elliptic').ec;
const curve = new EC('p256');

const Sha256Hash = require('./Sha256Hash.js')


const SignerLength = 32;
const SignatureLength = 64;

const getHash = (bufferHex) => {
  const buffer = Buffer.from(bufferHex, 'hex');

  const hashSha = Sha256Hash.Sha256Hash(buffer);
  return hashSha;
}

const sign = (bufferHex, privateKeyHex) => {
  const privateKey = Buffer.from(privateKeyHex, 'hex');
  const hash = getHash(bufferHex);

  // console.log('sign.hash',hash.toString('hex').toUpperCase());

  const signature = curve.sign(hash, privateKey, null);

  const r = signature.r.toArrayLike(Buffer, 'be', 32).toString('hex').toUpperCase();

  const s = signature.s.toArrayLike(Buffer, 'be', 32).toString('hex').toUpperCase();

  // console.log('sign.r',r);
  // console.log('sign.s',s);

  var signatureHex = r;
  while (signatureHex.length < SignerLength) {
    signatureHex = '0' + signatureHex;
  }

  signatureHex += s;

  while (signatureHex.length < SignatureLength) {
    signatureHex = '0' + signatureHex;
  }

  return signatureHex;
};


exports.sign = sign;
exports.getHash = getHash;
