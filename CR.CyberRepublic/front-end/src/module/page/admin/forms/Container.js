import { createContainer } from '@/util'
import Component from './Component'
import SubmissionService from '@/service/SubmissionService'
import _ from 'lodash'

import {SUBMISSION_TYPE} from '@/constant'

export default createContainer(Component, (state) => {

  const submissionState = state.submission

  // this fixes the issue with an array returning as a obj
  if (!_.isArray(state.submission.all_submissions)) {
    submissionState.all_submissions = _.values(state.submission.all_submissions)
  }

  return submissionState

}, () => {

  const submissionService = new SubmissionService()

  return {
    async getSubmissions(showArchived) {
      return submissionService.index({
        admin: true,
        type: JSON.stringify({
          $in: [SUBMISSION_TYPE.FORM_EXT, SUBMISSION_TYPE.EMPOWER_35]
        }),
        showArchived
      })
    },

    async resetSubmissions () {
      return submissionService.resetAllSubmissions()
    },

    async setFilter(options) {

    },

    async archiveSubmission(submissionId, showArchived) {
      await submissionService.archive(submissionId)

      return submissionService.index({
        admin: true,
        type: JSON.stringify({
          $in: [SUBMISSION_TYPE.FORM_EXT, SUBMISSION_TYPE.EMPOWER_35]
        }),
        showArchived
      })
    },

    async showArchived(showArchived) {
      await submissionService.index({
        admin: true,
        type: JSON.stringify({
          $in: [SUBMISSION_TYPE.FORM_EXT, SUBMISSION_TYPE.EMPOWER_35]
        }),
        showArchived
      })
    },

    saveFilter(filter) {
      submissionService.saveFilter(filter)
    }
  }
})
