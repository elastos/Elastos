import Base from './Base'
import * as _ from 'lodash'

export default class extends Base {
  private model: any
  protected init() {
    this.model = this.getDBModel('Release')
  }

  public async create(param: any): Promise<Document> {
    // get param
    const { title, desc } = param
    const doc: any = {
      title,
      desc,
      createdBy: _.get(this.currentUser, '_id'),
    }

    // save the document
    const result = await this.model.save(doc)

    return result
  }

  public async update(param: any): Promise<Document> {
    // get param
    const { id, title, desc } = param

    // build document object
    const doc: any = {
      title,
      desc,
    }

    await this.model.update({_id: id}, {$set: doc })

    return await this.show({ id })
  }

  public async list(): Promise<Object> {
    const list = await this.model.getDBInstance().find().sort({ createdAt: -1 })
    const total = list.length

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
}
