import { message } from 'antd'
import _ from 'lodash'
import { createContainer, logger } from '@/util'
import Component from './Component'
import CommentService from '@/service/CommentService'
import CouncilService from '@/service/CouncilService'
import I18N from '@/I18N'


export default createContainer(Component, (state) => {
  const commentables = ['task', 'submission', 'team', 'member', 'elip', 'suggestion']

  const props = {
    currentUserId: state.user.current_user_id,
    all_users: _.values(state.council.council_members || []),
    loading: {},
  }

  _.each(commentables, (commentable) => {
    props[commentable] = state[commentable].detail
    props.loading[commentable] = state[commentable].loading
  })

  return props
}, () => {
  const commentService = new CommentService()
  const councilService = new CouncilService()

  return {
    async postComment(type, reduxType, detailReducer, returnUrl, parentId, comment, headline) {
      try {
        const rs = await commentService.postComment(type, reduxType, detailReducer,
          returnUrl, parentId, comment, headline)

        if (rs) {
          message.success(I18N.get('comments.posted.success'))
        }
      } catch (err) {
        message.error(err.message)
        logger.error(err)
      }
    },

    async updateComment(type, reduxType, detailReducer, parentId, param) {
      try {
        const rs = await commentService.updateComment(type, reduxType, detailReducer,
          parentId, param)

        if (rs) {
          message.success(I18N.get('comments.updated.success'))
        }
      } catch (err) {
        message.error(err.message)
        logger.error(err)
      }
    },

    async removeComment(type, reduxType, detailReducer, parentId, param) {
      try {
        const rs = await commentService.removeComment(type, reduxType, detailReducer,
          parentId, param)

        if (rs) {
          message.success(I18N.get('comments.deleted.success'))
        }
      } catch (err) {
        message.error(err.message)
        logger.error(err)
      }
    },

    async listUsers() {
      try {
        return await councilService.getCouncilMembers()
      } catch (err) {
        message.error(err.message)
        logger.error(err)
      }
    },

    async subscribe(type, parentId) {
      try {
        await commentService.subscribe(type, parentId)
      } catch (err) {
        message.error(err.message)
        logger.error(err)
      }
    },

    async unsubscribe(type, parentId) {
      try {
        await commentService.unsubscribe(type, parentId)
      } catch (err) {
        message.error(err.message)
        logger.error(err)
      }
    },
  }
})
