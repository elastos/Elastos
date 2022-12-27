import {TASK_STATUS, TASK_CATEGORY, TASK_TYPE} from '@/constant'
import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import TeamCreateForm from '@/module/form/TeamCreateForm/Container'
import { Col, Row, Popconfirm, Divider, Button, Spin, Icon } from 'antd'
import TeamPublicDetail from './detail/Container'
import './style.scss'
import moment from 'moment/moment'
import _ from 'lodash'
import I18N from '@/I18N'

/**
 * This has 3 views
 *
 * 1. Public
 * 2. Admin
 * 3. Edit
 *
 */
export default class extends BaseComponent {

  componentDidMount() {
    const teamId = this.props.match.params.teamId
    teamId && this.props.getTeamDetail(teamId)
  }

  renderMain() {
    const isTeamOwner = this.props.current_user_id ===
            (this.props.team.owner && this.props.team.owner._id)

    return (
      <div className="c_CircleDetail">
        {(isTeamOwner || this.props.is_admin) && this.renderHeader()}
        {this.state.editing ? this.renderEditForm() : this.renderDetail()}
      </div>
    )
  }

  renderHeader() {
    return (
      <div className="l_banner">
        <div className="pull-left">
          {I18N.get('team.detail.team_active')}
.
        </div>
        <div className="pull-right right-align">
          <Button onClick={this.switchEditMode.bind(this)}>
            {this.state.editing ? I18N.get('.cancel') : I18N.get('.edit')}
          </Button>
          <Popconfirm title={I18N.get('.areYouSure')} onConfirm={this.deleteTeam.bind(this)}>
            <Button loading={this.props.loading}>
              {I18N.get('.delete')}
            </Button>
          </Popconfirm>
        </div>
        <div className="clearfix"/>
      </div>
    )
  }

  renderEditForm() {
    return (
      <div className="form-wrapper">
        <TeamCreateForm existingTeam={this.props.team} page={this.props.page}
        switchEditMode={this.switchEditMode.bind(this)}/>
      </div>
    )
  }

  renderDetail() {
    return <TeamPublicDetail teamId={this.props.team._id} page={this.props.page}/>
  }

  ord_render () {
    return (_.isEmpty(this.props.team) || this.props.team.loading ? (
      <div className="center"><Spin size="large" /></div>) : (
      this.renderMain())
    )
  }

  switchEditMode() {
    this.setState({editing: !this.state.editing})
  }

  deleteTeam() {
    this.props.deleteTeam(this.props.team._id).then(() => {
      this.props.history.goBack()
    })
  }
}
