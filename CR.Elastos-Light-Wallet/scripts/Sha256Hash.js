const crypto = require('crypto');

const Sha256Hash = (buffer) => {
  return crypto.createHash('sha256').update(buffer).digest();
}

exports.Sha256Hash = Sha256Hash;
