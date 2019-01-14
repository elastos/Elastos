import React from 'react';
import StandardPage from '../StandardPage';
import Footer from '@/module/layout/Footer/Container'
import I18N from '@/I18N'
import _ from 'lodash'
import './style.scss'

import { Row , Col } from 'antd'

export default class extends StandardPage {

    constructor(props) {
        super(props)

        this.state = {
            selectedBox: 0,
        }
    }

    componentDidMount() {
    }

    componentWillUnmount() {
    }

    switchToBox(box) {
        this.setState({
            selectedBox: box
        })
    }

    ord_renderContent() {
        const selectedBox = this.state.selectedBox;
        const title = I18N.get('home.box_' + (selectedBox + 1).toString() + '.title')
        const description1 = I18N.get('home.explanation_' + (selectedBox + 1).toString() + '.part_1')
        const description2 = I18N.get('home.explanation_' + (selectedBox + 1).toString() + '.part_2')

        return (
            <div className="c_Home">
                <div className="decoration-1">
                    <img className="upper-left" src="/assets/images/training_mini_connector.png"/>
                </div>
                <div className="decoration-slashed-left">
                    <img src="/assets/images/training_green_slashed_box.png"/>
                </div>
                <div className="decoration-square">
                    <div className="big-square"></div>
                    <div className="small-square"></div>
                </div>
                <Row className="top-section" type="flex" justify="center" gutter={32}>
                    <Col className={'box-wrap ' + (selectedBox === 0 ? 'selected-box' : '') } xs={24} sm={24} md={24} lg={8} onClick={this.switchToBox.bind(this, 0)}>
                        <div className={'box box-hover'}>
                            <h3>{I18N.get('home.box_1.title')}</h3>
                            <p className={"synthese" + (selectedBox === 0 ? ' selected-text' : 0)}>{I18N.get('home.box_1.description')}</p>
                        </div>
                        <div className="container">
                            <div className={"cuttoff-box" + (selectedBox === 0 ? '' : ' cutoff-box-hidden')}></div>
                        </div>
                        <img className={"arrow" + (selectedBox === 0 ? '' : ' arrow-hidden')} src="/assets/images/emp35/down_arrow.png"/>
                    </Col>
                    <Col className={'box-wrap ' + (selectedBox === 1 ? 'selected-box' : '') } xs={24} sm={24} md={24} lg={8} onClick={this.switchToBox.bind(this, 1)}>
                        <div className={'box box-hover'}>
                            <h3>{I18N.get('home.box_2.title')}</h3>
                            <p className={"synthese" + (selectedBox === 1 ? ' selected-text' : '')}>{I18N.get('home.box_2.description')}</p>
                        </div>
                        <div className="container">
                            <div className={"cuttoff-box" + (selectedBox === 1 ? '' : ' cutoff-box-hidden')}></div>
                        </div>
                        <img className={"arrow" + (selectedBox === 1 ? '' : ' arrow-hidden')} src="/assets/images/emp35/down_arrow.png"/>
                    </Col>
                    <Col className={'box-wrap ' + (selectedBox === 2 ? 'selected-box' : '') } xs={24} sm={24} md={24} lg={8} onClick={this.switchToBox.bind(this, 2)}>
                        <div className={'box box-hover'}>
                            <h3>{I18N.get('home.box_3.title')}</h3>
                            <p className={"synthese" + (selectedBox === 2 ? ' selected-text' : '')}>{I18N.get('home.box_3.description')}</p>
                        </div>
                        <div className="container">
                            <div className={"cuttoff-box" + (selectedBox === 2 ? '' : ' cutoff-box-hidden')}></div>
                        </div>
                        <img className={"arrow" + (selectedBox === 2 ? '' : ' arrow-hidden')} src="/assets/images/emp35/down_arrow.png"/>
                    </Col>
                </Row>
                <div className="mid-section">
                    <div className="decoration-2">
                        <img className="upper-left" src="/assets/images/training_green_slashed_box.png"/>
                    </div>
                    <div className="inner-box">
                        <div className="decoration-3">
                            <img className="upper-left" src="/assets/images/training_green_slashed_box.png"/>
                        </div>
                        <h3>{title}</h3>
                        <p className="synthese">{description1}</p>
                        <p className="synthese">{description2}</p>
                    </div>
                    <div className="rectangle-1"></div>
                    <div className="rectangle-2"></div>
                    <div className="rectangle-3"></div>
                </div>
                <div className="stay-updated">
                    <div className="form-wrap footer-email">
                        <p>{I18N.get('landing.footer.note')}</p>
                        <form id="footer-form" className="signup-form" name="mailing-list" action="https://cyberrepublic.us19.list-manage.com/subscribe/post-json?u=acb5b0ce41bfe293d881da424&id=272f303492"
                            method="get">
                            <div className="email-wrap">
                                <input type="email" name="EMAIL" data-type="req" placeholder={I18N.get('landing.footer.email')}/>
                                <button type="submit" className="arrow-submit">
                                    <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 17 34">
                                        <polygon points="0 0 0 33.487 16.744 16.744 0 0" style={{fill: '#1de9b6'}}/>
                                        <polygon points="0 24.579 7.835 16.744 0 8.91 0 24.579" className="small-tri"/>
                                    </svg>
                                </button>
                            </div>
                        </form>
                    </div>
                </div>
                <Footer/>
            </div>
        );
    }
}
