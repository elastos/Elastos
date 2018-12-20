import React from 'react';
import StandardPage from '../StandardPage';
import I18N from '@/I18N';
import Footer from '@/module/layout/Footer/Container'
import { Col, Row, Icon } from 'antd'

import './style.scss'

export default class extends StandardPage {

    ord_renderContent () {

        return (
            <div className="p_Directory">
                <div className="ebp-header-divider">

                </div>
                <div className="ebp-page">
                    <div className="ebp-page-title">

                    </div>

                    <Row>
                        <Col span={18}>
                            <div class="d_leadersList">
                                <h3>
                                    {I18N.get('0003')}
                                </h3>
                            </div>
                        </Col>
                        <Col span={6}>
                            <br/>
                            <div class="ebp-gray-box center">
                                <h4>

                                </h4>
                            </div>
                        </Col>
                    </Row>
                </div>
                <Footer />
            </div>
        )
    }
}
