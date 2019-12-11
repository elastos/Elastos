import React from 'react'
import Footer from '@/module/layout/Footer/Container'
import './style.scss'

import { Row } from 'antd'
import StandardPage from '../../StandardPage'

export default class extends StandardPage {

  ord_renderContent () {
    return (
      <div className="p_Slack">
        <div className="ebp-header-divider" />

        <div className="ebp-page-title">
          <Row className="d_row d_rowGrey">
            <h3 className="page-header">
                            Join Discord & Telegram
            </h3>
          </Row>
        </div>
        <div className="ebp-page">
          <Row className="d_row">
            <h4>
                            After joining Cyber Republic please join our Discord channel at
              {' '}
              <a href="https://discord.gg/UG9j6kh">https://discord.gg/UG9j6kh</a>
              {' '}
and
                            Telegram at
              {' '}
              <a href="https://t.me/elastosgroup">https://t.me/elastosgroup</a>
            </h4>

            <p>
                            Please use Discord to collaborate and find team members in your community.
              <br/>
              <br/>
                            If you have any issues please email us at
              {' '}
              <a href="mailto:cyberrepublic@elastos.org">cyberrepublic@elastos.org</a>
            </p>
          </Row>
        </div>
        <Footer/>
      </div>
    )
  }
}
