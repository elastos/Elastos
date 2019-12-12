import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import { Button, List, Avatar, Select, Input, InputNumber, Form } from 'antd'
import I18N from '@/I18N'
import TeamCreateForm from '@/module/form/TeamCreateForm/Container'
import {
  USER_AVATAR_DEFAULT,
  TEAM_AVATAR_DEFAULT,
  TASK_CATEGORY
} from '@/constant'
import _ from 'lodash'
import './style.scss'

class C extends BaseComponent {
  ord_states() {
    return {
      mode: null, // solo, team, newteam
      confirmation: false
    }
  }

  async componentDidMount() {
    if (this.props.currentUserId) {
      this.props.getTeams({
        owner: this.props.currentUserId
      })
    }
  }

  componentWillUnmount() {}

  handleSubmit(e) {
    e.preventDefault()

    this.props.form.validateFields((err, values) => {
      if (!err) {
        if (
          this.state.mode === 'solo' ||
          (this.state.mode === 'team' && values.team === '$me')
        ) {
          this.props
            .applyToTask(
              this.props.task._id,
              this.props.currentUserId,
              null,
              values.applyMsg,
              null,
              null,
              values.bid
            )
            .then(() => {
              this.setState({ confirmation: true })
            })
        } else if (this.state.mode === 'team') {
          this.props
            .applyToTask(
              this.props.task._id,
              null,
              values.team,
              values.applyMsg,
              null,
              null,
              values.bid
            )
            .then(() => {
              this.setState({ confirmation: true })
            })
        } else if (this.state.mode === 'newteam') {
          const sanitized = _.omit(values, ['team', 'applyMsg'])
          sanitized.domain = this.props.task.domain
          sanitized.tags = sanitized.tags.join(',')

          this.props.createTeam(sanitized).then(team => {
            this.props
              .applyToTask(
                this.props.task._id,
                null,
                team._id,
                null,
                null,
                null,
                values.bid
              )
              .then(() => {
                this.setState({ confirmation: true })
              })
          })
        }
      }
    })
  }

  ord_render() {
    return (
      <div className="c_ApplicationStart">
        {this.state.confirmation ? (
          <div className="c_ApplicationConfirm">
            <div className="left-box-container">
              <img src="/assets/images/rm-connector4-gray.svg" />
            </div>
            <div className="right-box-container">
              <div className="box" />
              <div className="small-box" />
              <img src="/assets/images/oomph.png" />
            </div>
            <div className="confirm-picture halign-wrapper">
              <img src="/assets/images/AUTHTH.png" />
            </div>
            <h2 className="confirm-message halign-wrapper komu-a">
              {I18N.get('developer.cr100.application.success')}
            </h2>
            <div className="confirm-actions halign-wrapper">
              <Button onClick={() => this.props.finisher()}>
                {I18N.get('developer.cr100.application.view')}
              </Button>
            </div>
            <div className="lower-box-container">
              <img src="/assets/images/training_green_slashed_box.png" />
            </div>
          </div>
        ) : (
          <Form onSubmit={this.handleSubmit.bind(this)}>
            {this.getHeader()}
            {this.getModeSelector()}
            {this.getModePanel()}
            {this.getActions()}
            <div className="left-box-container upper-corner">
              <img src="/assets/images/training_green_slashed_box.png" />
            </div>
            <div className="lower-box-container">
              <div className="small-box" />
              <div className="box" />
              <img src="/assets/images/oomph.png" />
            </div>
            <div className="right-box-container">
              <img src="/assets/images/rm-connector4-green.svg" />
            </div>
          </Form>
        )}
      </div>
    )
  }

  getAvatarWithFallback(avatar) {
    return _.isEmpty(avatar) ? USER_AVATAR_DEFAULT : avatar
  }

  getApplyWithDropdown() {
    return (
      <Select
        className="team-selector"
        disabled={this.props.loading}
        // Fix select dropdowns in modals
        // https://github.com/vazco/uniforms/issues/228
        getPopupContainer={x => {
          while (x && x.tagName.toLowerCase() !== 'form') {
            x = x.parentElement
          }

          return x
        }}
      >
        <Select.Option value="$me">
          Apply as myself
          <Avatar
            size="small"
            src={this.getAvatarWithFallback(this.props.currentUserAvatar)}
            className="pull-right"
          />
        </Select.Option>
        {_.map(this.props.ownedTeams, team => (
          <Select.Option key={team._id} value={team._id}>
            Apply with {team.name}
            {!_.isEmpty(team.pictures) ? (
              <Avatar
                size="small"
                src={this.getAvatarWithFallback(team.pictures[0].thumbUrl)}
                className="pull-right"
              />
            ) : (
              <Avatar
                size="small"
                src={this.getAvatarWithFallback()}
                className="pull-right"
              />
            )}
          </Select.Option>
        ))}
      </Select>
    )
  }

  getHeader() {
    const title =
      this.props.task.category === TASK_CATEGORY.CR100
        ? `#${this.props.task.dAppId} - ${this.props.task.name}`
        : this.props.task.name

    return (
      <div className="full-width halign-wrapper start-header">
        <h3 className="start-project-title komu-a">{title}</h3>
        <div className="strike-text start-welcome">
          <div className="strike-line" />
          <p>{I18N.get('profile.detail.thankforinterest')}</p>
        </div>
        <br />
        <div className="strike-text start-welcome">
          <div className="strike-line" />
          <p>{I18N.get('profile.detail.selectoption')}</p>
        </div>
      </div>
    )
  }

  getModeSelector() {
    // TODO: Change place holder user images with team

    const entityLookup = {
      TASK: 'Task',
      PROJECT: 'Project',
      EVENT: 'Event'
    }

    const entity = entityLookup[this.props.task.type]
    const data = [
      {
        img: '/assets/images/user_blurred.png',
        description: `I would like to contribute to this ${entity}`,
        text: 'Solo',
        mode: 'solo'
      },
      {
        img: TEAM_AVATAR_DEFAULT,
        description: `I registered a team and would like to work on this ${entity}`,
        text: 'Existing Team',
        mode: 'team'
      },
      {
        img: TEAM_AVATAR_DEFAULT,
        description: `I would like to create a team and work on this ${entity}`,
        text: 'Create Team',
        mode: 'newteam'
      }
    ]

    return (
      <div className="start-selector">
        <List
          grid={{ gutter: 16, column: 3 }}
          dataSource={data}
          renderItem={item => (
            <List.Item className="selector-item">
              <div className={this.state.mode === item.mode ? 'selected' : ''}>
                <div
                  className="selector-image"
                  onClick={() => this.changeMode(item.mode)}
                >
                  <div className="selector-combo">
                    <img src={item.img} />
                    <span className="overlay-text">{item.text}</span>
                  </div>
                </div>
              </div>
              <div className="selector-description">{item.description}</div>
            </List.Item>
          )}
        />
      </div>
    )
  }

  changeMode(mode) {
    this.setState({ mode })
  }

  getActions() {
    return (
      <div className="full-width halign-wrapper valign-wrapper">
        <Button
          htmlType="submit"
          type="ebp"
          className="d_btn"
          loading={this.props.loading}
          disabled={!this.state.mode}
        >
          <span className="komu-a">{I18N.get('.apply')}</span>
        </Button>
      </div>
    )
  }

  getModePanel() {
    if (!this.state.mode) {
      return
    }

    const { getFieldDecorator } = this.props.form
    const compLookup = {
      bidding: (
        <div className="mode-panel">
          <div className="label komu-a">
            {I18N.get('profile.detail.complookup')}
          </div>
          <InputNumber className="input-field" />
        </div>
      ),
      solo: (
        <div className="mode-panel">
          <div className="label komu-a">{I18N.get('profile.detail.solo')}</div>
          <Input.TextArea rows={4} className="input-field" />
        </div>
      ),
      team: this.getApplyWithDropdown(),
      newteam: <TeamCreateForm embedded={true} form={this.props.form} />
    }

    const decoratorLookup = {
      bidding: getFieldDecorator('bid', {
        rules: [
          {
            required: true,
            message: I18N.get('profile.detail.form.bid.required')
          }
        ],
        initialValue: 0
      }),
      solo: getFieldDecorator('applyMsg', {
        rules: [],
        initialValue: ''
      }),
      team: getFieldDecorator('team', {
        rules: [],
        initialValue: '$me'
      }),
      newteam: _.identity
    }

    const formLookup = {
      solo: (
        <div>
          {decoratorLookup.solo(compLookup.solo)}
          {this.props.task.bidding &&
            decoratorLookup.bidding(compLookup.bidding)}
        </div>
      ),
      team: (
        <div className="mode-panel">
          <div className="label komu-a">Choose your team</div>
          {decoratorLookup.team(compLookup.team)}
          {decoratorLookup.solo(compLookup.solo)}
          {this.props.task.bidding &&
            decoratorLookup.bidding(compLookup.bidding)}
        </div>
      ),
      newteam: (
        <div>
          {compLookup.newteam}
          {this.props.task.bidding &&
            decoratorLookup.bidding(compLookup.bidding)}
        </div>
      )
    }

    const className = `full-width start-mode ${
      this.state.mode !== 'newteam' ? 'halign-wrapper' : ''
    }`
    return <div className={className}>{formLookup[this.state.mode]}</div>
  }
}

export default Form.create()(C)
