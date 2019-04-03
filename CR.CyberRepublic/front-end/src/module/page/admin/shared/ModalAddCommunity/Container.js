import {createContainer, goPath} from '@/util'
import Component from './Component'
import CommunityService from '@/service/CommunityService'
import _ from 'lodash'


export default createContainer(Component, (state) => {
  return {
    is_admin: state.user.is_admin
  }
}, () => {
  const communityService = new CommunityService()

  return {
    async getAllCommunities() {
      return new Promise((resolve, reject) => {
        communityService.getAll().then((data) => {
          const cascaderItems = data.map((item) => {
            return {
              value: item._id,
              label: item.name,
              parentId: item.parentCommunityId,
            }
          })

          const rootCascaderItems = _.filter(cascaderItems, {
            parentId: null
          })

          rootCascaderItems.forEach((rootCascaderItem) => {
            const children = _.filter(cascaderItems, {
              parentId: rootCascaderItem.value
            })

            if (children && children.length) {
              rootCascaderItem.children = children
            }
          })

          resolve(rootCascaderItems)
        }).catch((err) => {
          reject(err)
        })
      })
    }
  }
})
