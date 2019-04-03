import {createContainer} from '@/util'
import Component from './Component'
import TeamService from '@/service/TeamService'

export default createContainer(Component, (state) => {
  return {
    current: {
      id: state.user.current_user_id
    }
  }
}, () => {
  const teamService = new TeamService()

  return {

  }
})
