import {createContainer, goPath} from '@/util'
import Component from './Component'
import SubmissionService from '@/service/SubmissionService'
import CommunityService from '@/service/CommunityService'
import {message} from 'antd'
import { logger } from '@/util'

import {SUBMISSION_TYPE, SUBMISSION_CAMPAIGN} from '@/constant'

message.config({
  top: 100
})


export default createContainer(Component, (state) => {
  return {
    user_email: state.user.email,
    is_login: state.user.is_login
  }
}, () => {
  const submissionService = new SubmissionService()
  const communityService = new CommunityService()

  return {
    async submitForm(formData, st) {

      try {
        const rs = await submissionService.create({

          title: `${st.community.name} Organizer App`,
          type: SUBMISSION_TYPE.FORM_EXT,
          campaign: SUBMISSION_CAMPAIGN.COMMUNITY_ORGANIZER,

          email: this.user_email,
          fullLegalName: formData.fullLegalName,
          occupation: formData.occupation,
          education: formData.education,

          audienceInfo: formData.audienceInfo,
          publicSpeakingExp: formData.publicSpeakingExp,
          eventOrganizingExp: formData.eventOrganizingExp,
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
          message.success('Success - one of the admins will be in touch shortly')
          submissionService.path.push('/profile/submissions')
        }
      } catch (err) {
        message.error(err.message)
        logger.error(err)
      }
    },

    async getCommunityDetail(communityId) {
      return communityService.get(communityId)
    }
  }
})
