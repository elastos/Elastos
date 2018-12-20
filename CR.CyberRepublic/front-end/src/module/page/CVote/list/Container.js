import { createContainer, api_request } from '@/util'
import Component from './Component'

export default createContainer(Component, (state) => {

    return {
        currentUserId: state.user.current_user_id,
        isLogin : state.user.is_login,

        // TODO: this should be dynamic
        isCouncil: [

            '5b28be2784f6f900350d30b9',
            '5b367c128f23a70035d35425',
            '5bcf21f030826d68a940b017',
            '5b4c3ba6450ff10035954c80'

        ].indexOf(state.user.current_user_id) >= 0,
        language: state.language.language
    }
}, ()=>{
    return {

        async listData(param, isCouncil){

            let result

            if (isCouncil) {
                result = await api_request({
                    path: '/api/cvote/list',
                    method: 'get',
                    data: param
                });

            } else {
                result = await api_request({
                    path: '/api/cvote/list_public',
                    method: 'get',
                    data: param
                });
            }

            return result;
        }
    }
})
