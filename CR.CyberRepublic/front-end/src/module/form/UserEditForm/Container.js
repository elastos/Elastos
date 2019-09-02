import { createContainer } from '@/util'
import Component from './Component'
import UserService from '@/service/UserService'
import { message } from 'antd'
import _ from 'lodash'
import { logger } from '@/util'

message.config({
  top: 100,
})


export default createContainer(Component, state => ({
  is_admin: state.user.is_admin,
  loading: state.member.loading,
}), () => {
  const userService = new UserService()

  return {
    async getCurrentUser() {
      try {
        await userService.getCurrentUser()
      } catch (err) {
        message.error(err.message)
        logger.error(err)
      }
    },

    async updateUser(userId, formData) {
      const doc = {
        email: formData.email,
        username: formData.username,
        password: formData.password,
        profile: {
          // General
          firstName: formData.firstName,
          lastName: formData.lastName,
          gender: formData.gender,
          country: formData.country,
          timezone: formData.timezone,
          skillset: formData.skillset,
          avatar: formData.avatar,
          walletAddress: formData.walletAddress,
          profession: formData.profession,
          portfolio: formData.portfolio,
          bio: formData.bio,
          motto: formData.motto,

          // Social Media
          telegram: formData.telegram,
          reddit: formData.reddit,
          wechat: formData.wechat,
          twitter: formData.twitter,
          facebook: formData.facebook,
          linkedin: formData.linkedin,
          github: formData.github,

          // Questions
          beOrganizer: formData.beOrganizer === 'yes',
          isDeveloper: formData.isDeveloper === 'yes',
        },
      }

      return userService.update(userId, doc)
    },

    async updateRole(userId, formData) {
      const doc = {
        role: formData.role,
      }

      return userService.updateRole(userId, doc)
    },

    async checkEmail(email) {
      try {
        await userService.checkEmail(email)
        return false
      } catch (err) {
        return true
      }
    },
  }
})
