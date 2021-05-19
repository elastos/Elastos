import React from 'react'
import Footer from '@/module/layout/Footer/Container'
import './style.scss'
import { Col, Row, Icon, Form, Input, Button, Dropdown } from 'antd'

import StandardPage from '../StandardPage'

const FormItem = Form.Item

export default class extends StandardPage {

  ord_renderContent () {

    return (
      <div className="p_Teams">
        <div className="ebp-header-divider" />
        <div className="ebp-page">
          <div className="ebp-page-title">
            <h1>
                            Teams
            </h1>
          </div>

                    TODO: your teams + teams recruiting + create a team / invite members - low priority for first release
        </div>
        <Footer />
      </div>
    )
  }
}
