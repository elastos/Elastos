import React, { Component } from 'react'
import styled from 'styled-components'
import I18N from '@/I18N'
import TeamInfoSection from './TeamInfoSection'
import Milestones from './Milestones'
import CodeMirrorEditor from '@/module/common/CodeMirrorEditor'

class ImplementationPlan extends Component {
  constructor(props) {
    super(props)
    const value = props.initialValue
    this.state = {
      // plan: props.initialValue ? props.initialValue : {}
      plan: (value && value.plan) || {},
      planIntro: (value && value.planIntro) || ''
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

  changeValueIntro = value => {
    const { onChange, callback } = this.props
    const { planIntro } = this.state
    this.setState({planIntro: value}, () => {
      onChange(this.state.planIntro)
      callback('planIntro')
    })
  }

  render() {
    const { plan, planIntro } = this.state
    const { callback, getFieldDecorator } = this.props
    return (
      <div>
        <Title>{I18N.get('suggestion.plan.milestones')}</Title>
        <Milestones onChange={this.changeValue} initialValue={plan.milestone} />
        <TeamInfoSection
          title={I18N.get('suggestion.plan.teamInfo')}
          onChange={this.changeValue}
          initialValue={plan.teamInfo}
        />
        <Section>
          <Label>{`${I18N.get('suggestion.plan.introduction')}`}</Label>
          {getFieldDecorator('planIntro')(
            <CodeMirrorEditor
              callback={callback}
              content={planIntro}
              activeKey='planIntro'
              name='planIntro'
            />
          )
          }
        </Section>
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

const Section = styled.div`
  margin-top: 24px;
  .ant-btn {
    margin-top: 16px;
    border: 1px solid #000000;
    color: #000000;
    &:hover {
      border: 1px solid #008d85;
      color: #008d85;
    }
  }
`

const Label = styled.div`
  font-size: 17px;
  line-height: 24px;
  color: #000000;
`