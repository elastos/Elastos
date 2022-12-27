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
    async getCommunityDetail(communityId) {
      return communityService.get(communityId)
    },
    async getSubCommunities(parentCommunityId) {
      return communityService.getSubCommunities(parentCommunityId)
    },
    async createSubCommunity(community) {
      return communityService.create(community)
    },
    async updateCommunity(community) {
      return communityService.update(community)
    },
    async deleteCommunity(communityId) {
      return communityService.delete(communityId)
    },
    async getUserByIds (ids) {
      return userService.getByIds(ids)
    },
    async getAllUsers() {
      return userService.getAll()
    }
  }
})
