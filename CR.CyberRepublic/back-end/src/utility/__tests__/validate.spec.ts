declare var test, expect, require, process;

import validate from '../validate';

test('email method', ()=>{
    expect(validate.email('aaa.bb@cc.com')).toBe(true);
});