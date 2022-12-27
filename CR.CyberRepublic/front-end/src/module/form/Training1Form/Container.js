import {createContainer, goPath} from '@/util'
import Component from './Component'
import SubmissionService from '@/service/SubmissionService'
import {message} from 'antd'
import _ from 'lodash'
import { logger } from '@/util'
import {SUBMISSION_TYPE} from '@/constant'

message.config({
  top: 100
})


export default createContainer(Component, (state) => {
  return {}
}, () => {
  const submissionService = new SubmissionService()

  return {
    async submitForm(formData, st) {
      try {
        const rs = await submissionService.create({

          title: 'Evangelist Training 1',
          type: SUBMISSION_TYPE.FORM_EXT,
          campaign: 'Evangelist Training 1',

          email: formData.email,
          fullLegalName: formData.fullLegalName,
          occupation: formData.occupation,
          education: formData.education,

          audienceInfo: formData.audienceInfo,
          publicSpeakingExp: formData.publicSpeakingExp,
          previousExp: formData.previousExp,

          isDeveloper: formData.isDeveloper,

          attachment: st.attachment_url,
          attachmentFilename: st.attachment_filename,
          attachmentType: st.attachment_type,

          devBackground: formData.devBackground,
          description: formData.description,
          reason: formData.reason
        })

        if (rs) {
          message.success('Submitted successfully')
          submissionService.path.push('/')
        }
      } catch (err) {
        message.error(err.message)
        logger.error(err)
      }
    }
  }
})
