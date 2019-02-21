import React from 'react'
import PropTypes from 'prop-types'
import { Icon, TextArea, Button, Popover } from 'antd'
import I18N from '@/I18N'

import { Container, CloseIcon, Title, StyledTextArea, Footer, Btn } from './style'

class Component extends React.Component {
  state = {
    reason: '',
  }

  onReasonChanged = (e) => {
    this.setState({ reason: e.target.value })
  }

  render() {
    const { triggeredBy, onToggle, onSubmit, visible } = this.props
    const { reason } = this.state
    const content = (
      <Container>
        <CloseIcon type="close" onClick={onToggle} />
        <Title>{I18N.get('council.voting.modal.voteNo')}</Title>
        <StyledTextArea onChange={this.onReasonChanged} />
        <Footer>
          <Btn type="default" onClick={onToggle}>
            {I18N.get('council.voting.modal.cancel')}
          </Btn>
          <Btn type="danger" onClick={() => onSubmit({ reason })}>
            {I18N.get('council.voting.modal.confirm')}
          </Btn>
        </Footer>
      </Container>
    )
    return (
      <Popover content={content} trigger="click" visible={visible}>
        {triggeredBy}
      </Popover>
    )
  }
}

const propTypes = {
  triggeredBy: PropTypes.node.isRequired,
  onToggle: PropTypes.func.isRequired,
  onSubmit: PropTypes.func.isRequired,
  visible: PropTypes.bool.isRequired,
}

Component.propTypes = propTypes


export default Component
