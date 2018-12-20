import BaseRedux from '@/model/BaseRedux';

class UserRedux extends BaseRedux {
    defineTypes() {
        return ['user'];
    }

    defineDefaultState() {
        return {
            loading: false,

            is_login: false,
            is_leader: false,
            is_admin: false,

            email: '',
            username: '',

            role: '',
            circles: [],
            subscribers: [],
            // TODO: I think we scrap this
            login_form: {
                username: '',
                password: '',
                loading: false
            },

            // TODO: I think we scrap this
            register_form: {
                firstName: '',
                lastName: '',
                email: '',
                password: ''
            },

            profile: {

            },
            current_user_id: null,

            teams: []
        };
    }
}

export default new UserRedux()
