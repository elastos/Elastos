import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import _ from 'lodash'
import {
  Row,
  Col,
  Modal,
  Spin,
} from 'antd'
import I18N from '@/I18N'
import { TranslateButton, ModalBody, TranslationText, Container } from './style'

export default class extends BaseComponent {
  constructor(props) {
    super(props)

    this.state = {
      isTranslateModalOpen: false,
      translation: '',
    }
  }

  translate = async () => {
    const { gTranslate, text } = this.props
    this.setState({ isTranslateModalOpen: true, translation: '' })
    const res = await gTranslate({ text })
    this.setState({ translation: res.translation })
  }

  renderTranslationModal() {
    const { isTranslateModalOpen, translation } = this.state
    const translationNode = translation ? <TranslationText dangerouslySetInnerHTML={{ __html: translation }} /> : <Spin />

    return (
      <Modal
        className="translate-modal-container"
        visible={isTranslateModalOpen}
        onOk={this.showTranslate}
        onCancel={this.showTranslate}
        footer={null}
        width="70%"
        closable
        centered
        style={{ minWidth: 400 }}
      >
        <ModalBody>
          {translationNode}
          <div>{I18N.get('suggestion.translatedByGoogle')}</div>
        </ModalBody>
      </Modal>
    )
  }

  showTranslate = () => {
    const { isTranslateModalOpen } = this.state
    this.setState({
      isTranslateModalOpen: !isTranslateModalOpen,
    })
  }

  ord_render() {
    const btn = (
      <TranslateButton onClick={this.translate}>{I18N.get('suggestion.translate')}</TranslateButton>
    )
    const translationModal = this.renderTranslationModal()
    return (
      <Container>
        {btn}
        {translationModal}
      </Container>
    )
  }
}
