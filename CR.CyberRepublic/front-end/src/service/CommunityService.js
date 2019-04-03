import BaseService from '../model/BaseService'
import { api_request } from '@/util'
import { COMMUNITY_TYPE } from '@/constant'
import config from '@/config'

export default class extends BaseService {
  async addCountry(country) {
    await api_request({
      path: '/api/community/create',
      method: 'post',
      // Mock data
      data: {
        ...country,
        name: config.data.mappingCountryCodeToName[country.geolocation],
        parentCommunityId: null,
        type: COMMUNITY_TYPE.COUNTRY,
      },
    })
  }

  async getSpecificCountryCommunities(countryCode) {
    const result = await api_request({
      path: `/api/community/country/${countryCode}`,
      method: 'get',
    })

    return result
  }

  async getAllCountryCommunities() {
    const result = await api_request({
      path: '/api/community',
      method: 'get',
      data: {
        type: COMMUNITY_TYPE.COUNTRY,
      },
    })

    return result
  }

  async getAll(qry) {
    const communityRedux = this.store.getRedux('community')
    this.dispatch(communityRedux.actions.loading_update(true))

    const result = await api_request({
      path: '/api/community/all',
      method: 'get',
      data: qry,
    })

    this.dispatch(communityRedux.actions.loading_update(false))

    return result
  }

  async getMyCommunities(userId) {
    const communityRedux = this.store.getRedux('community')
    this.dispatch(communityRedux.actions.loading_update(true))

    const result = await api_request({
      path: '/api/community/all',
      method: 'get',
      data: {
        communityHasUser: userId,
      },
    })

    this.dispatch(communityRedux.actions.my_communities_reset())
    this.dispatch(communityRedux.actions.my_communities_update(result))
    this.dispatch(communityRedux.actions.loading_update(false))

    return result
  }

  async update(community) {
    const result = await api_request({
      path: '/api/community/update',
      method: 'put',
      data: {
        ...community,
        leaderIds: (typeof community.leaderIds === 'object' ? community.leaderIds.toString() : community.leaderIds),
      },
    })

    return result
  }

  async delete(communityId) {
    const result = await api_request({
      path: `/api/community/${communityId}`,
      method: 'delete',
    })

    return result
  }

  async getLeadersACountry(countryCode) {
    const result = await api_request({
      path: `/api/community/country/${countryCode}`, method: 'get',
    })

    return result
  }

  async getMembers(communityId) {
    const result = await api_request({
      path: `/api/community/${communityId}/members`, method: 'get',
    })

    return result
  }

  async getSubCommunities(parentCommunityId) {
    const result = await api_request({
      path: `/api/community/parent/${parentCommunityId}`, method: 'get',
    })

    return result
  }

  async get(communityId) {
    const result = await api_request({
      path: `/api/community/${communityId}`, method: 'get',
    })

    return result
  }

  async create(community) {
    const result = await api_request({
      path: '/api/community/create', method: 'POST', data: community,
    })

    return result
  }

  async addMember(memberId, communityId) {
    const result = await api_request({
      path: `/api/community/${communityId}/${memberId}`,
      method: 'post',
    })

    return result
  }

  async removeMember(memberId, communityId) {
    const result = await api_request({
      path: `/api/community/${communityId}/${memberId}`,
      method: 'delete',
    })

    return result
  }
}
