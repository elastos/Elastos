import React from 'react'
import Footer from '@/module/layout/Footer/Container'
import I18N from '@/I18N'
import './style.scss'
import { Spin } from 'antd'
import _ from 'lodash'
import StandardPage from '../StandardPage'

export default class extends StandardPage {

  constructor(props) {
    super(props)

    this.state = {
      loading: false,
    }
  }

  async componentDidMount() {
    this.setState({ loading: false })
  }

  componentWillUnmount() {

  }

  checkForLoading(followup) {
    return this.state.loading
      ? <Spin size="large"/>
      : _.isFunction(followup) && followup()
  }

  ord_states() {
    return {
      showDetailId: null,
      loading: false
    }
  }

  ord_renderContent () {
    return (
      <div className="p_training">
        <div className="ebp-header-divider" />
        <div className="p_admin_index ebp-wrap">
          <div className="d_box">
            <div className="p_admin_content">
              {this.buildVision()}
            </div>
          </div>
        </div>
        <Footer/>
      </div>
    )
  }

  buildVision() {
    return (
      <div className="evangelist">
        <div className="container">
          <div className="left-box-container">
            <img src="assets/images/training_evangelist_logo.png"/>
          </div>
          <div className="right-box-container">
            <img src="assets/images/training_green_slashed_box.png"/>
          </div>
          <div className="title">
            {I18N.get('vision.00')}
          </div>
          <div className="content">
            <p>{I18N.get('vision.01')}</p>
            <br/>
            <p>{I18N.get('vision.02')}</p>
            <br/>
            <p>{I18N.get('vision.03')}</p>
            <br/>
            <p>{I18N.get('vision.04')}</p>
            <br/>
            <p>{I18N.get('vision.05')}</p>
            <br/>
            <p>{I18N.get('vision.06')}</p>
            <br/>
            <p>{I18N.get('vision.07')}</p>
          </div>
        </div>
      </div>
    )
  }
}
