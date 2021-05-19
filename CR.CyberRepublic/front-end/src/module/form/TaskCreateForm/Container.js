import {createContainer, goPath} from '@/util'
import Component from './Component'
import TaskService from '@/service/TaskService'
import TeamService from '@/service/TeamService'
import CommunityService from '@/service/CommunityService'
import {message} from 'antd'
import {TEAM_TYPE} from '@/constant'
import _ from 'lodash'
import { logger } from '@/util'

message.config({
  top: 100
})

export default createContainer(Component, (state) => {
  return {
    is_admin: state.user.is_admin,
    loading: state.task.loading || state.team.loading,
    all_circles: state.team.all_circles,
    all_circles_loading: state.team.all_circles_loading
  }
}, () => {
  const taskService = new TaskService()
  const teamService = new TeamService()
  const communityService = new CommunityService()

  return {
    async createTask(formData, st) {
      try {
        const createObj = {

          assignSelf: formData.assignSelf,

          name: formData.taskName,
          category: formData.taskCategory,
          type: formData.taskType,
          bidding: formData.bidding,
          referenceBid: formData.referenceBid,

          applicationDeadline: formData.taskApplicationDeadline,
          completionDeadline: formData.taskCompletionDeadline,
          goals: formData.taskGoals,

          description: formData.taskDesc,
          descBreakdown: formData.taskDescBreakdown,

          eventDateRange: formData.eventDateRange,
          eventDateRangeStart: formData.eventDateRangeStart,
          eventDateRangeEnd: formData.eventDateRangeEnd,
          eventDateStatus: formData.eventDateStatus,
          location: formData.taskLocation,
          readDisclaimer: formData.readDisclaimer,

          infoLink: formData.taskLink,
          circle: formData.circle,

          thumbnail: st.thumbnail_url,
          thumbnailFilename: st.thumbnail_filename,
          thumbnailType: st.thumbnail_type,

          attachment: st.attachment_url,
          attachmentFilename: st.attachment_filename,
          attachmentType: st.attachment_type,

          pitch: formData.pitch,
          pictures: formData.pictures,
          domain: formData.domain,
          recruitedSkillsets: formData.recruitedSkillsets,
          community: formData.community,
          communityParent: formData.communityParent,

          candidateLimit: formData.taskCandLimit,
          candidateSltLimit: formData.taskCandSltLimit
        }

        Object.assign(createObj, {
          rewardUpfront: {
            ela: formData.taskRewardUpfront * 1000,
            elaDisbursed: 0,

            usd: formData.taskRewardUpfrontUsd * 100,
            elaPerUsd: formData.taskRewardUpfrontElaPerUsd,
            isUsd: st.isUsd
          },
          reward: {
            ela: formData.taskReward ? formData.taskReward * 1000 : null,
            elaDisbursed: 0,
            votePower: parseFloat(formData.reward) * 100,

            usd: formData.taskRewardUsd * 100,
            elaPerUsd: formData.taskRewardElaPerUsd,
            isUsd: st.isUsd
          }
        })

        const rs = await taskService.create(createObj)

        if (rs) {
          message.success('Task created successfully')
          taskService.path.push(`/profile/task-detail/${rs._id}`)
        }
      } catch (err) {
        message.error(err.message)
        logger.error(err)
      }
    },

    async updateTask(formData, st) {

      const taskId = this.existingTask._id

      try {
        const updateObj = {
          assignSelf: formData.assignSelf,
          name: formData.taskName,
          category: formData.taskCategory,
          type: formData.taskType,
          bidding: formData.bidding,
          referenceBid: formData.referenceBid,

          applicationDeadline: formData.taskApplicationDeadline,
          completionDeadline: formData.taskCompletionDeadline,

          description: formData.taskDesc,
          descBreakdown: formData.taskDescBreakdown,
          goals: formData.taskGoals,
          readDisclaimer: formData.readDisclaimer,

          circle: formData.circle,
          eventDateRange: formData.eventDateRange,
          eventDateRangeStart: formData.eventDateRangeStart,
          eventDateRangeEnd: formData.eventDateRangeEnd,
          eventDateStatus: formData.eventDateStatus,
          location: formData.taskLocation,

          infoLink: formData.taskLink,
          thumbnail: st.upload_url,
          pictures: formData.pictures,
          domain: formData.domain,
          pitch: formData.pitch,
          recruitedSkillsets: formData.recruitedSkillsets,
          community: !formData.community ? null : formData.community,
          communityParent: formData.communityParent,

          // TODO: attachment

          candidateLimit: formData.taskCandLimit,
          candidateSltLimit: formData.taskCandSltLimit,
        }

        Object.assign(updateObj, {
          rewardUpfront: {
            ela: formData.taskRewardUpfront ? formData.taskRewardUpfront * 1000 : null,
            elaDisbursed: 0,

            usd: formData.taskRewardUpfrontUsd ? formData.taskRewardUpfrontUsd * 100 : null,
            elaPerUsd: formData.taskRewardUpfrontElaPerUsd,
            isUsd: st.isUsd
          },
          reward: {
            ela: formData.taskReward ? formData.taskReward * 1000 : null,
            elaDisbursed: 0,

            usd: formData.taskRewardUsd ? formData.taskRewardUsd * 100 : null,
            elaPerUsd: formData.taskRewardElaPerUsd,
            isUsd: st.isUsd
          }
        })

        if (st.removeAttachment) {
          Object.assign(updateObj, {
            attachment: null,
            attachmentFilename: null,
            attachmentType: null
          })
        } else if (st.attachment_url) {
          Object.assign(updateObj, {
            attachment: st.attachment_url,
            attachmentFilename: st.attachment_filename,
            attachmentType: st.attachment_type,
          })
        }

        if (st.removeThumbnail) {
          Object.assign(updateObj, {
            thumbnail: null,
            thumbnailFilename: null,
            thumbnailType: null
          })
        } else if (st.thumbnail_url) {
          Object.assign(updateObj, {
            thumbnail: st.thumbnail_url,
            thumbnailFilename: st.thumbnail_filename,
            thumbnailType: st.thumbnail_type,
          })
        }

        const rs = await taskService.update(taskId, updateObj)

        if (rs) {
          message.success('Task updated successfully')

          st.editing = false
          // this.setState({editing: false})
        }
      } catch (err) {
        message.error(err.message)
        logger.error(err)
      }
    },

    async getTaskDetail (taskId) {
      return taskService.get(taskId)
    },

    resetTaskDetail() {
      return taskService.resetTaskDetail()
    },

    async getAllCircles() {
      return teamService.loadAllCircles()
    },

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
