import {createContainer, goPath} from '@/util'
import Component from './Component'
import TeamService from '@/service/TeamService'
import {message} from 'antd'
import { logger } from '@/util'

export default createContainer(Component, (state) => {
  return {

  }
}, () => {
  const ts = new TeamService()
  return {
    async update(param) {
      try {
        return await ts.update(param)
      } catch (e) {
        message.error(e.message)
        logger.error(e)
      }
    }
  }
})
