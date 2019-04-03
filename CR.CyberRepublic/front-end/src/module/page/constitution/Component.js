import React from 'react'
import StandardPage from '../StandardPage'
import Footer from '@/module/layout/Footer/Container'
import {TEAM_TYPE, TEAM_SUBCATEGORY} from '@/constant'
import I18N from '@/I18N'
import './style.scss'
import { Col, Row, Modal } from 'antd'
import _ from 'lodash'
import Navigator from '@/module/page/shared/ConstitutionNavigator/Container'
import MediaQuery from 'react-responsive'

export default class extends StandardPage {
    ord_props() {
        return {
        }
    }

    componentWillUnmount() {
    }

    ord_renderContent () {
        const id = this.props.id
        const text = I18N.get(`council.article.${id}`)

        return (
            <div style={{marginTop: 80}}>
                <div className="ebp-header-divider">

                </div>
                <div className="p_admin_index ebp-wrap">
                    <div className="d_box">
                        <div className="p_Profile p_admin_content">
                            <MediaQuery maxWidth={720}>
                                <Row>
                                    <Col className="wrap-box-navigator">
                                        <Navigator selectedItem={`constitution/${id}`} />
                                    </Col>
                                </Row>
                            </MediaQuery>
                            <Row>
                                <MediaQuery minWidth={720}>
                                    <Col span={4} className="admin-left-column wrap-box-navigator">
                                        <Navigator selectedItem={`constitution/${id}`} />
                                    </Col>
                                </MediaQuery>
                                <Col xs={{span: 24}} md={{span: 20}} className="c_ConstitutionContainer wrap-box-user">
                                    <div className="content">
                                        <h1 className="title">{I18N.get(`counstitution.title${id}`)}</h1>
                                        <span dangerouslySetInnerHTML={{__html : text}}></span>
                                    </div>
                                </Col>
                            </Row>
                            <Row>
                                <Col>
                                    <br/>
                                </Col>
                            </Row>
                        </div>
                    </div>
                </div>
                <Footer/>
            </div>
        )
    }
}
