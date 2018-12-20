import userRedux from '../user';

test('user redux', ()=>{

    expect(userRedux.types.is_login_update).toBe('user/is_login_update');
});