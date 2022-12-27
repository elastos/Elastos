import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import TeamDetail from '@/module/team/Container'
import { Form } from 'antd'
import './style.scss'

class C extends BaseComponent {
  ord_states() {
  }

  async componentDidMount() {
    const teamId = this.props.match.params.teamId
    await this.props.getTeamDetail(teamId)
  }

  componentWillUnmount() {
    this.props.resetTeamDetail()
  }

  ord_render () {
    return (
      <TeamDetail team={this.props.detail}/>
    )
  }
}

export default Form.create()(C)
