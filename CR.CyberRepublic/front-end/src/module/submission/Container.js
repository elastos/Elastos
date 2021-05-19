import {createContainer} from '@/util'
import Component from './Component'
import SubmissionService from '@/service/SubmissionService'

export default createContainer(Component, (state) => {

  let page = 'PUBLIC' // default

  if (/^\/admin/.test(state.router.location.pathname)) {
    page = 'ADMIN'
  } else if (/^\/profile/.test(state.router.location.pathname)) {
    page = 'LEADER'
  }

  return {
    is_admin: state.user.is_admin,
    is_login: state.user.is_login,

    page
  }
}, () => {

  const submissionService = new SubmissionService()

  return {
    async getSubmissionDetail(submissionId) {
      return submissionService.get(submissionId)
    },

    async resetSubmissionDetail() {
      return submissionService.resetSubmissionDetail()
    },
  }
})
