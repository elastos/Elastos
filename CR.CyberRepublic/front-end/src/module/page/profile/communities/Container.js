import {createContainer} from '@/util'
import Component from './Component'
import CommunityService from '@/service/CommunityService'
import _ from 'lodash'

export default createContainer(Component, (state) => {

  const profileState = {
    currentUserId: state.user.current_user_id,
    profileCountry: state.user.profile.country,
    myCommunities: state.community.my_communities,
    loading: state.community.loading,
  }
  if (!_.isArray(profileState.myCommunities)) {
    profileState.myCommunities = _.values(state.community.my_communities)
  }

  return profileState
}, () => {
  const communityService = new CommunityService()

  return {
    async getMyCommunities(currentUserId) {
      return communityService.getMyCommunities(currentUserId)
    },
    async removeMember(memberId, communityId) {
      return communityService.removeMember(memberId, communityId)
    }
  }
})
