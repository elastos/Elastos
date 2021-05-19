import Base from '../Base'
import * as _ from 'lodash'
import { Types } from 'mongoose'
import SuggestionService from '../../service/SuggestionService'

const ObjectId = Types.ObjectId

const FILTERS = {
  ALL: 'all',
  CREATED: 'createdBy',
  COMMENTED: 'commented',
  SUBSCRIBED: 'subscribed',
  ARCHIVED: 'archived'
}

export default class extends Base {
  protected needLogin = false

  /**
   * For consistency we call the service
   * with the entire query
   *
   * @param param
   * @returns {Promise<["mongoose".Document]>}
   */
  public async action() {
    const service = this.buildService(SuggestionService)
    const param = this.getParam()

    if (param.profileListFor) {
      const currentUserId = new ObjectId(param.profileListFor)
      // make sure this is the logged in user
      if (this.session.userId !== currentUserId.toString()) {
        throw 'suggestion.list API - profileListFor does not match session.userId'
      }

      param.$or = []
      if (_.includes([FILTERS.ALL, FILTERS.CREATED], param.filter)) {
        param.$or.push({ createdBy: currentUserId })
      }
      if (_.includes([FILTERS.ALL, FILTERS.COMMENTED], param.filter)) {
        param.$or.push({
          comments: { $elemMatch: { $elemMatch: { createdBy: currentUserId } } }
        })
      }
      if (_.includes([FILTERS.ALL, FILTERS.SUBSCRIBED], param.filter)) {
        param.$or.push({ 'subscribers.user': currentUserId })
      }
      if (_.includes([FILTERS.ARCHIVED], param.filter)) {
        param.$or.push({ status: 'ARCHIVED', createdBy: currentUserId })
      }
    }
    if (param.search) {
      let or = [
          { title: { $regex: _.trim(param.search), $options: 'i' } },
          { vid: _.toNumber(_.trim(param.search)) || 0 }
      ]
      if(param.$or){
        param.$or = param.$or.concat(or)
      }else{
        param.$or = or
      }
    }

    const result = await service.list(param)

    return this.result(1, result)
  }
}
