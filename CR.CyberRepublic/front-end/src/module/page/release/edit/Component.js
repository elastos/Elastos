import React from 'react'
import { Modal, Icon, Button } from 'antd'
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
    const btn = (
      <StyledButton onClick={this.showForm} className="cr-btn cr-btn-primary">
        <Icon type="edit" style={{ color: 'white' }} />
        {I18N.get('release.btn.edit')}
      </StyledButton>
    )
    const form = this.renderForm()
    return (
      <Container>
        {btn}
        {form}
      </Container>
    )
  }

  onFormSubmit = async param => {
    try {
      await this.props.update(param)
      this.showForm()
      this.refetch()
    } catch (error) {
      // console.log(error)
    }
  }

  renderForm = () => {
    const { detail } = this.props

    const props = {
      onFormCancel: this.showForm,
      onFormSubmit: this.onFormSubmit,
      header: I18N.get('release.form.edit'),
      data: detail
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

export const Container = styled.div``

const StyledButton = styled(Button)`
  display: flex;
  align-items: center;
  justify-content: center;
  > span {
    font-size: 12px !important;
  }
`
