import {createContainer} from '@/util'
import Component from './Component'
import SubmissionService from '@/service/SubmissionService'
import { message } from 'antd/lib/index'

export default createContainer(Component, (state) => {
  return {
    userId: state.user.current_user_id,
    walletAddress: state.user.profile.walletAddress,
    is_login: state.user.is_login
  }
}, () => {

  const submissionService = new SubmissionService()

  return {
  }
})
