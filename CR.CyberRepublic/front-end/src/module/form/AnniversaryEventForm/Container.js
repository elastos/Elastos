import {createContainer, goPath} from "@/util";
import Component from './Component';
import SubmissionService from '@/service/SubmissionService'
import UserService from '@/service/UserService'
import {message} from 'antd'
import _ from 'lodash'

import {SUBMISSION_TYPE, SUBMISSION_CAMPAIGN} from '@/constant'

message.config({
    top: 100
})


export default createContainer(Component, (state)=>{
    return {
        user: state.user,
        is_login: state.user.is_login
    };
}, () => {
    const submissionService = new SubmissionService();
    const userService = new UserService()

    return {

        // we only allow one submission for the campaign ANNI_2008 (2018)
        async getExistingSubmission() {
            return submissionService.getExistingSubmission(SUBMISSION_CAMPAIGN.ANNI_2008)
        },

        /**
         * This should dynamically submit/update based on whether the state
         * has an _id
         *
         * @param formData
         * @param st
         * @returns {Promise<void>}
         */
        async submitForm(formData, st){

            let rs

            try {
                if (st._id) {
                    rs = await submissionService.update(st._id, {

                        fullLegalName: formData.fullLegalName,

                        attachment: st.attachment_url,
                        attachmentFilename: st.attachment_filename,
                        attachmentType: st.attachment_type,

                        removeAttachment: st.removeAttachment,

                        passportUpload: st.passport_url,
                        passportFilename: st.passport_filename,
                        passportUploadType: st.passport_type,

                        removePassport: st.removePassport
                    })

                } else {
                    rs = await submissionService.create({

                        title: 'Anniversary 2018 - Application',
                        type: SUBMISSION_TYPE.FORM_EXT,
                        campaign: SUBMISSION_CAMPAIGN.ANNI_2008,

                        fullLegalName: formData.fullLegalName,
                        email: this.user.email,

                        description: '',

                        attachment: st.attachment_url,
                        attachmentFilename: st.attachment_filename,
                        attachmentType: st.attachment_type,

                        passportUpload: st.passport_url,
                        passportFilename: st.passport_filename,
                        passportUploadType: st.passport_type
                    });
                }

                if (formData.walletAddress) {
                    const userRs = await userService.update(this.user.current_user_id, {
                        profile: {
                            walletAddress: formData.walletAddress
                        }
                    })
                }

                if (rs) {
                    message.success('Success');
                    submissionService.path.push('/form/anniversary2018');
                }
            } catch (err) {
                console.error(err)
                message.error(err.message) // TODO: add rollbar?
            }
        }
    };
});
