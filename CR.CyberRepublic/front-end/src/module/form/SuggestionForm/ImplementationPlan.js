import React, { Component } from 'react'
import styled from 'styled-components'
import TeamInfoSection from './TeamInfoSection'
import I18N from '@/I18N'

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
        <Title>{I18N.get('suggestion.plan.teamInfo')}</Title>
        <TeamInfoSection
          onChange={this.changeValue}
          initialValue={plan.teamInfo}
        />
      </div>
    )
  }
}

export default ImplementationPlan

const Title = styled.div`
  font-weight: 500;
  font-size: 16px;
  margin-bottom: 16px;
`
