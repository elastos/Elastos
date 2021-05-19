import React from 'react'
import BaseComponent from '@/model/BaseComponent'

import {Modal, Row, Col, Form} from 'antd'
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
        width="76%"
        className="c_Popup_Notif"
      >
        <Row>
          <Col>
            <div className="right-col">
              <h1 className="komu-a title">{I18N.get('popup.changes.title')}</h1>
              <ol className="synthese changes-list">
                <li>{I18N.get('popup.changes.2018-12-26.1')}</li>
                <li>{I18N.get('popup.changes.2018-12-26.2')}</li>
                <li>
                  {I18N.get('popup.changes.2018-12-26.3')}
                  <br/>
                  <br/>
                  <a href={`${process.env.FORUM_URL}/login`} target="_blank">https://forum.cyberrepublic.org</a>
                  <br/>
                  <br/>
                  {I18N.get('popup.changes.2018-12-26.4')}
                  <div className="center">
                    {lang === 'en' ? <img src="/assets/images/popup-changes-2018-12-26-en.png"/> : <img src="/assets/images/popup-changes-2018-12-26-zh.png"/>}
                  </div>
                </li>
              </ol>

              {I18N.get('popup.changes.2018-12-26.5')}
              {' '}
              <a target="_blank" href={I18N.get('popup.changes.2018-12-26.blog_link')}>{I18N.get('popup.changes.2018-12-26.blog_link')}</a>
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
