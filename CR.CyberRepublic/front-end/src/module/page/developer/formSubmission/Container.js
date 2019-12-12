import { createContainer } from '@/util'
import Component from './Component'
import SubmissionService from '@/service/SubmissionService'
import { message } from 'antd'
import I18N from '@/I18N'
import { logger } from '@/util'

export default createContainer(Component, (state) => {
  return {
    ...state
  }
}, () => {
  const submissionService = new SubmissionService()

  return {
    async createSubmission (type, title, description) {
      try {
        const rs = await submissionService.create({
          type,
          title,
          description
        })

        if (rs) {
          message.success(I18N.get('developer.form.submission.message.success'))
        }
      } catch (err) {
        message.error(err.message)
        logger.error(err)
      }
    }
  }
})
