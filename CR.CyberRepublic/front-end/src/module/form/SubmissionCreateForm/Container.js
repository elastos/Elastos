import {createContainer} from '@/util'
import Component from './Component'
import SubmissionService from '@/service/SubmissionService'

export default createContainer(Component, (state) => {
  return {

  }
}, () => {
  const ss = new SubmissionService()
  return {
    async create(param) {
      return ss.create(param)
    }
  }
})
