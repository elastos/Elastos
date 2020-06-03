const assert = require('chai').assert;
const expect = require('chai').expect;

const app = require('../scripts/App.js');

describe.only('NumberFormat', function() {
  it('raw 1', function() {
    const input = 1;
    const expectedValue = '0.0000001';
    const actualValue = app.formatTxValue(input);
    expect(expectedValue).to.equal(actualValue);
  });
});
