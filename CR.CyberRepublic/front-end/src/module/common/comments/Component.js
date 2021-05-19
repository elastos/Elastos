import React from 'react'
import {
  Form,
  Col,
  Row,
  List,
  Avatar,
  Icon,
  Button,
  Input,
  Mention,
  Modal,
  Popconfirm
} from 'antd'
import _ from 'lodash'
import moment from 'moment'
import MediaQuery from 'react-responsive'
import linkifyStr from 'linkifyjs/string'
import BaseComponent from '@/model/BaseComponent'
import ProfilePopup from '@/module/profile/OverviewPopup/Container'
import Translation from '@/module/common/Translation/Container'
import sanitizeHtml from '@/util/html'
import userUtil from '@/util/user'
import I18N from '@/I18N'
import {
  MAX_WIDTH_MOBILE,
  MIN_WIDTH_PC,
  MAX_LENGTH_COMMENT
} from '@/config/constant'
import { USER_AVATAR_DEFAULT, LINKIFY_OPTION } from '@/constant'
import './style.scss'

const {TextArea} = Input
const FormItem = Form.Item

class C extends BaseComponent {
  ord_states() {
    return {
      showUserInfo: null,
    }
  }

  async componentDidMount() {
    this.props.listUsers()
    const {hash} = this.props.location
    if (hash && hash === '#comments') {
      document.getElementById('comments').scrollIntoView({
        behavior: 'smooth',
        block: 'nearest'
      })
    }
  }

  componentWillUnmount() {
  }

  linkUserDetail(user) {
    this.setState({
      showUserInfo: user,
    })
  }

  // only wraps loading / renderMain
  ord_render() {
    return (
      this.renderMain()
    )
  }

  // header + main area
  renderMain() {
    return (
      <div className="c_Comments" id={this.props.id}>
        {this.renderHeader()}
        {this.renderComments()}
        <Modal
          className="profile-overview-popup-modal"
          visible={!!this.state.showUserInfo}
          onCancel={this.handleCancelProfilePopup.bind(this)}
          footer={null}
        >
          { this.state.showUserInfo
            && <ProfilePopup showUserInfo={this.state.showUserInfo} />
          }
        </Modal>
      </div>
    )
  }

  handleCancelProfilePopup() {
    this.setState({
      showUserInfo: null,
    })
  }

  renderHeader() {
    return (
      <h3 className="no-margin with-gizmo">{this.props.header || I18N.get('comments')}</h3>
    )
  }

  getInputProps() {
    const { getFieldDecorator } = this.props.form

    const headline_fn = getFieldDecorator('headline', {
      rules: [{
        max: 100, message: 'Headline is too long',
      }, {
        required: this.props.headlines, message: 'Please input headline!',
      }],
      initialValue: '',
    })
    const headline_el = (
      <Input placeholder="Headline" />
    )

    return {
      headline: headline_fn(headline_el),
    }
  }

  renderComment() {
    // const allUsers = _.map(this.props.all_users, user => user.username)
    const allUsers = [`ALL (${I18N.get('suggestion.form.mention.allCouncil')})`]
    _.each(this.props.all_users, obj => {
      const mentionStr = `${obj.username} (${userUtil.formatUsername(obj)})`
      allUsers.push(mentionStr)
    })

    const { getFieldDecorator } = this.props.form
    const comment_fn = getFieldDecorator('comment', {
      rules: [],
      initialValue: Mention.toContentState(''),
    })
    const comment_el = (
      <Mention
        multiLines={true}
        style={{ width: '100%', height: 100 }}
        defaultSuggestions={allUsers}
        notFoundContent={I18N.get('mentions.notFound')}
        placeholder={I18N.get('comments.placeholder')}
      />
    )

    return comment_fn(comment_el)
  }

  renderCommentMobile() {
    const allUsers = _.map(this.props.all_users, user => user.username)
    const { getFieldDecorator } = this.props.form
    const comment_fn = getFieldDecorator('comment', {
      rules: [],
      initialValue: '',
    })
    const comment_el = (
      <TextArea
        style={{ width: '100%', height: 100 }}
        placeholder={I18N.get('comments.placeholder')}
      />
    )

    return comment_fn(comment_el)
  }

  isUserSubscribed() {
    const curDetail = this.props[this.props.reduxType || this.props.type]
    const subscribers = curDetail.subscribers || []
    return !!_.find(subscribers, subscriber => subscriber.user && subscriber.user._id === this.props.currentUserId)
  }

  isLoading() {
    return this.props.loading[this.props.reduxType || this.props.type]
  }

  getSubscribeButton() {
    if (this.isUserSubscribed() && this.props.canSubscribe) {
      return (
        <Button
          className="ant-btn-ebp pull-left"
          size="small"
          onClick={this.unsubscribe.bind(this)}
          loading={this.isLoading()}
        >
          Unsubscribe
        </Button>
      )
    }

    return this.props.canSubscribe
      ? (
        <Button
          className="ant-btn-ebp pull-left"
          size="small"
          onClick={this.subscribe.bind(this)}
          loading={this.isLoading()}
        >
          Subscribe
        </Button>
      ) : null
  }

  getFooter() {
    if (!this.props.currentUserId) {
      return <div />
    }

    const p = this.getInputProps()
    const subscribeButton = this.getSubscribeButton()

    // TODO - canSubscribe requires canPost here, could be improved
    return this.props.canPost
      ? (
        <Form onSubmit={this.handleSubmit.bind(this)} className="c_commentForm">
          { this.props.headlines
            && (
              <FormItem>
                {p.headline}
              </FormItem>
            )
          }
          <MediaQuery minWidth={MIN_WIDTH_PC}>
            <FormItem>
              {this.renderComment()}
            </FormItem>
          </MediaQuery>
          <MediaQuery maxWidth={MAX_WIDTH_MOBILE}>
            <FormItem>
              {this.renderCommentMobile()}
            </FormItem>
          </MediaQuery>
          <FormItem>
            {subscribeButton}
            <Button
              className="ant-btn-ebp pull-right"
              type="primary"
              size="small"
              htmlType="submit"
              loading={this.isLoading()}
            >
              {I18N.get('comments.post')}
            </Button>
          </FormItem>
        </Form>
      ) : null
  }

  getModelId() {
    return _.isString(this.props.model) // Bit naive IMPROVEME
      ? this.props.model
      : this.props.model._id
  }

  subscribe() {
    this.props.subscribe(this.props.type, this.getModelId())
  }

  unsubscribe() {
    this.props.unsubscribe(this.props.type, this.getModelId())
  }

  renderComments() {
    let curDetail = this.props[this.props.reduxType || this.props.type]

    if (this.props.detailReducer) {
      curDetail = this.props.detailReducer(curDetail) || {}
    }

    const comments = curDetail.comments || []
    const dateFormatter = createdAt => (createdAt ? moment(createdAt).format('MMM D - h:mma') : '')

    const footer = this.getFooter()

    const enrichComment = (comment) => {
      if (!comment) {
        return
      }

      const mentions = Mention.getMentions(Mention.toContentState(comment))

      /*
        ** Format visuals
        */
      const spanCreator = (content, key) => <div key={key} className="non-mention" dangerouslySetInnerHTML={{ __html: sanitizeHtml(content) }} />
      const mentionCreator = (mention, key) => (
        <a key={key} onClick={() => this.showUserProfile(mention.replace('@', ''))}>
          {mention}
        </a>
      )

      /*
        ** Recursive
        ** Convert a comment into a lookup, split by mentions
        */
      const splitter = (mentionIndex, content) => {
        if (mentionIndex >= mentions.length) {
          return content
        }

        const mention = mentions[mentionIndex]
        const splits = content.split(mention)
        const innerSplits = _.map(splits, split => splitter(mentionIndex + 1, split))

        return {
          splits: innerSplits,
          connector: mention,
        }
      }

      /*
        ** Recursive
        ** Format the lookup into visual components
        */
      const formatSplit = (split, ind = 0) => {
        if (_.isString(split)) {
          return spanCreator(split, ind)
        }

        /*
          ** Mentions go between the splits, so for an array of N splits,
          ** we want to 'cross' it with an array of N-1 mentions.
          ** [A,B,C] cross [X,Y] = [A,X,B,Y,C]
          **
          ** Note the React key magic, splits get keys equal to their indices,
          ** whereas mentions indices are offset by the splits length for uniqueness
          ** (though the result is not sequential)
          */
        const innerSplits = _.map(split.splits, formatSplit)
        const keyedMentions = _.times(innerSplits.length - 1,
          index => mentionCreator(split.connector, innerSplits.length + index))

        return _.compact(_.flatten(_.zip(innerSplits, keyedMentions)))
      }

      // Build the lookup
      const fragments = splitter(0, comment)

      // Format the lookup
      const formatted = formatSplit(fragments)

      return (
        <div>
          {formatted}
        </div>
      )
    }

    const avatarItem = (info) => {
      const profile = info && info.profile
      const { avatar, firstName, lastName} = profile || {}

      if (avatar || (!firstName && !lastName)) {
        return (
          <Avatar
            className="comment-avatar pull-left"
            src={avatar || USER_AVATAR_DEFAULT}
            shape="circle"
            size={64}
            onClick={() => this.linkUserDetail(info)}
        />
        )
      }

      if (firstName || lastName) {
        return (
          <Avatar
            className="comment-avatar pull-left"
            style={{
              backgroundColor: '#000',
              fontSize: 24
            }}
            shape="circle"
            size={64}
            onClick={() => this.linkUserDetail(info)}
          >
            {`${firstName && firstName.toUpperCase().substr(0, 1)}${lastName && lastName.toUpperCase().substr(0, 1)}`}
          </Avatar>
        )
      }
    }

    const commentItems = _.map(comments, (comment, ind) => {
      const thread = _.first(comment)
      const createdByUsername = (thread.createdBy && thread.createdBy.username) || ''
      const avatar = (thread.createdBy && thread.createdBy.profile && thread.createdBy.profile.avatar) || USER_AVATAR_DEFAULT
      const createdById = (thread.createdBy && thread.createdBy._id)
      const dateFormatted = dateFormatter(thread.createdAt)
      const isDeletable = thread.createdBy && (thread.createdBy._id === this.props.currentUserId)
      const linkifyComment = linkifyStr(thread.comment || '', LINKIFY_OPTION)
      return {
        comment: linkifyComment,
        headline: thread.headline,
        description: (
          <div className="commenter-info">
            <a onClick={() => this.linkUserDetail(thread.createdBy)}>
              {createdByUsername}
            </a>
            {dateFormatted && (
              <span>
                <span className="date-colon">, </span>
                <span className="date">{dateFormatted}</span>
              </span>
            )}
          </div>
        ),
        avatar: avatarItem(thread.createdBy),
        delete: isDeletable && (
          <h5>
            <Popconfirm
              title={I18N.get('comments.delete.confirm')}
              onConfirm={() => this.handleDelete(thread)}
              okText={I18N.get('.yes')}
              cancelText={I18N.get('.no')}
            >
              <Icon type="delete" />
            </Popconfirm>
          </h5>
        )
      }
    })

    // Show in reverse chronological order
    if (commentItems) {
      commentItems.reverse()
    }
    const emptyText = (
      <div>
        {I18N.get('comments.noComments')}
        {this.props.currentUserId ? null : <a href="/login">{I18N.get('comments.signIn')}</a>}
        {I18N.get('comments.firstToPost')}
      </div>
    )
    return (
      <List
        size="large"
        itemLayout="horizontal"
        locale={{
          emptyText
        }}
        dataSource={commentItems}
        header={footer}
        renderItem={(item, ind) => (
          <List.Item key={ind}>
            {item.avatar}
            <div className="comment-content pull-left">
              <Row>
                <Col span={22}>
                  {item.headline && <h4>{item.headline}</h4>}
                  <h5>{enrichComment(item.comment)}</h5>
                </Col>
                <Col span={2}>
                  {item.delete}
                </Col>
              </Row>
              {this.renderTranslationBtn(item.comment)}
              <hr />
              {item.description}
            </div>
          </List.Item>
        )}
      />
    )
  }

  renderTranslationBtn(text) {
    return (
      <div style={{ marginBottom: 20 }}>
        <Translation text={text} />
      </div>
    )
  }

  handleSubmit(e) {
    e.preventDefault()
    this.props.form.validateFields((err, values) => {
      if (!err) {
        const {comment} = values
        const commentPlainText = _.isFunction(comment.getPlainText)
          ? comment.getPlainText()
          : comment

        if (_.isEmpty(commentPlainText)) {
          this.props.form.setFields({
            comment: {
              errors: [new Error('Please input comment')],
            },
          })

          return
        }

        if (commentPlainText.length > MAX_LENGTH_COMMENT) {
          this.props.form.setFields({
            comment: {
              value: comment,
              errors: [new Error('Comment is too long')],
            },
          })

          return
        }

        this.props.postComment(this.props.type,
          this.props.reduxType,
          this.props.detailReducer,
          this.props.returnUrl,
          this.getModelId(),
          commentPlainText,
          values.headline)
          .then(() => {
            this.props.form.resetFields()
          })
      }
    })
  }

  handleDelete = comment => {
    this.props
      .removeComment(
        this.props.type,
        this.props.reduxType,
        this.props.detailReducer,
        this.getModelId(),
        {
          commentId: comment._id
        }
      )
  }

  showUserProfile(username) {
    const user = _.find(this.props.all_users, { username })
    if (user) {
      this.props.history.push(`/member/${user._id}`)
    }
  }
}

export default Form.create()(C)
