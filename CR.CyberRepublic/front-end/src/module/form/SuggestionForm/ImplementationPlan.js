import React, { Component } from 'react'
import styled from 'styled-components'
import I18N from '@/I18N'
import TeamInfoSection from './TeamInfoSection'
import Milestones from './Milestones'

class ImplementationPlan extends Component {
  constructor(props) {
    super(props)
    this.state = {
      plan: props.initialValue ? props.initialValue : {}
    }
  }

  changeValue = value => {
    const { onChange, callback } = this.props
    const { plan } = this.state
    this.setState({ plan: { ...plan, ...value } }, () => {
      onChange(this.state.plan)
      callback('plan')
    })
  }

  render() {
    const { plan } = this.state
    return (
      <div>
        <Title>{I18N.get('suggestion.plan.milestones')}</Title>
        <Milestones onChange={this.changeValue} initialValue={plan.milestone} />
        <TeamInfoSection
          title={I18N.get('suggestion.plan.teamInfo')}
          onChange={this.changeValue}
          initialValue={plan.teamInfo}
        />
      </div>
    )
  }
}

export default ImplementationPlan

const Title = styled.div`
  font-size: 17px;
  line-height: 24px;
  color: #000000;
  margin-bottom: 20px;
`
