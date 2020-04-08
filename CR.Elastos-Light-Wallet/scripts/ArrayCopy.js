'use strict';

const arraycopy = (from, fromIx, to, toIx, length) => {
  // console.log('arraycopy.from', from);
  // console.log('arraycopy.fromIx', fromIx);
  // console.log('arraycopy.to', to);
  // console.log('arraycopy.toIx', toIx);
  // console.log('arraycopy.length', length);
  for (let i = 0; i < length; i++) {
    to[i + toIx] = from[i + fromIx];
  }
};

exports.arraycopy = arraycopy;
