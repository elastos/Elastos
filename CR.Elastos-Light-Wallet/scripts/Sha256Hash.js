'use strict';

const crypto = require('crypto');

const sha256Hash = (buffer) => {
  return crypto.createHash('sha256').update(buffer).digest();
};

exports.sha256Hash = sha256Hash;
