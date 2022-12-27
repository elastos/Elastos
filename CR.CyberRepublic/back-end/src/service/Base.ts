import * as _ from 'lodash'
import { constant } from '../constant'

export default class Base {
  protected db
  private session
  protected currentUser

  constructor(db, session) {
    this.db = db
    this.session = session
    this.currentUser = session.user

    this.init()
  }

  protected init() {}

  public getDBModel(name: string) {
    return this.db.getModel(name)
  }

  protected getService<T extends Base>(service: { new (...args): T }): T {
    return new service(this.db, this.session)
  }

  protected async markLastSeenComment(commentable, createdBy, db_commentable) {
    if (!this.currentUser) {
      return
    }

    if (commentable.comments && commentable.comments.length) {
      const subscriberInfo = _.find(commentable.subscribers, subscriber => {
        return (
          subscriber.user &&
          subscriber.user._id.toString() === this.currentUser._id.toString()
        )
      })

      if (subscriberInfo) {
        subscriberInfo.lastSeen = new Date()
      } else if (
        createdBy &&
        createdBy._id.toString() === this.currentUser._id.toString()
      ) {
        commentable.lastCommentSeenByOwner = new Date()
      }

      await db_commentable.update(
        { _id: commentable._id },
        {
          subscribers: commentable.subscribers,
          lastCommentSeenByOwner: commentable.lastCommentSeenByOwner
        }
      )
    }
  }

  /**
   * We trust this.currentUser because it was fetched during each request in the middleware
   * via a back-end encrypted token of the userId
   *
   * @returns {boolean}
   */
  protected isLoggedIn() {
    let isLoggedIn = false

    if (this.currentUser && this.currentUser._id) {
      isLoggedIn = true
    }

    return isLoggedIn
  }

  protected isAdmin() {
    return this.currentUser.role === constant.USER_ROLE.ADMIN
  }
}
