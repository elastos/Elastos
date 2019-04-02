import Base from './Base'
import * as _ from 'lodash'
import { constant } from '../constant'
import { validate } from '../utility'

const emptyDoc = {
  title: '',
  desc: '',
  benefits: '',
  funding: '',
  timeline: undefined,
  link: [],
}

export default class extends Base {
  private model: any
  protected init() {
    this.model = this.getDBModel('Suggestion')
  }

  public async create(param: any): Promise<Document> {
    // get param
    const { title, desc, benefits, funding, timeline, link, } = param
    // validation
    this.validateTitle(title)
    this.validateDesc(desc)

    const docCore: any = {
      title,
      desc,
      benefits,
      // funding,
      // timeline,
      // link,
    }
    if (!_.isEmpty(funding)) {
      docCore.funding = funding
    }
    if (!_.isEmpty(timeline)) {
      docCore.timeline = timeline
    }
    if (!_.isEmpty(link)) {
      docCore.link = link
    }

    console.log('docCore is: ', docCore)

    // build document object
    const doc = {
      ...docCore,
      createdBy: _.get(this.currentUser, '_id'),
      editHistory: [emptyDoc, docCore],
    }
    // save the document
    return await this.model.save(doc)
  }

  public async update(param: any): Promise<Document> {
    // get param
    const { id, title, desc, benefits, funding, timeline, link } = param
    const userId = _.get(this.currentUser, '_id')
    const currDoc = await this.model.getDBInstance().findById(id)

    if (!currDoc) {
      throw 'Current document does not exist'
    }

    if (!userId.equals(_.get(currDoc, 'createdBy'))) {
      throw 'Only owner can edit suggestion'
    }

    // validation
    this.validateTitle(title)
    this.validateDesc(desc)

    // build document object
    const doc: any = {
      title,
      desc,
      benefits,
      funding,
      timeline,
      link,
    }
    if (_.isEmpty(link)) {
      doc.link = []
    }

    // update the document
    if (!_.isEmpty(currDoc.editHistory)) {
      await this.model.update({_id: id}, {$set: doc, $push: { editHistory: doc }})
    } else {
      const firstHistoryItem = {
        title: currDoc.title,
        desc: currDoc.desc,
        benefits: currDoc.benefits,
        funding: currDoc.funding,
        timeline: currDoc.timeline,
        link: currDoc.link,
      }
      // for old data
      await this.model.update({_id: id}, {$set: { ...doc, editHistory: [emptyDoc, firstHistoryItem, doc] }})
    }

    return await this.show({ id })
  }

  public async list(param: any): Promise<Object> {
    const query = _.omit(param, ['results', 'page', 'sortBy', 'sortOrder', 'filter', 'profileListFor', 'search'])
    const cursor = this.model.getDBInstance()
      .find(query)
      .populate('createdBy', constant.DB_SELECTED_FIELDS.USER.NAME)
    const totalCursor = this.model.getDBInstance().find(query).count()

    if (param.sortBy) {
      const sortObject = {}
      sortObject[param.sortBy] = _.get(constant.SORT_ORDER, param.sortOrder, constant.SORT_ORDER.DESC)
      cursor.sort(sortObject)
    }

    if (param.results) {
      const results = parseInt(param.results, 10)
      const page = parseInt(param.page, 10)
      cursor.skip(results * (page - 1)).limit(results)
    }

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
      .populate('reference', constant.DB_SELECTED_FIELDS.CVOTE.ID_STATUS)

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

  // like or unlike
  public async like(param: any): Promise<Document> {
    const { id: _id } = param
    const userId = _.get(this.currentUser, '_id')
    const doc = await this.model.findById(_id)
    const { likes, dislikes } = doc

    // can not both like and dislike, use ObjectId.equals to compare
    if (_.findIndex(dislikes, oid => userId.equals(oid)) !== -1) return doc

    // already liked, will unlike, use ObjectId.equals to compare
    if (_.findIndex(likes, oid => userId.equals(oid)) !== -1) {
      await this.model.findOneAndUpdate({ _id }, {
        $pull: { likes: userId },
        $inc: { likesNum: -1, activeness: -1 }
      })
    } else {
      // not like yet, will like it
      await this.model.findOneAndUpdate({ _id }, {
        $push: { likes: userId },
        $inc: { likesNum: 1, activeness: 1 }
      })
    }

    return this.model.findById(_id)
  }
  // dislike <=> undislike
  // can not both like and dislike
  public async dislike(param: any): Promise<Document> {
    const { id: _id } = param
    const userId = _.get(this.currentUser, '_id')
    const doc = await this.model.findById(_id)
    const { likes, dislikes } = doc

    // can not both like and dislike, use ObjectId.equals to compare
    if (_.findIndex(likes, oid => userId.equals(oid)) !== -1) return doc

    // already liked, will unlike, use ObjectId.equals to compare
    if (_.findIndex(dislikes, oid => userId.equals(oid)) !== -1) {
      await this.model.findOneAndUpdate({ _id }, {
        $pull: { dislikes: userId },
        $inc: { dislikesNum: -1, activeness: -1 }
      })
    } else {
      // not like yet, will like it
      await this.model.findOneAndUpdate({ _id }, {
        $push: { dislikes: userId },
        $inc: { dislikesNum: 1, activeness: 1 }
      })
    }

    return this.model.findById(_id)
  }

  public async reportabuse(param: any): Promise<Document> {
    const { id: _id } = param
    const updateObject = {
      abusedStatus: constant.SUGGESTION_ABUSED_STATUS.REPORTED
    }
    await this.model.findOneAndUpdate({ _id }, updateObject)
    return this.model.findById(_id)
  }

  /**
   * Council only
   */
  public async addTag(param: any): Promise<Document> {
    const { id: _id, type, desc } = param
    const currDoc = await this.model.getDBInstance().findById(_id)

    if (!currDoc) {
      throw 'Current document does not exist'
    }

    if (_.findIndex(currDoc.tags, (tagObj: any) => tagObj.type === type) !== -1) return currDoc

    const tag: any = {
      type,
      createdBy: _.get(this.currentUser, '_id'),
    }
    if (desc) tag.desc = desc
    const updateObject = {
      $addToSet: { tags: tag }
    }

    await this.model.findOneAndUpdate({ _id }, updateObject)
    return this.model.findById(_id)
  }

  /**
   * Admin only
   */
  public async abuse(param: any): Promise<Document> {
    const { id: _id } = param
    const updateObject = {
      status: constant.SUGGESTION_STATUS.ABUSED,
      abusedStatus: constant.SUGGESTION_ABUSED_STATUS.HANDLED
    }
    await this.model.findOneAndUpdate({ _id }, updateObject)
    return this.model.findById(_id)
  }

  public async archive(param: any): Promise<Document> {
    const { id: _id } = param
    const updateObject = {
      status: constant.SUGGESTION_STATUS.ARCHIVED,
    }
    await this.model.findOneAndUpdate({ _id }, updateObject)
    return this.model.findById(_id)
  }

  public async delete(param: any): Promise<Document> {
    const { id: _id } = param
    return this.model.findByIdAndDelete(_id)
  }

  /**
   * Utils
   */
  public validateTitle(title: String) {
    if (!validate.valid_string(title, 4)) {
      throw 'invalid title'
    }
  }

  public validateDesc(desc: String) {
    if (!validate.valid_string(desc, 1)) {
      throw 'invalid description'
    }
  }
}
