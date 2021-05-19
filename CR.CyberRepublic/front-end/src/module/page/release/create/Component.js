import React from 'react'
import { Modal, Button } from 'antd'
import BaseComponent from '@/model/BaseComponent'
import ReleaseForm from '@/module/form/ReleaseForm/Container'
import I18N from '@/I18N'
import styled from 'styled-components'

export default class extends BaseComponent {
  constructor(props) {
    super(props)
    this.state = {
      showForm: false
    }
  }

  ord_render() {
    const { className, btnStyle, btnText } = this.props
    const classNameLocal = `cr-btn cr-btn-primary ${className}`
    const createBtn = (
      <StyledButton
        onClick={this.showForm}
        className={classNameLocal}
        style={btnStyle}
      >
        {btnText || I18N.get('release.btn.add')}
      </StyledButton>
    )
    const form = this.renderForm()
    return (
      <Container>
        {createBtn}
        {form}
      </Container>
    )
  }

  onFormSubmit = async param => {
    try {
      await this.props.create(param)
      this.showForm()
      this.refetch()
    } catch (error) {
      // console.log(error)
    }
  }

  renderForm = () => {
    const props = {
      onFormCancel: this.showForm,
      onFormSubmit: this.onFormSubmit,
      header: I18N.get('release.form.add')
    }

    return (
      <Modal
        className="project-detail-nobar"
        maskClosable={false}
        visible={this.state.showForm}
        onOk={this.showForm}
        onCancel={this.showForm}
        footer={null}
        width="70%"
      >
        {this.state.showForm && <ReleaseForm {...props} />}
      </Modal>
    )
  }

  showForm = () => {
    const { showForm } = this.state
    this.setState({
      showForm: !showForm
    })
  }

  refetch = () => {
    this.props.getList()
  }
}

export const Container = styled.span`
  text-align: center;
`
const StyledButton = styled(Button)`
  display: flex;
  align-items: center;
  justify-content: center;
  > span {
    font-size: 12px !important;
  }
`
