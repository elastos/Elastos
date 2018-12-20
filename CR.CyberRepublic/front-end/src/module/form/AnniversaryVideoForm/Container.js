import {createContainer, goPath} from "@/util";
import Component from './Component';
import SubmissionService from '@/service/SubmissionService'
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

    return {
        async submitForm(formData, st){

            try {
                const rs = await submissionService.create({

                    title: 'Anniversary 2018 - Video',
                    type: SUBMISSION_TYPE.FORM_EXT,
                    campaign: SUBMISSION_CAMPAIGN.ANNI_VIDEO_2008,

                    fullLegalName: formData.fullLegalName,
                    email: this.user.email,
                    location: formData.location,

                    description: '',

                    attachment: st.attachment_url,
                    attachmentFilename: st.attachment_filename,
                    attachmentType: st.attachment_type
                });

                if (rs) {
                    message.success('Success');
                    submissionService.path.push('/profile/submissions');
                }
            } catch (err) {
                console.error(err)
                message.error(err.message) // TODO: add rollbar?
            }
        }
    };
});
