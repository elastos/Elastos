import Base from './Base'
import * as _ from 'lodash'
import { constant } from '../constant'
import { validate } from '../utility'

export default class extends Base {
  private model: any
  protected init() {
    this.model = this.getDBModel('Permission_Role')
  }

  public async create(param: any): Promise<Document> {
    // get param
    const doc = _.pick(param, ['role', 'resourceType', 'permissionId', 'isAllowed', 'httpMethod', 'url'])
    console.log(doc, '----------')
    // save the document
    return await this.model.save(doc)
  }

  public async list(param: any): Promise<Object> {
    const query = _.omit(param, ['results', 'page', 'sortBy', 'sortOrder', 'filter', 'profileListFor', 'search'])
    const cursor = this.model.getDBInstance()
      .find(query)
      // .populate('createdBy', constant.DB_SELECTED_FIELDS.USER.NAME)
    const totalCursor = this.model.getDBInstance().find(query).count()

    const list = await cursor
    const total = await totalCursor

    return {
      list,
      total,
    }
  }

  public async show(param: any): Promise<Document> {
    const { id: _id, incViewsNum } = param
    if (incViewsNum === 'true') {
      await this.model.findOneAndUpdate({ _id }, {
        $inc: { viewsNum: 1, activeness: 1 }
      })
    }
    const doc = await this.model.getDBInstance()
      .findById(_id)
      .populate('createdBy', constant.DB_SELECTED_FIELDS.USER.NAME)

    if (_.isEmpty(doc.comments)) return doc

    for (const comment of doc.comments) {
      for (const thread of comment) {
        await this.model.getDBInstance().populate(thread, {
          path: 'createdBy',
          select: `${constant.DB_SELECTED_FIELDS.USER.NAME} profile.avatar`
        })
      }
    }

    return doc
  }

  // TODO
  public async update(param: any): Promise<Document> {
    // TODO: checkPermission admin
    const { id: _id } = param
    const updateObject = {
      status: constant.SUGGESTION_STATUS.ARCHIVED,
    }
    await this.model.findOneAndUpdate({ _id }, updateObject)
    return this.model.findById(_id)
  }

  public async delete(param: any): Promise<Document> {
    // TODO: checkPermission admin
    const { id: _id } = param
    return this.model.findByIdAndDelete(_id)
  }
}
