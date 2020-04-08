'use strict';

const bs58 = require('bs58');
const SmartBuffer = require('smart-buffer').SmartBuffer;
const ripemd160 = require('ripemd160');

const Sha256Hash = require('./Sha256Hash.js');
const ArrayCopy = require('./ArrayCopy.js');
const KeyTranscoder = require('./KeyTranscoder.js');

const sha256HashTwice = (buffer) => {
  return Sha256Hash.sha256Hash(Sha256Hash.sha256Hash(buffer));
};

const getSingleSignatureRedeemScript = (pubkey, signType) => {
  // console.log('getSingleSignatureRedeemScript.pubkey', pubkey);
  const script = createSingleSignatureRedeemScript(pubkey, signType);
  const scriptBuffer = Buffer.from(script);
  // console.log('getSingleSignatureRedeemScript.scriptBuffer', scriptBuffer);
  const scriptBufferHex = scriptBuffer.toString('hex').toUpperCase();
  // console.log('getSingleSignatureRedeemScript.scriptBufferHex', scriptBufferHex);
  return scriptBufferHex;
};


const createSingleSignatureRedeemScript = (pubkeyRaw) => {
  // console.log('createSingleSignatureRedeemScript.pubkeyRaw', pubkeyRaw.length, pubkeyRaw.toString('hex').toUpperCase());
  const pubkey = KeyTranscoder.getPublicKeyEncoded(pubkeyRaw);
  // console.log('createSingleSignatureRedeemScript.pubkey   ', pubkey.length, pubkey.toString('hex').toUpperCase());
  const script = new Uint8Array(35);
  script[0] = 33;
  ArrayCopy.arraycopy(pubkey, 0, script, 1, 33);
  script[34] = 0xAC;

  // console.log('createSingleSignatureRedeemScript.script  ', Buffer.from(script).toString('hex').toUpperCase());
  return script;
};

const sha256hash160 = (input) => {
  // console.log('sha256hash160.input', input);
  const sha256 = Sha256Hash.sha256Hash(input);
  // console.log('sha256hash160.sha256', sha256);

  /* eslint-disable */
  const digest = new ripemd160();
  /* eslint-enable */
  digest.update(sha256, 0, sha256.length);
  digest.end();
  const out = digest.read();
  // console.log('sha256hash160.out', out);
  return out;
};

const toCodeHash = (code) => {
  // console.log('sha256hash160.code', code);
  const f = sha256hash160(code);
  const g = new Uint8Array(f.length + 1);
  // console.log('toCodeHash.signType', signType);
  g[0] = 33;
  ArrayCopy.arraycopy(f, 0, g, 1, f.length);
  // console.log('toCodeHash.f', f);
  // ArrayCopy.arraycopy(f, 0, g, 1, f.length);
  // console.log('toCodeHash.g', Buffer.from(g).toString('hex').toUpperCase());
  return Buffer.from(g);
};

const getProgram = (publicKey) => {
  return createSingleSignatureRedeemScript(publicKey);
};
const getSingleSignProgramHash = (publicKey) => {
  return toCodeHash(getProgram(publicKey));
};

const getAddressFromPublicKey = (publicKey) => {
  return getAddressFromProgramHash(getSingleSignProgramHash(Buffer.from(publicKey, 'hex')));
};

const getProgramHashFromAddress = (address) => {
  // console.log('getProgramHashFromAddress.address', address);
  const programHashAndChecksum = bs58.decode(address);
  // console.log('getProgramHashFromAddress.programHashAndChecksum', programHashAndChecksum);
  const programHash = programHashAndChecksum.slice(0, 21);
  // console.log('getProgramHashFromAddress.programHash', programHash);
  return programHash;
};

const getAddressFromProgramHash = (programHash) => {
  // console.log('getAddressFromProgramHash.programHash', programHash.toString('hex').toUpperCase());
  const f = SmartBuffer.fromBuffer(sha256HashTwice(programHash));
  // console.log('getAddressFromProgramHash.f', f.length, f.toBuffer().toString('hex').toUpperCase());
  const g = new SmartBuffer();
  // console.log('getAddressFromProgramHash.g[0]', g.length, g.toBuffer().toString('hex').toUpperCase());
  g.writeBuffer(programHash);
  // console.log('getAddressFromProgramHash.g[1]', g.length, g.toBuffer().toString('hex').toUpperCase());
  g.writeBuffer(f.readBuffer(4));
  // console.log('getAddressFromProgramHash.g[2]', g.length, g.toBuffer().toString('hex').toUpperCase());
  const gBuffer = g.toBuffer();
  // console.log('getAddressFromProgramHash.addressHex', gBuffer.length, gBuffer.toString('hex').toUpperCase());
  const address = bs58.encode(gBuffer);
  // console.log( 'getAddressFromProgramHash.address', address );
  return address;
};

exports.getAddressFromPublicKey = getAddressFromPublicKey;
exports.getAddressFromProgramHash = getAddressFromProgramHash;
exports.getProgramHashFromAddress = getProgramHashFromAddress;
exports.getSingleSignatureRedeemScript = getSingleSignatureRedeemScript;
