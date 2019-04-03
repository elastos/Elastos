import {createContainer} from '@/util'
import Component from './Component'
import CommunityService from '@/service/CommunityService'
import UserService from '@/service/UserService'

export default createContainer(Component, (state, ownProps) => {
  return {}
}, () => {

  const communityService = new CommunityService()
  const userService = new UserService()

  return {
    async getAllCountryCommunity () {
      return communityService.getAllCountryCommunities()
    },
    async getSpecificCountryCommunities (countryCode) {
      return communityService.getSpecificCountryCommunities(countryCode)
    },
    async addCountry (country) {
      return communityService.addCountry(country)
    },
    async getUserByIds (ids) {
      return userService.getByIds(ids)
    },
    async getAllUsers() {
      return userService.getAll()
    }
  }
})
