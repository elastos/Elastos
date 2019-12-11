import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import { Modal, Spin } from 'antd'
import I18N from '@/I18N'
import sanitizeHtml from '@/util/html'
import { TranslateButton, ModalBody, TranslationText, Container, Footer, LangText } from './style'

export default class extends BaseComponent {
  constructor(props) {
    super(props)

    this.state = {
      isTranslateModalOpen: false,
      translation: '',
      selectedLang: I18N.getLang(),
    }
  }

  translate = async (lang) => {
    const { gTranslate, text } = this.props
    this.setState({
      isTranslateModalOpen: true,
      translation: '',
      selectedLang: lang,
    })
    const res = await gTranslate({ text, target: lang })
    this.setState({ translation: res.translation })
  }

  renderTranslationModal() {
    const { isTranslateModalOpen, translation } = this.state
    const translationNode = translation ? <TranslationText dangerouslySetInnerHTML={{ __html: sanitizeHtml(translation) }} /> : <Spin />

    return (
      <Modal
        className="translate-modal-container"
        visible={isTranslateModalOpen}
        onOk={this.showTranslate}
        onCancel={this.showTranslate}
        footer={null}
        width="70%"
        closable={true}
        centered={true}
        style={{ minWidth: 400 }}
      >
        <ModalBody>
          {translationNode}
          <Footer>{I18N.get('suggestion.translatedByGoogle')}</Footer>
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
    const { selectedLang } = this.state
    const btn = (
      <TranslateButton>
        <span>{I18N.get('suggestion.translate')}</span>
        <LangText onClick={() => this.translate('en')} type="en" selectedLang={selectedLang}>{I18N.get('suggestion.translate.en')}</LangText>
        <span> | </span>
        <LangText onClick={() => this.translate('zh')} type="zh" selectedLang={selectedLang}>{I18N.get('suggestion.translate.zh')}</LangText>
      </TranslateButton>
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
