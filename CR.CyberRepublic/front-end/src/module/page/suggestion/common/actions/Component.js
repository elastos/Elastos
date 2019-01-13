import React from 'react';
import _ from 'lodash'
import { Popover, Icon } from 'antd';
import I18N from '@/I18N'
import { SUGGESTION_ABUSED_STATUS } from '@/constant'
import BaseComponent from '@/model/BaseComponent'

import { ReactComponent as LikeIcon } from '@/assets/images/icon-like.svg'
import { ReactComponent as DislikeIcon } from '@/assets/images/icon-dislike.svg'
import { ReactComponent as CommentIcon } from '@/assets/images/icon-comment.svg'
import { ReactComponent as FollowIcon } from '@/assets/images/icon-follow.svg'
import { ReactComponent as FlagIcon } from '@/assets/images/icon-flag.svg'

import './style.scss'

const IconText = ({
  component, text, onClick, className = '',
}) => (
  <div className={`cr-icon-group ${className}`} onClick={onClick}>
    <span>{component}</span>
    <span style={{ marginLeft: 16 }}>{text}</span>
  </div>
)

export default class extends BaseComponent {
  constructor(props) {
    super(props)
    const {
      currentUserId, data: {
        likesNum, dislikesNum, likes, dislikes, subscribers, abusedStatus,
      },
    } = this.props
    const isLiked = _.includes(likes, currentUserId)
    const isDisliked = _.includes(dislikes, currentUserId)
    const isSubscribed = _.findIndex(subscribers,
      subscriber => subscriber.user === currentUserId) !== -1
    const isAbused = abusedStatus === SUGGESTION_ABUSED_STATUS.REPORTED

    this.state = {
      isLiked,
      isDisliked,
      isSubscribed,
      isAbused,
      likesNum,
      dislikesNum,
    }
  }

  ord_render() {
    const { data, like, dislike } = this.props
    const popoverActions = this.renderPopover()
    const { commentsNum, viewsNum, _id } = data
    const {
      isLiked, isDisliked, likesNum, dislikesNum,
    } = this.state
    const likeClass = isLiked ? 'selected' : ''
    const dislikeClass = isDisliked ? 'selected' : ''
    const likeNode = (
      <IconText
        component={!!LikeIcon && <LikeIcon />}
        text={likesNum}
        onClick={() => this.handleClick({ callback: like, param: _id, state: 'isLiked' })}
        className={likeClass}
      />
    )

    const dislikeNode = (
      <IconText
        component={!!DislikeIcon && <DislikeIcon />}
        text={dislikesNum}
        onClick={() => this.handleClick({ callback: dislike, param: _id, state: 'isDisliked' })}
        className={dislikeClass}
      />
    )

    const commentNode = (
      <div className="cr-icon-group">
        <IconText component={!!CommentIcon && <CommentIcon />} text={commentsNum} />
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
    const {
      subscribe, unsubscribe, reportAbuse, data: { _id },
    } = this.props
    const { isSubscribed, isAbused } = this.state
    const subscribeCallback = isSubscribed ? unsubscribe : subscribe
    const content = (
      <div className="popover-actions">
        <IconText
          component={!!FollowIcon && <FollowIcon />}
          text={I18N.get('suggestion.follow')}
          onClick={() => this.handleClick({
            callback: subscribeCallback, param: _id, state: 'isSubscribed', before: true,
          })}
          className={`follow-icon ${isSubscribed ? 'selected' : ''}`}
        />
        <IconText
          component={!!FlagIcon && <FlagIcon />}
          text={I18N.get('suggestion.reportAbuse')}
          onClick={() => this.handleClick({ callback: reportAbuse, param: _id, state: 'isAbused' })}
          className={`abuse-icon ${isAbused ? 'selected' : ''}`}
        />
      </div>
    )
    return (
      <Popover content={content} trigger="click">
        <Icon type="ellipsis" />
      </Popover>
    )
  }

  // use setState for better UX
  // before param is used to setState before api request which will cause rerendering
  // and get error of setState on unmounted component
  handleClick = async ({ callback, param, state }) => {
    const { refetch } = this.props
    const {
      isLiked, isDisliked, isAbused, likesNum, dislikesNum,
    } = this.state
    if ((state === 'isAbused' && isAbused)
        || (state === 'isLiked' && isDisliked)
        || (state === 'isDisliked' && isLiked)) {
      return
    }
    try {
      await callback(param)

      if (refetch) {
        refetch()
      } else {
        if (state === 'isLiked') {
          this.setState({ likesNum: isLiked ? likesNum - 1 : likesNum + 1 })
        } else if (state === 'isDisliked') {
          this.setState({ dislikesNum: isDisliked ? dislikesNum - 1 : dislikesNum + 1 })
        }

        this.setState({ [state]: !this.state[state] })
      }
    } catch (error) {
      // err happened
    }
  }
}
