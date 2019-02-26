import Base from './Base'
import * as _ from 'lodash'
import { constant } from '../constant'
import { validate } from '../utility'

export default class extends Base {
  private model: any
  protected init() {
    this.model = this.getDBModel('Permission')
  }

  public async create(param: any): Promise<Document> {
    // get param
    const doc = _.pick(param, ['name', 'desc', 'resourceType', 'url', 'httpMethod'])
    const existedDoc = await this.model.findOne(_.pick(param, [ 'resourceType', 'url', 'httpMethod']))

    // every permission role is unique
    if (existedDoc) return existedDoc
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
    const { id: _id } = param
    const doc = await this.model.getDBInstance()
      .findById(_id)

    return doc
  }

  public async update(param: any): Promise<Document> {
    // TODO: checkPermission admin
    const { id: _id } = param
    const updateObject = _.pick(param, ['name', 'desc', 'resourceType', 'url', 'httpMethod'])
    await this.model.findOneAndUpdate({ _id }, updateObject)
    return this.model.findById(_id)
  }

  public async delete(param: any): Promise<Document> {
    // TODO: checkPermission admin
    const { id: _id } = param
    return this.model.findByIdAndDelete(_id)
  }
}
