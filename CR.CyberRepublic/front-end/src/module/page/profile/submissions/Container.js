import {createContainer} from '@/util'
import Component from './Component'
import SubmissionService from '@/service/SubmissionService'
import _ from 'lodash'
import {USER_ROLE} from '@/constant'


export default createContainer(Component, (state) => {


  const currentUserId = state.user.current_user_id
  const submissionState = {
    ...state.submission,
    currentUserId,
    is_leader: state.user.role === USER_ROLE.LEADER,
    is_admin: state.user.is_admin
  }

  if (!_.isArray(submissionState.all_submissions)) {
    submissionState.all_submissions = _.values(submissionState.all_submissions)
  }

  submissionState.subscribed_submissions = []
  submissionState.owned_submissions = []
  if (submissionState.all_submissions.length) {
    for (const submission of submissionState.all_submissions) {
      if (_.find(submission.subscribers, (subscriber) => {
        return subscriber.user && subscriber.user._id === state.user.current_user_id
      })) {
        submissionState.subscribed_submissions.push(submission)
      }

      if (_.get(submission, 'createdBy._id') === state.user.current_user_id) {
        submissionState.owned_submissions.push(submission)
      }
    }
  }

  return submissionState
}, () => {
  const submissionService = new SubmissionService()

  return {

    /**
         * @returns {Promise<*>}
         */
    async getSubmissions(query) {
      return submissionService.index(query)
    },

    async resetSubmissions () {
      return submissionService.resetAllSubmissions()
    },

    async setFilter(options) {

    }
  }
})
