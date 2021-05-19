import {createContainer} from '@/util'
import UserService from '@/service/UserService'
import Component from './Component'
import SubmissionService from '@/service/SubmissionService'

import {SUBMISSION_TYPE} from '@/constant'

export default createContainer(Component, (state) => {
  return {
    ...state.task,
    user: state.user,
    is_login: state.user.is_login
  }
}, () => {
  const userService = new UserService()
  const submissionService = new SubmissionService()

  return {
    async getEmpowerUsers() {
      return userService.getAll({
        empower: JSON.stringify({$exists: true})
      })
    },

    async empowerApply(formData, state) {

      await submissionService.create({

        title: `${state.applyEmpowerType} Empower35 Ambassador Application`,
        type: SUBMISSION_TYPE.EMPOWER_35,
        campaign: state.applyEmpowerType,

        reason: formData.applyReason,
        suitedReason: formData.suitedReason,

        attachment: formData.filePath,
        attachmentFilename: formData.fileName,
        attachmentType: formData.fileType,

      })
    }
  }
})
