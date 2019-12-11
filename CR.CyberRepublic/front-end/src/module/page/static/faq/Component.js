import React from 'react'
import StandardPage from '../../StandardPage'
import Footer from '@/module/layout/Footer/Container'
import './style.scss'

import { Col, Row } from 'antd'

export default class extends StandardPage {

  ord_renderContent () {

    return (
      <div className="p_FAQ">
        <div className="ebp-header-divider" />

        <div className="ebp-page-title">
          <Row className="d_row d_rowGrey">
            <h3 className="page-header">
                            FAQ
            </h3>
          </Row>
        </div>
        <div className="ebp-page">
          <Row className="d_row">
            <Col>
              <h4>
                                How do I create Developer Events/Tasks?
              </h4>

              <p>
                                Right now only admins can create these, if you have an idea or find a bug please use the submission feature to get in contact with us
              </p>
            </Col>
            <Col>
              <h4>
                                How do I create tasks/events or become an organizer?
              </h4>

              <p>
                                Only organizers can create tasks/events, to become an organizer you must apply using the form found under each country in the
                {' '}
                <a href="/community">Community page</a>
.
              </p>
              <p>
                                We do accept more than one organizer per country/community, however your chances are lower if there is already one.
              </p>
            </Col>
            <Col>
              <h4>
                                What's the difference between the budget and reward?
              </h4>

              <p>
                                The budget is paid upfront and must only go towards expenses/costs, receipts are required and any unused portion must be returned.
              </p>
              <p>
                                Upon completion the reward is then paid, for larger tasks we will consider a partial payment of the reward upfront, please contact us directly.
              </p>
            </Col>
            <Col>
              <h4>
                                What's the difference between a task's max applicants and max accepted?
              </h4>

              <p>
                                An organizer may set a maximum number of applicants, but not all applications need to be accepted.
              </p>
              <p>
                                They also do not have to wait until the number of applications accepted reaches the "max accepted" number.
                <br/>
                                At any time an organizer can
                {' '}
                <span style={{fontWeight: 600}}>force start</span>
                {' '}
the task/event.
              </p>
            </Col>
          </Row>
        </div>
        <Footer/>
      </div>
    )
  }
}
