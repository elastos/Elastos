import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import { Button } from 'antd'
import I18N from '@/I18N'
import TeamEditForm from '@/module/form/TeamCreateForm/Container'
import TeamDetail from '@/module/team/detail/Container'

export default class extends BaseComponent {
  ord_states() {
    return {
      editing: false
    }
  }

  ord_render() {
    return (
      <div className="w100">
        {this.renderHeader()}
        {this.state.editing ? this.renderEditForm() : this.renderTeamDetail()}
      </div>
    )
  }

  renderEditForm() {
    return (
      <div className="form-wrapper">
        <TeamEditForm
          existingTeam={this.props.data}
          switchEditMode={this.switchEditMode.bind(this)}
        />
      </div>
    )
  }

  renderHeader() {
    const canEdit = this.props.canEdit || false
    return (
      <div className="l_banner">
        <div className="pull-left">{I18N.get('team.detail.title')}</div>
        <div className="pull-right right-align">
          {canEdit && (
            <Button onClick={this.switchEditMode.bind(this)}>
              {this.state.editing ? I18N.get('.cancel') : I18N.get('.edit')}
            </Button>
          )}
        </div>
        <div className="clearfix" />
      </div>
    )
  }

  renderTeamDetail() {
    return (
      <div className="form-wrapper">
        {this.props.data && <TeamDetail teamId={this.props.data._id} />}
      </div>
    )
  }

  switchEditMode() {
    this.setState({ editing: !this.state.editing })
  }
}
