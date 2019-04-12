import React from 'react'
import PropTypes from 'prop-types'
import I18N from '@/I18N'

import { Container, StyledPopover, CloseIcon, Title, StyledTextArea, Footer, Btn } from './style'

class Component extends React.Component {
  state = {
    reason: '',
  }

  onReasonChanged = (e) => {
    this.setState({ reason: e.target.value })
  }

  onSubmit = () => {
    const { onToggle, onSubmit } = this.props
    const { reason } = this.state
    onToggle()
    onSubmit({ reason })
    this.setState({ reason: '' })
  }

  render() {
    const { triggeredBy, onToggle, visible, title } = this.props
    const content = (
      <Container>
        <CloseIcon type="close" onClick={onToggle} />
        <Title>{title || I18N.get('council.voting.modal.voteReason')}</Title>
        <StyledTextArea onChange={this.onReasonChanged} />
        <Footer>
          <Btn type="default" onClick={onToggle}>
            {I18N.get('council.voting.modal.cancel')}
          </Btn>
          <Btn type="danger" onClick={this.onSubmit}>
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
}

Component.propTypes = propTypes


export default Component
