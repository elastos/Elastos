'use strict';

require('babel-polyfill');

const TransportNodeHid = require('@ledgerhq/hw-transport-node-hid');
// const TransportNodeHid = {};
// TransportNodeHid.default = {};
// TransportNodeHid.default.isSupported = ( () => { return false; } );
// TransportNodeHid.default.list = [];

const mainConsoleUtil = require('console');
const mainConsole = new mainConsoleUtil.Console(process.stdout, process.stderr);

// max length in bytes.
const MAX_SIGNED_TX_LEN = 1024;

const bip44Path =
  '8000002C' +
  '80000901' +
  '80000000' +
  '00000000' +
  '00000000';

const LOG_LEDGER_MESSAGE = false;
if (LOG_LEDGER_MESSAGE) {
  mainConsole.log('Ledger Console Logging Enabled.');
} else {
  mainConsole.log('Ledger Console Logging Disabled.');
}

const getPublicKey = (callback) => {
  const deviceThenCallback = (device) => {
    try {
      // mainConsole.log('sending message ');
      const message = Buffer.from('8004000000' + bip44Path, 'hex');
      // mainConsole.log(`STARTED sending message ${message.toString('hex').toUpperCase()}`);
      // mainConsole.log(`STARTED sending device ${JSON.stringify(device)}`);
      device.exchange(message).then((response) => {
        // mainConsole.log('SUCCESS sending message');
        device.close();
        const responseStr = response.toString('hex').toUpperCase();
        let success = false;
        let message = '';
        let publicKey = '';
        // mainConsole.log(`INTERIM sending message: responseStr:${responseStr}`);
        if (responseStr.endsWith('9000')) {
          success = true;
          message = responseStr;
          publicKey = responseStr.substring(0, 130);
        } else {
          if (responseStr == '6E00') {
            message = 'App Not Open On Ledger Device';
          } else {
            message = 'Unknown Error';
          }
        }
        callback({
          success: success,
          message: message,
          publicKey: publicKey,
        });
      }).catch((error) => {
        // mainConsole.trace(error);
        // mainConsole.log(`FAILURE sending message. error:${error}`);
        callback({
          success: false,
          message: 'Error ' + JSON.stringify(error),
        });
      });
    } catch (error) {
      // mainConsole.log(`FAILURE creating and sending message. error:${error}`);
      callback({
        success: false,
        message: 'Error ' + JSON.stringify(error),
      });
    }
  };
  const deviceErrorCallback = (error) => {
    callback({
      success: false,
      message: 'Error ' + JSON.stringify(error),
    });
  };
  getLedgerInfo(deviceThenCallback, deviceErrorCallback);
};

const finishLedgerDeviceInfo = (msg) => {
  if (LOG_LEDGER_MESSAGE) {
    mainConsole.log('finishLedgerDeviceInfo : ', JSON.stringify(msg));
  }
  return msg;
};


const getLedgerDeviceInfo = (callback) => {
  const deviceThenCallback = (device) => {
    try {
      const deviceInfo = device.device.getDeviceInfo();
      const deviceInfoStr = JSON.stringify(deviceInfo);
      callback(finishLedgerDeviceInfo({
        enabled: true,
        error: false,
        message: `Ledger Device Found.${deviceInfoStr}`,
      }));
    } catch (error) {
      callback(finishLedgerDeviceInfo({
        enabled: false,
        error: true,
        message: error,
      }));
    } finally {
      device.close();
    }
  };
  const deviceErrorCallback = (error) => {
    callback(finishLedgerDeviceInfo(error));
  };
  getLedgerInfo(deviceThenCallback, deviceErrorCallback);
};

const getLedgerInfo = (deviceThenCallback, deviceErrorCallback) => {
  const supported = TransportNodeHid.default.isSupported();
  if (!supported) {
    deviceErrorCallback(finishLedgerDeviceInfo({
      enabled: false,
      error: true,
      message: 'Your computer does not support the ledger device.',
    }));
    return;
  }

  TransportNodeHid.default.list().then((paths) => {
    if (paths.length === 0) {
      deviceErrorCallback(finishLedgerDeviceInfo({
        enabled: false,
        error: false,
        message: 'USB Error: No device found.',
      }));
    } else {
      TransportNodeHid.default.open(paths[0]).then((device) => {
        deviceThenCallback(device);
      }, (error) => {
        deviceErrorCallback(error);
      });
    }
  }, (error) => {
    deviceErrorCallback(error);
  });
};

const splitMessageIntoChunks = (ledgerMessage) => {
  const messages = [];
  const bufferSize = 255 * 2;
  let offset = 0;
  while (offset < ledgerMessage.length) {
    let chunk;
    let p1;
    if ((ledgerMessage.length - offset) > bufferSize) {
      chunk = ledgerMessage.substring(offset, offset + bufferSize);
    } else {
      chunk = ledgerMessage.substring(offset);
    }
    if ((offset + chunk.length) == ledgerMessage.length) {
      p1 = '80';
    } else {
      p1 = '00';
    }

    const chunkLength = chunk.length / 2;

    // mainConsole.log(`Ledger Signature chunkLength ${chunkLength}`);

    let chunkLengthHex = chunkLength.toString(16);
    while (chunkLengthHex.length < 2) {
      chunkLengthHex = '0' + chunkLengthHex;
    }

    // mainConsole.log(`Ledger Signature chunkLength hex ${chunkLengthHex}`);

    messages.push('8002' + p1 + '00' + chunkLengthHex + chunk);
    offset += chunk.length;
  }

  return messages;
};

const decodeSignature = (response) => {
  /**
   * https://stackoverflow.com/questions/25829939/specification-defining-ecdsa-signature-data
   * <br>
   * the signature is TLV encoded.
   * the first byte is 30, the "signature" type<br>
   * the second byte is the length (always 44)<br>
   * the third byte is 02, the "number: type<br>
   * the fourth byte is the length of R (always 20)<br>
   * the byte after the encoded number is 02, the "number: type<br>
   * the byte after is the length of S (always 20)<br>
   * <p>
   * eg:
   * 304402200262675396fbcc768bf505c9dc05728fd98fd977810c547d1a10c7dd58d18802022069c9c4a38ee95b4f394e31a3dd6a63054f8265ff9fd2baf68a9c4c3aa8c5d47e9000
   * is
   * 30LL0220RRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR0220SSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSS
   */

  const rLenHex = response.substring(6, 8);
  // mainConsole.log(`Ledger Signature rLenHex ${rLenHex}`);
  const rLen = parseInt(rLenHex, 16) * 2;
  // mainConsole.log(`Ledger Signature rLen ${rLen}`);
  let rStart = 8;
  // mainConsole.log(`Ledger Signature rStart ${rStart}`);
  const rEnd = rStart + rLen;
  // mainConsole.log(`Ledger Signature rEnd ${rEnd}`);

  while ((response.substring(rStart, rStart + 2) == '00') && ((rEnd - rStart) > 64)) {
    rStart += 2;
  }

  const r = response.substring(rStart, rEnd);
  // mainConsole.log(`Ledger Signature R [${rStart},${rEnd}:${(rEnd - rStart)} ${r}`);
  const sLenHex = response.substring(rEnd + 2, rEnd + 4);
  // mainConsole.log(`Ledger Signature sLenHex ${sLenHex}`);
  const sLen = parseInt(sLenHex, 16) * 2;
  // mainConsole.log(`Ledger Signature sLen ${sLen}`);
  let sStart = rEnd + 4;
  // mainConsole.log(`Ledger Signature sStart ${sStart}`);
  const sEnd = sStart + sLen;
  // mainConsole.log(`Ledger Signature sEnd ${sEnd}`);

  while ((response.substring(sStart, sStart + 2) == '00') && ((sEnd - sStart) > 64)) {
    sStart += 2;
  }

  const s = response.substring(sStart, sEnd);
  // mainConsole.log(`Ledger Signature S [${sStart},${sEnd}:${(sEnd - sStart)} ${s}`);

  const msgHashStart = sEnd + 4;
  const msgHashEnd = msgHashStart + 64;
  const msgHash = response.substring(msgHashStart, msgHashEnd);
  // mainConsole.log(`Ledger Signature msgHash [${msgHashStart},${msgHashEnd}:${msgHash}`);

  const SignerLength = 32;
  const SignatureLength = 64;

  let signatureHex = r;
  while (signatureHex.length < SignerLength) {
    signatureHex = '0' + signatureHex;
  }

  signatureHex += s;

  while (signatureHex.length < SignatureLength) {
    signatureHex = '0' + signatureHex;
  }

  return signatureHex;
};

const sign = (transactionHex, callback) => {
  const transactionByteLength = Math.ceil(transactionHex.length/2);
  if (transactionByteLength > MAX_SIGNED_TX_LEN) {
    callback({
      success: false,
      message: `Transaction length of ${transactionByteLength} bytes exceeds max length of ${MAX_SIGNED_TX_LEN} bytes. Send less candidates and consolidate utxos.`,
    });
    return;
  } else {
    mainConsole.log(`transaction length of ${transactionByteLength} bytes is under ${MAX_SIGNED_TX_LEN} bytes. Sending.`);
  }

  const ledgerMessage = transactionHex + bip44Path;

  // mainConsole.log(`sign ${ledgerMessage}`);

  const messages = splitMessageIntoChunks(ledgerMessage);

  const deviceThenCallback = async (device) => {
    try {
      let lastResponse = undefined;
      for (let ix = 0; ix < messages.length; ix++) {
        const message = Buffer.from(messages[ix], 'hex');
        // mainConsole.log(`STARTED sending message ${ix+1} of ${messages.length}: ${message.toString('hex').toUpperCase()}`);
        const response = await device.exchange(message);
        const responseStr = response.toString('hex').toUpperCase();
        // mainConsole.log(`SUCCESS sending message ${ix+1} of ${messages.length}: ${responseStr.toString('hex').toUpperCase()}`);

        lastResponse = responseStr;
      }
      device.close();

      let signature = undefined;
      let success = false;
      let message = lastResponse;
      mainConsole.log(`sign lastResponse`, lastResponse);
      if (lastResponse !== undefined) {
        if (lastResponse.endsWith('9000')) {
          signature = decodeSignature(lastResponse);
          success = true;
        } else {
          if (lastResponse == '6985') {
            message += ' Tx Denied on Ledger';
          }
          if (lastResponse == '6D08') {
            message += ' Tx Too Large for Ledger';
          }
        }
      }

      callback({
        success: success,
        message: message,
        signature: signature,
      });
    } catch (error) {
      // mainConsole.log(`FAILURE creating and sending message. error:${error}`);
      // mainConsole.log(error.stack);
      callback({
        success: false,
        message: 'Error ' + JSON.stringify(error),
      });
    }
  };
  const deviceErrorCallback = (error) => {
    callback({
      success: false,
      message: 'Error ' + JSON.stringify(error),
    });
  };
  getLedgerInfo(deviceThenCallback, deviceErrorCallback);
};

exports.getLedgerDeviceInfo = getLedgerDeviceInfo;
exports.getPublicKey = getPublicKey;
exports.sign = sign;
