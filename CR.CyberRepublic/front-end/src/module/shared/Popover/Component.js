import React from 'react'
import PropTypes from 'prop-types'
import I18N from '@/I18N'
import _ from 'lodash'

import {
  Container,
  StyledPopover,
  CloseIcon,
  Title,
  StyledTextArea,
  Footer,
  Btn,
  DataExplain
} from './style'

class Component extends React.Component {
  state = {
    reason: '',
    emptyStatus: false
  }

  onReasonChanged = e => {
    const { required } = this.props
    if (required) {
      if (_.isEmpty(e.target.value)) {
        this.setState({ emptyStatus: true })
      } else {
        this.setState({ emptyStatus: false })
      }
    }
    this.setState({ reason: e.target.value })
  }

  onClick = () => {
    this.setState({ emptyStatus: false, reason: '' })
    this.props.onToggle()
  }

  onSubmit = () => {
    const { onToggle, onSubmit, required } = this.props
    const { reason } = this.state
    if (required && _.isEmpty(reason)) {
      this.setState({ emptyStatus: true })
      return
    }
    onToggle()
    onSubmit({ reason })
    this.setState({ reason: '' })
  }

  render() {
    const { triggeredBy, visible, title, btnType, requiredMsg } = this.props
    const { emptyStatus } = this.state
    const dataExplain = emptyStatus && <DataExplain>{requiredMsg}</DataExplain>
    const content = (
      <Container>
        <CloseIcon type="close" onClick={this.onClick} />
        <Title>{title || I18N.get('council.voting.modal.voteReason')}</Title>
        <StyledTextArea
          value={this.state.reason}
          className={emptyStatus && 'has-error'}
          onChange={this.onReasonChanged}
        />
        {dataExplain}
        <Footer>
          <Btn
            type="default"
            onClick={this.onClick}
            style={{ borderRadius: 0 }}
          >
            {I18N.get('council.voting.modal.cancel')}
          </Btn>
          <Btn
            type={btnType || 'default'}
            colored="true"
            onClick={this.onSubmit}
            style={{ borderRadius: 0 }}
          >
            {I18N.get('council.voting.modal.confirm')}
          </Btn>
        </Footer>
      </Container>
    )
    return (
      <StyledPopover content={content} trigger="click" visible={visible}>
        {triggeredBy}
      </StyledPopover>
    )
  }
}

const propTypes = {
  title: PropTypes.string,
  triggeredBy: PropTypes.node.isRequired,
  onToggle: PropTypes.func.isRequired,
  onSubmit: PropTypes.func.isRequired,
  visible: PropTypes.bool.isRequired,
  required: PropTypes.bool,
  requiredMsg: PropTypes.string
}

Component.propTypes = propTypes

export default Component
