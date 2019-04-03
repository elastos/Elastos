import {createContainer} from '@/util'
import Component from './Component'

export default createContainer(Component, (state) => {
  return {
    current: {
      id: state.user.current_user_id
    }
  }
}, () => {
  return {

  }
})
