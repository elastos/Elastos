import React from 'react'
import { Modal, Col, Button } from 'antd'
import BaseComponent from '@/model/BaseComponent'
import CVoteForm from '@/module/form/CVoteForm/Container'
import I18N from '@/I18N'

import { Container } from './style'
import { StyledButton } from '../../suggestion/detail/style'

export default class extends BaseComponent {
  constructor(props) {
    super(props)
    this.state = {
      creating: false,
    }
  }

  ord_render() {
    const { className, btnStyle, btnText } = this.props
    const classNameLocal = `cr-btn cr-btn-primary ${className}`
    const createBtn = (
      <StyledButton onClick={this.switchCreateMode} className={classNameLocal} style={btnStyle}>
        {btnText || I18N.get('from.CVoteForm.button.add')}
      </StyledButton>
    )
    const form = this.renderForm()
    return (
      <span>
        {createBtn}
        {form}
      </span>
    )
  }

  renderForm() {
    const props = {
      ...this.props,
      onCreated: this.onCreated,
      onCancel: this.switchCreateMode,
      header: I18N.get('from.CVoteForm.button.add'),
    }
    return (
      <Modal
        className="project-detail-nobar"
        maskClosable={false}
        visible={this.state.creating}
        onOk={this.switchCreateMode}
        onCancel={this.switchCreateMode}
        footer={null}
        width="70%"
      >
        <Container>
          <CVoteForm {...props} />
        </Container>
      </Modal>
    )
  }

  switchCreateMode = () => {
    const { creating } = this.state
    this.setState({
      creating: !creating,
    })
  }

  onCreated = () => {
    this.switchCreateMode()
    this.props.onCreated()
  }
}
