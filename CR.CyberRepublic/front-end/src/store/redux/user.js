import BaseRedux from '@/model/BaseRedux'

class UserRedux extends BaseRedux {
  defineTypes() {
    return ['user']
  }

  defineDefaultState() {
    return {
      loading: false,
      avatar_loading: false,

      is_login: false,
      is_leader: false,
      is_admin: false,
      is_council: false,
      is_secretary: false,

      email: '',
      username: '',
      did: {},

      role: '',
      circles: [],
      subscribers: [],
      // TODO: I think we scrap this
      login_form: {
        username: '',
        password: '',
        loading: false,
      },

      // TODO: I think we scrap this
      register_form: {
        firstName: '',
        lastName: '',
        email: '',
        password: '',
      },

      profile: {

      },
      current_user_id: null,

      teams: [],

      popup_update: false,
    }
  }
}

export default new UserRedux()
