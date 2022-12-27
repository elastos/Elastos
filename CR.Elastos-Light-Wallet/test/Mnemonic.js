const assert = require('chai').assert;
const expect = require('chai').expect;

const mnemonic = require('../scripts/Mnemonic.js');

const mnemonicInput = 'abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about';

const privateKey = '9007fb0e9149e8940559a6c69f53c276d94cf94956d8e7b10ef6c2b2e5237d1a';

const address = 'EdB7W1rRh5KgUha9Wa676ZRmr18voCDS6k';

describe('Mnemonic.sign', function() {
  it('Mnemonic mnemonic to seed', function() {
    const expectedPrivateKey = mnemonic.getPrivateKeyFromMnemonic(mnemonicInput);
    const actualPrivateKey = privateKey;
    expect(expectedPrivateKey).to.equal(actualPrivateKey);
  });
});
