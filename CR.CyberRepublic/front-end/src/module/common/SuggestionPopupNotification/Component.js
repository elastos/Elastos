import React from 'react'
import BaseComponent from '@/model/BaseComponent'

import { Modal, Row, Col, Form } from 'antd'
import I18N from '@/I18N'
import './style.scss'

class C extends BaseComponent {

  ord_states() {
    return {
      visible: false
    }
  }

  async componentDidMount() {
    await this.props.getCurrentUser()
    this.updateModalVisibility()
  }

  updateModalVisibility() {
    const popupUpdate = localStorage.getItem('popup-update')

    if (popupUpdate === 'force') {
      this.setState({
        visible: true
      })
      return
    }

    const value = popupUpdate === 'true'
    if (!value) {
      if (!this.props.is_login || !this.props.popup_update) {
        // User didn't see it yet
        this.setState({
          visible: true
        })
      }
    }
  }

  /**
     * TODO: if we reuse this we need an HTML templating solution
     */
  ord_render() {

    const lang = localStorage.getItem('lang') || 'en'

    return (
      <Modal
        visible={this.state.visible}
        onCancel={this.handleCancel.bind(this)}
        footer={null}
        width="50%"
        className="c_Popup_Notif"
      >
        <Row>
          <Col>
            <div className="right-col">
              <h1 className="title">{I18N.get('popup.suggestion.title')}</h1>
              <p>{I18N.get('popup.suggestion.member')}</p>
              <p style={{ forntSize: 30 + 'px',textIndent:2+'em'}}>{I18N.get('popup.suggestion.content')}</p>
              <br/>
              <p>{I18N.get('popup.suggestion.team')}</p>
            </div>
          </Col>
        </Row>
      </Modal>
    )
  }

  handleCancel() {
    if (this.props.is_login) {
      this.props.updateUserPopupUpdate(this.props.currentUserId)
    }

    localStorage.setItem('popup-update', 'true')

    this.setState({
      visible: false
    })
  }
}

export default Form.create()(C)
