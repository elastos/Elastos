import {createContainer} from '@/util'
import Component from './Component'
import SubmissionService from '@/service/SubmissionService'
import { message } from 'antd/lib/index'

export default createContainer(Component, (state) => {

  return {
    submission: state.submission.detail,
    loading: state.submission.loading
  }
}, () => {
  const submissionService = new SubmissionService()

  return {
    async getSubmissionDetail (submissionId) {
      return submissionService.get(submissionId)
    },

    async resetSubmissionDetail () {
      return submissionService.resetSubmissionDetail()
    }
  }
})
