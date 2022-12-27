const assert = require('chai').assert;
const expect = require('chai').expect;

const app = require('../scripts/App.js');

describe('NumberFormat', function () {
  it('raw 1', function () {
    const input = 1;
    const expectedValue = '0.00000001';
    const actualValue = app.formatTxValue(input);
    expect(expectedValue).to.equal(actualValue);
  });
  it('raw 500', function () {
    const input = 500;
    const expectedValue = '0.000005';
    const actualValue = app.formatTxValue(input);
    expect(expectedValue).to.equal(actualValue);
  });
  it('raw 1 ELA', function () {
    const input = 100000000;
    const expectedValue = '1.0';
    const actualValue = app.formatTxValue(input);
    expect(expectedValue).to.equal(actualValue);
  });
  it('raw 1.00000005 ELA', function () {
    const input = 100000005;
    const expectedValue = '1.00000005';
    const actualValue = app.formatTxValue(input);
    expect(expectedValue).to.equal(actualValue);
  });
});
