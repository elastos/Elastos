import React from 'react'
import _ from 'lodash'
import { Popover, Icon } from 'antd'
import URI from 'urijs'
import I18N from '@/I18N'
import { loginRedirectWithQuery } from '@/util'
import { SUGGESTION_ABUSED_STATUS, SUGGESTION_STATUS } from '@/constant'
import BaseComponent from '@/model/BaseComponent'
import { Link } from 'react-router-dom'
import { ReactComponent as LikeIcon } from '@/assets/images/icon-like.svg'
import { ReactComponent as DislikeIcon } from '@/assets/images/icon-dislike.svg'
import { ReactComponent as CommentIcon } from '@/assets/images/icon-comment.svg'
import { ReactComponent as FollowIcon } from '@/assets/images/icon-follow.svg'
import { ReactComponent as FlagIcon } from '@/assets/images/icon-flag.svg'
import { ReactComponent as ArchiveIcon } from '@/assets/images/icon-archive.svg'

import './style.scss'

const IconText = ({ component, text, onClick, className = '' }) => (
  <div className={`cr-icon-group ${className}`} onClick={onClick}>
    <span>{component}</span>
    <span style={{ marginLeft: 16 }}>{text}</span>
  </div>
)

export default class extends BaseComponent {
  constructor(props) {
    super(props)
    const {
      currentUserId,
      data: {
        likesNum,
        dislikesNum,
        likes,
        dislikes,
        subscribers,
        abusedStatus,
        status
      }
    } = this.props
    const isLiked = _.includes(likes, currentUserId)
    const isDisliked = _.includes(dislikes, currentUserId)
    const isSubscribed = _.findIndex(
        subscribers,
        subscriber => subscriber.user === currentUserId
      ) !== -1
    const isAbused = abusedStatus === SUGGESTION_ABUSED_STATUS.REPORTED
    const isArchived = status === SUGGESTION_STATUS.ARCHIVED

    this.state = {
      isLiked,
      isDisliked,
      isSubscribed,
      isAbused,
      isArchived,
      likesNum,
      dislikesNum
    }

    this.listRefetch = props.listRefetch
  }

  componentDidMount() {
    const {
      location: { search },
      isLogin,
      data: { _id }
    } = this.props
    const uri = URI(search || '')
    const { action } = URI.parseQuery(search || '')

    if (!isLogin || !uri.hasQuery('id', _id) || !uri.hasQuery('action')) return

    this.handleClick(action)
  }

  ord_render() {
    const {
      data: { commentsNum, viewsNum, _id }
    } = this.props
    const popoverActions = this.renderPopover()
    const { isLiked, isDisliked, likesNum, dislikesNum } = this.state
    const likeClass = isLiked ? 'selected' : ''
    const dislikeClass = isDisliked ? 'selected' : ''
    const likeNode = (
      <IconText
        component={!!LikeIcon && <LikeIcon />}
        text={likesNum}
        onClick={this.handleClick('isLiked')}
        className={likeClass}
      />
    )

    const dislikeNode = (
      <IconText
        component={!!DislikeIcon && <DislikeIcon />}
        text={dislikesNum}
        onClick={this.handleClick('isDisliked')}
        className={dislikeClass}
      />
    )

    const commentNode = (
      <div className="cr-icon-group">
        <Link to={`/suggestion/${_id}/#comments`}>
          <IconText
            component={!!CommentIcon && <CommentIcon />}
            text={commentsNum}
          />
        </Link>
      </div>
    )

    const viewsNode = (
      <div className="cr-icon-group self-right">
        {`${viewsNum} ${I18N.get('suggestion.views').toLowerCase()}`}
      </div>
    )

    const result = (
      <div className="c_SuggestionActions">
        {likeNode}
        {dislikeNode}
        {commentNode}
        <div className="cr-icon-group">{popoverActions}</div>
        {viewsNode}
      </div>
    )
    return result
  }

  renderPopover() {
    const { isSubscribed, isAbused, isArchived } = this.state
    const content = (
      <div className="popover-actions">
        <IconText
          component={<Icon type="edit" />}
          text={I18N.get('suggestion.showEditHistory')}
          onClick={this.gotoEditHistory}
          className="history-icon"
        />
        <IconText
          component={!!FollowIcon && <FollowIcon />}
          text={I18N.get('suggestion.follow')}
          onClick={this.handleClick('isSubscribed')}
          className={`follow-icon ${isSubscribed ? 'selected' : ''}`}
        />
        <IconText
          component={!!FlagIcon && <FlagIcon />}
          text={I18N.get('suggestion.reportAbuse')}
          onClick={this.handleClick('isAbused')}
          className={`abuse-icon ${isAbused ? 'selected' : ''}`}
        />
        <IconText
          component={!!ArchiveIcon && <ArchiveIcon />}
          text={
            isArchived ? I18N.get('suggestion.unarchive') : I18N.get('suggestion.archive')
          }
          onClick={this.handleClick('isArchived')}
          className="archive-icon"
        />
      </div>
    )
    return (
      <Popover content={content} trigger="click">
        <Icon type="ellipsis" />
      </Popover>
    )
  }

  getActionParams(action) {
    // these are the actual action calls on Container
    const {
      like,
      dislike,
      subscribe,
      unsubscribe,
      reportAbuse,
      archiveOrUnarchive,
      data: { _id }
    } = this.props
    const unsubOrSub = this.state.isSubscribed ? unsubscribe : subscribe
    const actionMapping = {
      isLiked: like,
      isDisliked: dislike,
      isSubscribed: unsubOrSub,
      isAbused: reportAbuse,
      isArchived: archiveOrUnarchive,
    }
    const params = {
      callback: actionMapping[action],
      param: _id,
      state: action
    }
    if (action === 'isArchived') {
      params.param = { id: _id, isArchived: this.state.isArchived }
    }
    return params
  }

  gotoEditHistory = () => {
    const {
      data: { _id, editHistory },
      history,
      saveEditHistory
    } = this.props
    saveEditHistory(editHistory)
    history.push(`/suggestion/history/${_id}`)
  }

  // high order function to return a debounce function with param
  handleClick = action =>
    _.debounce(() => {
      this.handleClickWithoutDebounce(action)
    }, 300)

  // use setState to change UI state for better UX
  handleClickWithoutDebounce = async action => {
    // callback is a function defined in getActionParams
    const { callback, param, state } = this.getActionParams(action)
    const { refetch, isLogin, history } = this.props
    const {
      isLiked,
      isDisliked,
      isAbused,
      likesNum,
      dislikesNum,
    } = this.state

    if (!isLogin) {
      const query = { action: state, id: param }
      loginRedirectWithQuery({ query })
      history.push('/login')
      return
    }

    if (
      (state === 'isAbused' && isAbused) ||
      (state === 'isLiked' && isDisliked) ||
      (state === 'isDisliked' && isLiked)
    ) {
      return
    }
    try {
      await callback(param)
      if (refetch) {
        refetch()
      } else {
        if (state === 'isLiked') {
          const likesCount = likesNum > 0 ? likesNum - 1 : 0
          this.setState({ likesNum: isLiked ? likesCount : likesNum + 1 })
        } else if (state === 'isDisliked') {
          const dislikesCount = dislikesNum > 0 ? dislikesNum - 1 : 0
          this.setState({
            dislikesNum: isDisliked ? dislikesCount : dislikesNum + 1
          })
        } else if (state === 'isArchived') {
          this.listRefetch()
        }

        this.setState({ [state]: !this.state[state] })
      }
    } catch (error) {
      console.log('suggestion actions error...', error)
    }
  }
}
