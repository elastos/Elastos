import React from 'react'
import StandardPage from '../StandardPage'
import _ from 'lodash'
import I18N from '@/I18N'
import './style.scss'
import { MAX_WIDTH_MOBILE, MIN_WIDTH_PC } from '@/config/constant'
import Video from '@/module/shared/video/Container'
import { Modal, Menu, Dropdown, Col, Row, List, Button, Select } from 'antd'
import Footer from '@/module/layout/Footer/Container'
import moment from 'moment/moment'
import MediaQuery from 'react-responsive'
import Flag from 'react-flags'
import Data from '@/config/data'
import {USER_LANGUAGE} from '@/constant'

export default class extends StandardPage {
    constructor(props) {
        super(props)

        this.state = {
            showVideo: false,
        }
    }

    changeLanguage(e) {
        const key = e.key
        if (_.includes([
            'en',
            'zh'
        ], key)) {
            this.props.changeLanguage(e.key);
        }
    }

    buildLanguageDropdown() {
        const menu = (
            <Menu onClick={this.changeLanguage.bind(this)} className="language-menu">
                <Menu.Item key="en">
                    <div>
                        <Flag name="US" format="png"
                            basePath="/assets/images/flags"
                            pngSize={24} shiny={true} alt="English" />
                        <span className="language-us">English</span>
                    </div>
                </Menu.Item>
                <Menu.Item key="zh">
                    <div>
                        <Flag name="CN" format="png"
                            basePath="/assets/images/flags"
                            pngSize={24} shiny={true} alt="English" />
                        <span className="language-cn">简体中文</span>
                    </div>
                </Menu.Item>
            </Menu>
        )

        return (
            <Dropdown overlay={menu} >
                <a className="ant-dropdown-link">
                    <Flag name={Data.mappingLanguageKeyToName[this.props.lang]} format="png"
                        basePath="/assets/images/flags"
                        pngSize={24} shiny={true} alt="English" />
                </a>
            </Dropdown>
        )
    }

    handleCancelVideo() {
        this.setState({
            showVideo: false
        })
    }

    renderVideoModal() {
        return (
            <Modal
                className="video-popup-modal"
                visible={!!this.state.showVideo}
                onCancel={this.handleCancelVideo.bind(this)}
                footer={null}
                width="76%"
                style={{ top: 35 }}>
                { this.state.showVideo ? < Video /> : '' }
            </Modal>
        )
    }

    playVideo() {
        this.setState({
            showVideo: true
        })
    }

    ord_renderContent () {
        let linkToBlog = 'https://blog.cyberrepublic.org'

        if (I18N.getLang() === USER_LANGUAGE.zh) {
            linkToBlog += `/${USER_LANGUAGE.zh}`
        }

        return <div className="p_landingBg">
            {this.renderVideoModal()}
            <div id="loader">
                <div className="load-clip">
                    <div className="logo-text part"><img src="assets/images/logo-text.svg"/></div>
                    <div className="logo-mark part"><img src="assets/images/logo-mark.svg"/></div>
                </div>
            </div>

            <section id="hero" className="hasAnim">
                <div className="bg-wrap">
                    <div className="background">
                        <div className="layer base"><img src="assets/images/hero-base.svg"/></div>

                        <h1>{I18N.get('landing.header')}</h1>

                        <div className="cardfly-wrap part-wrap" data-num="1">
                            <div className="cardfly part" data-num="1"><img src="assets/images/parts/cardfly1.svg"/></div>
                            <div className="cardfly part" data-num="2"><img src="assets/images/parts/cardfly2.svg"/></div>
                            <div className="cardfly part" data-num="3"><img src="assets/images/parts/cardfly3.svg"/></div>
                        </div>
                        <div className="cardfly-wrap part-wrap" data-num="2">
                            <div className="cardfly part" data-num="1"><img src="assets/images/parts/cardfly2.svg"/></div>
                            <div className="cardfly part" data-num="2"><img src="assets/images/parts/cardfly3.svg"/></div>
                            <div className="cardfly part" data-num="3"><img src="assets/images/parts/cardfly1.svg"/></div>
                        </div>
                        <div className="cardfly-wrap part-wrap" data-num="3">
                            <div className="cardfly part" data-num="1"><img src="assets/images/parts/cardfly3.svg"/></div>
                            <div className="cardfly part" data-num="2"><img src="assets/images/parts/cardfly1.svg"/></div>
                            <div className="cardfly part" data-num="3"><img src="assets/images/parts/cardfly2.svg"/></div>
                        </div>
                        <div className="cardfly-wrap part-wrap" data-num="4">
                            <div className="cardfly part" data-num="1"><img src="assets/images/parts/cardfly1.svg"/></div>
                            <div className="cardfly part" data-num="2"><img src="assets/images/parts/cardfly3.svg"/></div>
                            <div className="cardfly part" data-num="3"><img src="assets/images/parts/cardfly2.svg"/></div>
                        </div>
                        <div className="cardfly-wrap part-wrap" data-num="5">
                            <div className="cardfly part" data-num="1"><img src="assets/images/parts/cardfly3.svg"/></div>
                            <div className="cardfly part" data-num="2"><img src="assets/images/parts/cardfly2.svg"/></div>
                            <div className="cardfly part" data-num="3"><img src="assets/images/parts/cardfly1.svg"/></div>
                        </div>

                        <div className="static-lines"></div>

                        <div className="glow-line" data-num="1">
                            <div className="glow-ball"></div>
                        </div>
                        <div className="glow-line" data-num="2">
                            <div className="glow-ball"></div>
                        </div>
                        <div className="glow-line" data-num="3">
                            <div className="glow-ball"></div>
                        </div>
                        <div className="glow-line" data-num="4">
                            <div className="glow-ball"></div>
                        </div>

                    </div>
                </div>

                <div className="contentContainer">
                    <h1 className="mob">{I18N.get('landing.elastos')}</h1>
                    <h2 className="mob">{I18N.get('landing.header')}</h2>
                    <div className="cta-btn" style={{cursor: 'pointer'}}>
                        <p style={{paddingTop: '24px'}}>{I18N.get('landing.action.enter')} <strong>{I18N.get('landing.action.here')}</strong></p>
                        <div className="arrow sized"><img src="assets/images/arrow.svg"/></div>
                        <a href="/developer"></a>
                    </div>
                    <div className="cta-btn" style={{cursor: 'pointer'}} style={{marginRight: '24px'}}>
                        <p style={{paddingTop: '24px'}}>{I18N.get('landing.playVideo')} <strong>{I18N.get('landing.action.here')}</strong></p>
                        <div className="arrow sized"><img src="assets/images/arrow.svg"/></div>
                        <a onClick={this.playVideo.bind(this)}></a>
                    </div>
                </div>
            </section>

            <section id="what" className="hasAnim">

                <div className="contentContainer expanded">
                    <div className="row main spaced">
                        <div className="col left">
                            <div className="txt">
                                <header>
                                    <div className="tri-square sized"><img src="assets/images/tri-square.svg"/></div>
                                    <h3>{I18N.get('landing.whatIs')}</h3>
                                </header>

                                <p>{I18N.get('landing.whatIs.content.1')}</p>
                                <p>{I18N.get('landing.whatIs.content.2')}</p>
                            </div>
                        </div>
                        <div className="col right">
                            <div className="scale-wrap">
                                <img src="assets/images/what-spacer.png" className="spacer"/></div>

                            <div className="background">

                                <div className="burst part" data-num="1"><img src="assets/images/what-burst@2x.png"/></div>
                                <div className="burst part" data-num="2"><img src="assets/images/what-burst@2x.png"/></div>

                                <div className="layer base"><img src="assets/images/what-base@2x.png"/></div>

                                <div className="radio-group" data-num="1"></div>
                                <div className="radio-group" data-num="2"></div>
                                <div className="radio-group" data-num="3"></div>
                                <div className="radio-group" data-num="4"></div>
                                <div className="radio-group" data-num="5"></div>

                                <div className="glow-line" data-num="1">
                                    <div className="glow-ball"></div>
                                </div>
                                <div className="glow-line" data-num="2">
                                    <div className="glow-ball"></div>
                                </div>
                                <div className="glow-line" data-num="3">
                                    <div className="glow-ball"></div>
                                </div>

                            </div>
                        </div>
                    </div>
                </div>

                <div id="cr100" className="callout-text">
                    <div className="contentContainer">
                        <div className="bg-square"></div>
                        <div className="txt">
                            {this.props.language === 'zh' ?
                                <h2 className="hasStatic cr100content zh">
                                    {I18N.get('landing.cr100.content.1')}<br/><span>{I18N.get('landing.cr100.content.2')}</span><span>{I18N.get('landing.cr100.content.3')}</span>
                                </h2> :
                                <h2 className="hasStatic cr100content">
                                    {I18N.get('landing.cr100.content.1')}<br/>{I18N.get('landing.cr100.content.2')}<br/>{I18N.get('landing.cr100.content.3')}
                                </h2>
                            }

                            <div className="strike-text dsk">
                                <div className="strike-line"></div>
                                <p>{I18N.get('landing.cr100.content.4')}</p>
                            </div>

                            <div className="strike-text mob">
                                <div className="strike-line"></div>
                                <p>{I18N.get('landing.cr100.content.4')}</p>
                            </div>

                        </div>
                    </div>
                    <div className="cta-btn">
                        <p>View the <strong>CR100</strong></p>
                        <div className="arrow sized"><img src="assets/images/arrow.svg"/></div>
                        <a href="/cr100"></a>
                    </div>
                </div>

            </section>

            <section id="solution">
                <div className="contentContainer">

                    <header>
                        <div className="tri-square sized"><img src="assets/images/tri-square.svg"/></div>
                        <h3>{I18N.get('landing.elaSol')}</h3>
                    </header>

                    <div className="row spaced" data-num="1">
                        <div className="col left">
                            <div className="scale-group">
                                <div className="arc-plus sized"><img src="assets/images/arc-plus.svg"/></div>
                                <div className="illus sized"><img src="assets/images/solution-illus1.svg"/></div>
                            </div>
                        </div>
                        <div className="col right">
                            <p><strong>{I18N.get('landing.scalability')}</strong></p>
                            <div className="strike-text">
                                <div className="strike-line"></div>
                                <p>{I18N.get('landing.mainSideChain')}</p>
                            </div>
                            <p>{I18N.get('landing.solution.explained.1')}</p>
                        </div>
                    </div>

                    <div className="row spaced" data-num="2">
                        <div className="col left">
                            <p><strong>{I18N.get('landing.security')}</strong></p>
                            <div className="strike-text">
                                <div className="strike-line"></div>
                                <p>{I18N.get('landing.mergeMining')}</p>
                            </div>
                            <p>{I18N.get('landing.solution.explained.2')}</p>
                        </div>
                        <div className="col right">
                            <div className="scale-group">
                                <div className="illus sized"><img src="assets/images/solution-illus2.svg"/></div>
                                <div className="arc-plus sized"><img src="assets/images/arc-plus.svg"/></div>
                            </div>
                        </div>
                    </div>

                    <div className="row spaced" data-num="3">
                        <div className="col left">
                            <div className="scale-group">
                                <div className="arc-plus sized"><img src="assets/images/arc-plus.svg"/></div>
                                <div className="illus sized"><img src="assets/images/solution-illus3.svg"/></div>
                            </div>
                        </div>
                        <div className="col right">
                            <p><strong>{I18N.get('landing.consensus')}</strong></p>
                            <div className="strike-text">
                                <div className="strike-line"></div>
                                <p>{I18N.get('landing.consensusMulti')}</p>
                            </div>
                            <p>{I18N.get('landing.solution.explained.3')}</p>
                        </div>
                    </div>

                </div>
            </section>

            <section id="pillars" className="hasAnim">
                <div className="contentContainer">

                    <header>
                        <div className="tri-square sized"><img src="assets/images/tri-square.svg"/></div>
                        <h3>{I18N.get('landing.fourPillars')}<span className="dsk"> {I18N.get('landing.ofSmartWeb')}</span></h3>
                    </header>

                    <div className="pillar-boxes spaced">
                        <div className="pillar-box left" data-num="1">
                            <div className="pillar-type"><img src="assets/images/pillars-text1.svg"/></div>
                            <div className="contents">

                                <div className="illus">
                                    <img src="assets/images/pillars-illus1.svg" className="spacer"/>

                                    <div className="dot-wrap part-wrap" data-num="1">
                                        <div className="dot part"><img src="assets/images/parts/pillar1-dot.svg"/></div>
                                    </div>
                                    <div className="dot-wrap part-wrap" data-num="2">
                                        <div className="dot part"><img src="assets/images/parts/pillar1-dot.svg"/></div>
                                    </div>
                                    <div className="dot-wrap part-wrap" data-num="3">
                                        <div className="dot part"><img src="assets/images/parts/pillar1-dot.svg"/></div>
                                    </div>

                                    <div className="lock-group" data-num="1">
                                        <div className="lock top part">
                                            <div className="locktop"><img src="assets/images/parts/pillar1-locktop.svg"/></div>
                                            <div className="lock-line"></div>
                                        </div>
                                        <div className="lock bot part"><img src="assets/images/parts/pillar1-lockbot.svg"/></div>
                                    </div>
                                    <div className="lock-group" data-num="2">
                                        <div className="lock top part">
                                            <div className="locktop"><img src="assets/images/parts/pillar1-locktop.svg"/></div>
                                            <div className="lock-line"></div>
                                        </div>
                                        <div className="lock bot part"><img src="assets/images/parts/pillar1-lockbot.svg"/></div>
                                    </div>
                                    <div className="lock-group" data-num="3">
                                        <div className="lock top part">
                                            <div className="locktop"><img src="assets/images/parts/pillar1-locktop.svg"/></div>
                                            <div className="lock-line"></div>
                                        </div>
                                        <div className="lock bot part"><img src="assets/images/parts/pillar1-lockbot.svg"/></div>
                                    </div>
                                </div>

                                <p><strong>{I18N.get('landing.blockchain')}</strong></p>
                                <p>{I18N.get('landing.blockchain.content.1')}<br/> {I18N.get('landing.blockchain.content.2')}
                                    <br/> {I18N.get('landing.blockchain.content.3')}<br/> {I18N.get('landing.blockchain.content.4')}</p>
                            </div>
                        </div>
                        <div className="pillar-box right" data-num="2">
                            <div className="pillar-type"><img src="assets/images/pillars-text2.svg"/></div>
                            <div className="contents">
                                <div className="illus">
                                    <img src="assets/images/pillars-illus2.svg" className="spacer"/>

                                    <div className="box-wrap part-wrap" data-num="1">
                                        <div className="box part"><img src="assets/images/parts/pillar2-block.svg"/></div>
                                    </div>
                                    <div className="box-wrap part-wrap" data-num="2">
                                        <div className="box part"><img src="assets/images/parts/pillar2-block.svg"/></div>
                                    </div>
                                    <div className="box-wrap part-wrap" data-num="3">
                                        <div className="box part"><img src="assets/images/parts/pillar2-block.svg"/></div>
                                    </div>

                                    <div className="type-wrap part-wrap">
                                        <div className="type part"><img src="assets/images/parts/pillar2-type.svg"/></div>
                                    </div>

                                    <div className="shine part"><img src="assets/images/parts/pillar2-shine.svg"/></div>

                                </div>

                                <p><strong>{I18N.get('landing.runtime')}</strong></p>
                                <p>{I18N.get('landing.runtime.content.1')}<br/> {I18N.get('landing.runtime.content.2')}
                                    <br/> {I18N.get('landing.runtime.content.3')}<br/> {I18N.get('landing.runtime.content.4')}</p>
                            </div>
                        </div>
                    </div>

                    <div className="pillar-boxes spaced">
                        <div className="pillar-box left" data-num="3">
                            <div className="pillar-type"><img src="assets/images/pillars-text3.svg"/></div>
                            <div className="contents">
                                <div className="illus">
                                    <img src="assets/images/pillars-illus3.svg" className="spacer"/>

                                    <div className="scanner">
                                        <div className="line"></div>
                                        <div className="line"></div>
                                        <div className="line"></div>
                                    </div>

                                    <div className="dot-group">
                                        <div className="dot" data-num="1"></div>
                                        <div className="dot" data-num="2"></div>
                                    </div>

                                    <div className="graph-wrap part-wrap">
                                        <div className="graph part"><img src="assets/images/parts/pillar3-graphline.svg"/></div>
                                    </div>

                                </div>

                                <p><strong>{I18N.get('landing.software')}<br className="mob"/> {I18N.get('landing.software.devkit')}</strong></p>
                                <p>{I18N.get('landing.software.content.1')}<br/> {I18N.get('landing.software.content.2')}</p>
                            </div>
                        </div>
                        <div className="pillar-box right" data-num="4">
                            <div className="pillar-type"><img src="assets/images/pillars-text4.svg"/></div>
                            <div className="contents">
                                <div className="illus">
                                    <img src="assets/images/pillars-illus4.svg" className="spacer base"/>

                                    <div className="dot-group" data-num="1">
                                        <div className="dot" data-num="1"></div>
                                        <div className="dot" data-num="2"></div>
                                    </div>
                                    <div className="dot-group" data-num="2">
                                        <div className="dot" data-num="1"></div>
                                        <div className="dot" data-num="2"></div>
                                    </div>
                                    <div className="dot-group" data-num="3">
                                        <div className="dot" data-num="1"></div>
                                        <div className="dot" data-num="2"></div>
                                    </div>

                                    <div className="curve-path" data-num="1">
                                        <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 44.86413 112.88758"><title>pillar4-curve1</title>
                                            <path id="pillar-path1"
                                                d="M.681,112.88758V15.74908A10.87551,10.87551,0,0,1,11.55651,4.87357h0A10.87551,10.87551,0,0,1,22.43207,15.74906V68.25822h0A10.87551,10.87551,0,0,0,33.30756,79.13374h0a10.87551,10.87551,0,0,0,10.8755-10.87552V0"
                                                style={{
                                                    fill: 'none',
                                                    stroke: '#1de9b6',
                                                    'strokeMiterlimit': 10,
                                                    'strokeWidth': '1.36px'
                                                }}/>
                                        </svg>
                                        <div className="dot path-move" data-num="1"></div>
                                    </div>
                                    <div className="curve-path" data-num="2">
                                        <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 66.61518 124.64787"><title>pillar4-curve2</title>
                                            <path id="pillar-path2"
                                                d="M.681,0V68.93029a10.87549,10.87549,0,0,0,10.87547,10.8755h0A10.87555,10.87555,0,0,0,22.43206,68.93024V38.2958A10.87568,10.87568,0,0,1,33.30765,27.42025h0A10.87548,10.87548,0,0,1,44.18316,38.2957v74.79559a10.87556,10.87556,0,0,0,10.8755,10.87555h0a10.87548,10.87548,0,0,0,10.87547-10.87549V62.82553"
                                                style={{
                                                    fill: 'none',
                                                    stroke: '#1de9b6',
                                                    'strokeMiterlimit': 10,
                                                    'strokeWidth': '1.36px'
                                                }}/>
                                        </svg>
                                        <div className="dot path-move" data-num="2"></div>
                                    </div>

                                    <div className="radio-group" data-num="1"></div>
                                    <div className="radio-group" data-num="2"></div>
                                    <div className="radio-group" data-num="3"></div>

                                </div>

                                <p><strong>{I18N.get('landing.carrier')}</strong></p>
                                <p>{I18N.get('landing.carrier.content.1')}<br/> {I18N.get('landing.carrier.content.2')}
                                    <br/> {I18N.get('landing.carrier.content.3')}<br/> {I18N.get('landing.carrier.content.4')}</p>
                            </div>
                        </div>
                    </div>

                </div>
            </section>

            <section id="model" className="hasAnim">

                <div className="model-slides">

                    <div className="notch top cn"></div>
                    <div className="notch bot cn"></div>
                    <div className="notch top rt"></div>
                    <div className="notch bot lt"></div>

                    <div className="contentContainer">
                        <header>
                            <div className="tri-square sized"><img src="assets/images/tri-square-dark.svg"/></div>
                            <h3><span className="dsk">{I18N.get('landing.businessModel.the')} </span>{I18N.get('landing.businessModel')}</h3>
                        </header>

                        <div className="model-slides-wrap" id="model-slider">
                            <div className="model-slide">
                                <div className="vdiv"></div>
                                <div className="contents">
                                    <div className="model-num sized"><img src="assets/images/model-number1.svg"/></div>
                                    <div className="model-icon sized"><img src="assets/images/model-icon1.svg"/></div>
                                    <p>{I18N.get('landing.businessModel.content.1')}<br/> {I18N.get('landing.businessModel.content.2')}
                                        <br/> {I18N.get('landing.businessModel.content.3')}<br/> {I18N.get('landing.businessModel.content.4')}</p>
                                </div>
                            </div>
                            <div className="model-slide">
                                <div className="vdiv"></div>
                                <div className="contents">
                                    <div className="model-num sized"><img src="assets/images/model-number2.svg"/></div>
                                    <div className="model-icon sized"><img src="assets/images/model-icon2.svg"/></div>
                                    <p>{I18N.get('landing.businessModel.content.5')}<br/> {I18N.get('landing.businessModel.content.6')}
                                        <br/> {I18N.get('landing.businessModel.content.7')}</p>
                                </div>
                            </div>
                            <div className="model-slide">
                                <div className="vdiv"></div>
                                <div className="contents">
                                    <div className="model-num sized"><img src="assets/images/model-number3.svg"/></div>
                                    <div className="model-icon sized"><img src="assets/images/model-icon3.svg"/></div>
                                    <p>{I18N.get('landing.businessModel.content.8')}<br/> {I18N.get('landing.businessModel.content.9')}
                                        <br/> {I18N.get('landing.businessModel.content.10')}</p>
                                </div>
                            </div>

                            <div className="model-slide off">
                                <div className="vdiv"></div>
                                <div className="contents">
                                    <div className="model-num sized"><img src="assets/images/model-number4.svg"/></div>
                                    <div className="model-icon sized"><img src="assets/images/model-icon4.svg"/></div>
                                    <p>{I18N.get('landing.businessModel.content.11')}<br/> {I18N.get('landing.businessModel.content.12')}
                                        <br/> {I18N.get('landing.businessModel.content.13')}</p>
                                </div>
                            </div>
                            <div className="model-slide off">
                                <div className="vdiv"></div>
                                <div className="contents">
                                    <div className="model-num sized"><img src="assets/images/model-number5.svg"/></div>
                                    <div className="model-icon sized"><img src="assets/images/model-icon5.svg"/></div>
                                    <p>{I18N.get('landing.businessModel.content.14')}<br/> {I18N.get('landing.businessModel.content.15')}
                                        <br/> {I18N.get('landing.businessModel.content.16')}<br/> {I18N.get('landing.businessModel.content.17')}</p>
                                </div>
                            </div>
                            <div className="model-slide off">
                                <div className="vdiv"></div>
                                <div className="contents">
                                    <div className="model-num sized"><img src="assets/images/model-number6.svg"/></div>
                                    <div className="model-icon sized"><img src="assets/images/model-icon6.svg"/></div>
                                    <p>{I18N.get('landing.businessModel.content.18')}<br/> {I18N.get('landing.businessModel.content.19')}
                                        <br/> {I18N.get('landing.businessModel.content.20')}</p>
                                </div>
                            </div>

                            <div className="model-slide off">
                                <div className="vdiv"></div>
                                <div className="contents">
                                    <div className="model-num sized"><img src="assets/images/model-number7.svg"/></div>
                                    <div className="model-icon sized"><img src="assets/images/model-icon7.svg"/></div>
                                    <p>{I18N.get('landing.businessModel.content.21')}<br/> {I18N.get('landing.businessModel.content.22')}
                                        <br/> {I18N.get('landing.businessModel.content.23')}</p>
                                </div>
                            </div>
                            <div className="model-slide off">
                                <div className="vdiv"></div>
                                <div className="contents">
                                    <div className="model-num sized"><img src="assets/images/model-number8.svg"/></div>
                                    <div className="model-icon sized"><img src="assets/images/model-icon8.svg"/></div>
                                    <p>{I18N.get('landing.businessModel.content.24')}<br/> {I18N.get('landing.businessModel.content.25')}
                                        <br/> {I18N.get('landing.businessModel.content.26')}<br/> {I18N.get('landing.businessModel.content.27')}</p>
                                </div>
                            </div>
                            <div className="model-slide off">
                                <div className="vdiv"></div>
                                <div className="contents">
                                    <div className="model-num sized"><img src="assets/images/model-number9.svg"/></div>
                                    <div className="model-icon wide sized"><img src="assets/images/model-icon9.svg"/></div>
                                    <p>{I18N.get('landing.businessModel.content.28')}<br/> {I18N.get('landing.businessModel.content.29')}
                                        <br/> {I18N.get('landing.businessModel.content.30')}</p>
                                </div>
                            </div>

                            <div className="model-slide off">
                                <div className="vdiv"></div>
                                <div className="contents">
                                    <div className="model-num wide sized"><img src="assets/images/model-number10.svg"/></div>
                                    <div className="model-icon wide sized"><img src="assets/images/model-icon10.svg"/></div>
                                    <p>{I18N.get('landing.businessModel.content.31')}<br/> {I18N.get('landing.businessModel.content.32')}</p>
                                </div>
                            </div>

                        </div>

                        <div className="slide-controls">
                            <div className="slide-count"><span className="count-current">01</span><span className="slash">/</span><span className="count-total">04</span></div>

                            <div className="slide-arrows">
                                <div className="arrow-btn left off">
                                    <div className="arrow-border"></div>
                                    <div className="arrow-circle">
                                        <img src="assets/images/arrow-submit.svg"/>
                                    </div>
                                </div>
                                <div className="arrow-btn right">
                                    <div className="arrow-border"></div>
                                    <div className="arrow-circle">
                                        <img src="assets/images/arrow-submit.svg"/>
                                    </div>
                                </div>
                            </div>
                        </div>
                    </div>

                    <div className="background cn">
                        <div className="layer base"><img src="assets/images/model-bottom.svg"/></div>

                        <div className="cardfly-wrap part-wrap" data-num="1">
                            <div className="cardfly part" data-num="1"><img src="assets/images/parts/token-card3.svg"/></div>
                            <div className="cardfly part" data-num="2"><img src="assets/images/parts/token-card1.svg"/></div>
                            <div className="cardfly part" data-num="3"><img src="assets/images/parts/token-card2.svg"/></div>
                        </div>
                        <div className="cardfly-wrap part-wrap" data-num="2">
                            <div className="cardfly part" data-num="1"><img src="assets/images/parts/token-card2.svg"/></div>
                            <div className="cardfly part" data-num="2"><img src="assets/images/parts/token-card3.svg"/></div>
                            <div className="cardfly part" data-num="3"><img src="assets/images/parts/token-card1.svg"/></div>
                        </div>

                        <div className="spin-dial part" data-num="1"><img src="assets/images/parts/token-spinner.svg"/></div>
                        <div className="spin-dial part" data-num="2"><img src="assets/images/parts/token-spinner.svg"/></div>

                        <div className="radio-group" data-num="1"></div>
                        <div className="radio-group" data-num="2"></div>
                        <div className="radio-group" data-num="3"></div>

                        <div className="graph-wrap part-wrap">
                            <div className="graph part"><img src="assets/images/parts/app3-graphline.svg"/></div>
                        </div>

                        <div className="glow-line" data-num="1">
                            <div className="glow-ball"></div>
                        </div>
                        <div className="glow-line" data-num="2">
                            <div className="glow-ball"></div>
                        </div>
                        <div className="glow-line" data-num="3">
                            <div className="glow-ball"></div>
                        </div>

                    </div>

                </div>

                <div className="callout-wrap">
                    <div className="callout-text">
                        <div className="contentContainer">
                            <div className="bg-square"></div>
                            <div className="txt">
                                <p>{I18N.get('landing.vision.header')}</p>

                                {this.props.language === 'zh' ?
                                    <h2 className="hasStatic">
                                        {I18N.get('landing.vision.content.1')}
                                        {I18N.get('landing.vision.content.2')}
                                        {I18N.get('landing.vision.content.3')}
                                    </h2>
                                    : <h2 className="hasStatic">
                                        {I18N.get('landing.vision.content.1')}<br/>
                                        {I18N.get('landing.vision.content.2')}<br/>
                                        {I18N.get('landing.vision.content.3')}
                                    </h2>
                                }
                                <div className="arrow arrow-next sized"><img src="assets/images/arrow.svg"/></div>
                            </div>
                        </div>
                    </div>
                </div>

            </section>

            <section id="applications" className="hasAnim">
                <div className="contentContainer expanded">

                    <header>
                        <div className="tri-square sized"><img src="assets/images/tri-square.svg"/></div>
                        <h3>{I18N.get('landing.application.header')}</h3>

                        <p>{I18N.get('landing.application.content.1')}</p>
                        <p>{I18N.get('landing.application.content.2')}<br className="mob"/> {I18N.get('landing.application.content.2')}</p>
                    </header>

                    <div className="application-boxes spaced">
                        <div className="application-box big left" data-num="1">
                            <div className="background">
                                <img src="assets/images/applications1.svg" className="spacer"/>

                                <div className="radio-group light" data-num="1"></div>
                                <div className="radio-group light" data-num="2"></div>

                                <div className="phone-wrap part-wrap">
                                    <div className="phone-text part"><img src="assets/images/parts/app1-phone.svg"/><img src="assets/images/parts/app1-phone.svg"/></div>
                                </div>

                                <div className="card-wrap part-wrap">
                                    <div className="card part"><img src="assets/images/parts/app1-card.svg"/></div>
                                </div>

                                <div className="comp-wrap part-wrap">
                                    <div className="comp-text part"><img src="assets/images/parts/app1-comp-scroll.svg"/></div>
                                </div>

                            </div>
                            <div className="txt">
                                <p>{I18N.get('landing.usecase.content.1')}<br/> {I18N.get('landing.usecase.content.2')}
                                    <br/> {I18N.get('landing.usecase.content.3')}</p>
                            </div>
                        </div>
                        <div className="application-box small right" data-num="2">
                            <div className="inner">
                                <div className="background">
                                    <img src="assets/images/applications2.svg" className="spacer"/>

                                    <div className="build-wrap" data-num="1">
                                        <div className="building part"><img src="assets/images/parts/app2-building.svg"/></div>
                                        <div className="roof part">
                                            <div className="roof-color"><img src="assets/images/parts/app2-roof1.svg"/></div>
                                            <div className="roof-color top"><img src="assets/images/parts/app2-roof2.svg"/></div>
                                        </div>
                                        <img src="assets/images/parts/app2-slot.svg" className="slot"/>
                                        <div className="coin-wrap part-wrap">
                                            <div className="coin part" data-num="1"><img src="assets/images/parts/app2-coin1.svg"/></div>
                                            <div className="coin part" data-num="2"><img src="assets/images/parts/app2-coin2.svg"/></div>
                                        </div>
                                    </div>

                                    <div className="build-wrap" data-num="2">
                                        <div className="building part"><img src="assets/images/parts/app2-building.svg"/></div>
                                        <div className="roof part">
                                            <div className="roof-color"><img src="assets/images/parts/app2-roof2.svg"/></div>
                                            <div className="roof-color top"><img src="assets/images/parts/app2-roof1.svg"/></div>
                                        </div>
                                        <img src="assets/images/parts/app2-slot.svg" className="slot"/>
                                        <div className="coin-wrap part-wrap">
                                            <div className="coin part" data-num="1"><img src="assets/images/parts/app2-coin2.svg"/></div>
                                            <div className="coin part" data-num="2"><img src="assets/images/parts/app2-coin1.svg"/></div>
                                        </div>
                                    </div>

                                    <div className="build-wrap" data-num="3">
                                        <div className="building part"><img src="assets/images/parts/app2-building.svg"/></div>
                                        <div className="roof part">
                                            <div className="roof-color"><img src="assets/images/parts/app2-roof1.svg"/></div>
                                            <div className="roof-color top"><img src="assets/images/parts/app2-roof2.svg"/></div>
                                        </div>
                                        <img src="assets/images/parts/app2-slot.svg" className="slot"/>
                                        <div className="coin-wrap part-wrap">
                                            <div className="coin part" data-num="1"><img src="assets/images/parts/app2-coin1.svg"/></div>
                                            <div className="coin part" data-num="2"><img src="assets/images/parts/app2-coin2.svg"/></div>
                                        </div>
                                    </div>

                                </div>
                                <div className="txt"><p>{I18N.get('landing.usecase.content.4')}<br/> {I18N.get('landing.usecase.content.5')}</p></div>
                            </div>
                        </div>
                    </div>

                    <div className="application-boxes spaced">
                        <div className="application-box small left" data-num="3">
                            <div className="inner">
                                <div className="background">
                                    <img src="assets/images/applications3.svg" className="spacer"/>

                                    <div className="graph-wrap part-wrap">
                                        <div className="graph part"><img src="assets/images/parts/app3-graphline.svg"/></div>
                                    </div>

                                    <div className="dot-wrap part-wrap">
                                        <div className="line" data-num="1"></div>
                                        <div className="line" data-num="2"></div>
                                        <div className="dot" data-num="1"></div>
                                        <div className="dot" data-num="2"></div>
                                    </div>

                                    <div className="print-wrap">
                                        <div className="print part"><img src="assets/images/parts/app3-print.svg"/></div>

                                        <div className="print-line" data-num="1">
                                            <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 54 24">
                                                <path id="app-path1" d="M.12476,1.67685A52.14724,52.14724,0,0,1,53.746,23.58424"
                                                    style={{
                                                        fill: 'none',
                                                        stroke: '#1de9b6',
                                                        'strokeMiterlimit': 10,
                                                        'strokeWidth': '1.25px'
                                                    }}/>
                                            </svg>
                                            <div className="dot"></div>
                                        </div>
                                        <div className="print-line" data-num="2">
                                            <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 37.6 8">
                                                <path id="app-path2" d="M.35051,7.2935A39.1712,39.1712,0,0,1,37.35907,3.67705"
                                                    style={{
                                                        fill: 'none',
                                                        stroke: '#1de9b6',
                                                        'strokeMiterlimit': 10,
                                                        'strokeWidth': '1.25px'
                                                    }}/>
                                            </svg>
                                            <div className="dot"></div>
                                        </div>
                                        <div className="print-line" data-num="3">
                                            <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 8 43">
                                                <path id="app-path3" d="M.625,43.09323V22.17561A38.85743,38.85743,0,0,1,7.32708.3509"
                                                    style={{
                                                        fill: 'none',
                                                        stroke: '#1de9b6',
                                                        'strokeMiterlimit': 10,
                                                        'strokeWidth': '1.25px'
                                                    }}/>
                                            </svg>
                                            <div className="dot"></div>
                                        </div>
                                        <div className="print-line" data-num="4">
                                            <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 5 48">
                                                <path id="app-path4" d="M4.72147,48.12952A51.762,51.762,0,0,1,.625,27.89V0"
                                                    style={{
                                                        fill: 'none',
                                                        stroke: '#1de9b6',
                                                        'strokeMiterlimit': 10,
                                                        'strokeWidth': '1.25px'
                                                    }}/>
                                            </svg>
                                            <div className="dot"></div>
                                        </div>
                                        <div className="print-line" data-num="5">
                                            <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 23 32.5">
                                                <path id="app-path5" d="M22.36567,0V20.91749a10.87033,10.87033,0,1,1-21.74067,0h0"
                                                    style={{
                                                        fill: 'none',
                                                        stroke: '#1de9b6',
                                                        'strokeMiterlimit': 10,
                                                        'strokeWidth': '1.25px'
                                                    }}/>
                                            </svg>
                                            <div className="dot"></div>
                                        </div>
                                        <div className="print-line" data-num="6">
                                            <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 32.5 62.5">
                                                <path id="app-path6" d="M32.051,0V13.945A52.19366,52.19366,0,0,1,.24412,61.89471"
                                                    style={{
                                                        fill: 'none',
                                                        stroke: '#1de9b6',
                                                        'strokeMiterlimit': 10,
                                                        'strokeWidth': '1.25px'
                                                    }}/>
                                            </svg>
                                            <div className="dot"></div>
                                        </div>

                                    </div>

                                </div>
                                <div className="txt">
                                    <p>{I18N.get('landing.usecase.content.6')}<br/> {I18N.get('landing.usecase.content.7')}
                                        <br/> {I18N.get('landing.usecase.content.8')}</p>
                                </div>
                            </div>
                        </div>
                        <div className="application-box big right" data-num="4">
                            <div className="background">
                                <img src="assets/images/applications4.svg" className="spacer"/>
                                <img src="assets/images/parts/app4-shadow.svg" className="shadow"/>

                                <div className="card-circle">
                                    <div className="card part" data-num="1"><img src="assets/images/parts/app4-card1.svg"/></div>
                                    <div className="card part" data-num="2"><img src="assets/images/parts/app4-card2.svg"/></div>
                                    <div className="card part" data-num="3"><img src="assets/images/parts/app4-card1.svg"/></div>
                                    <div className="card part" data-num="4"><img src="assets/images/parts/app4-card3.svg"/></div>

                                </div>

                                <div className="lock part">
                                    <img src="assets/images/parts/app4-lock.svg"/>
                                    <div className="check part"><img src="assets/images/parts/app4-check.svg"/></div>
                                    <div className="dot-group">
                                        <div className="dot" data-num="1"></div>
                                        <div className="dot" data-num="2"></div>
                                        <div className="dot" data-num="3"></div>
                                        <div className="dot" data-num="4"></div>
                                    </div>
                                </div>

                            </div>
                            <div className="txt">
                                <p>{I18N.get('landing.usecase.content.9')}<br/> {I18N.get('landing.usecase.content.10')}
                                    <br/> {I18N.get('landing.usecase.content.11')}</p>
                            </div>
                        </div>
                    </div>

                </div>

            </section>

            <section id="token" className="hasAnim">
                <div className="contentContainer">

                    <div className="row spaced">
                        <div className="col left">
                            <div className="scale-wrap">
                                <div className="background">
                                    <div className="layer base"><img src="assets/images/token-01.svg"/></div>
                                    <div className="square-ornament">
                                        <div className="sq fill sm"></div>
                                        <div className="sq fill med"></div>
                                        <div className="sq diags"><img src="assets/images/diags-blue.svg"/></div>
                                    </div>

                                    <div className="coin-wrap part-wrap">
                                        <div className="coin part" data-num="1"><img src="assets/images/parts/token-coin.svg"/></div>
                                        <div className="coin part" data-num="2"><img src="assets/images/parts/token-coin.svg"/></div>
                                        <div className="coin part" data-num="3"><img src="assets/images/parts/token-coin.svg"/></div>
                                    </div>

                                    <div className="card-wrap part-wrap">
                                        <div className="card part" data-num="1"><img src="assets/images/parts/token-card1.svg"/></div>
                                        <div className="card part" data-num="2"><img src="assets/images/parts/token-card2.svg"/></div>
                                        <div className="card part" data-num="3"><img src="assets/images/parts/token-card3.svg"/></div>
                                    </div>

                                    <div className="dot-group" data-num="1">
                                        <div className="dot" data-num="1"></div>
                                        <div className="dot" data-num="2"></div>
                                        <div className="dot" data-num="3"></div>
                                        <div className="dot" data-num="4"></div>
                                    </div>
                                    <div className="dot-group" data-num="2">
                                        <div className="dot" data-num="1"></div>
                                        <div className="dot" data-num="2"></div>
                                        <div className="dot" data-num="3"></div>
                                        <div className="dot" data-num="4"></div>
                                    </div>

                                    <div className="spin-dial part" data-num="1"><img src="assets/images/parts/token-spinner.svg"/></div>
                                    <div className="spin-dial part" data-num="2"><img src="assets/images/parts/token-spinner.svg"/></div>
                                    <div className="spin-dial part" data-num="3"><img src="assets/images/parts/token-spinner.svg"/></div>

                                    <div className="print-wrap part-wrap">
                                        <div className="print-scroll">
                                            <img src="assets/images/parts/token-print.svg" className="print-img"/>
                                            <div className="line"></div>
                                            <img src="assets/images/parts/token-print.svg" className="print-img"/>
                                        </div>
                                    </div>

                                    <div className="type-wrap part-wrap">
                                        <div className="type part"><img src="assets/images/parts/token-type.svg"/></div>
                                    </div>

                                    <div className="bigA part"><img src="assets/images/parts/token-a.svg"/></div>

                                    <div className="cardfly-wrap part-wrap" data-num="1">
                                        <div className="cardfly part" data-num="1"><img src="assets/images/parts/cardfly1.svg"/></div>
                                        <div className="cardfly part" data-num="2"><img src="assets/images/parts/cardfly3.svg"/></div>
                                        <div className="cardfly part" data-num="3"><img src="assets/images/parts/cardfly2.svg"/></div>
                                    </div>
                                    <div className="cardfly-wrap part-wrap" data-num="2">
                                        <div className="cardfly part" data-num="1"><img src="assets/images/parts/cardfly2.svg"/></div>
                                        <div className="cardfly part" data-num="2"><img src="assets/images/parts/cardfly1.svg"/></div>
                                        <div className="cardfly part" data-num="3"><img src="assets/images/parts/cardfly3.svg"/></div>
                                    </div>

                                    <div className="radio-group light" data-num="1"></div>
                                    <div className="radio-group light" data-num="2"></div>
                                    <div className="radio-group" data-num="3"></div>

                                    <div className="token-city part"><img src="assets/images/parts/token-city.svg"/></div>

                                    <div className="glow-line">
                                        <div className="glow-ball"></div>
                                    </div>

                                </div>
                            </div>
                        </div>
                        <div className="col right">

                            <div className="txt">
                                <div className="square-ornament">
                                    <div className="sq fill sm"></div>
                                    <div className="sq fill med"></div>
                                    <div className="sq diags"><img src="assets/images/diags-green.svg"/></div>
                                </div>

                                <header>
                                    <div className="tri-square sized"><img src="assets/images/tri-square.svg"/></div>
                                    <h3>{I18N.get('landing.elaToken')}</h3>
                                </header>
                                <p>{I18N.get('landing.elaToken.content.1')}</p>
                                <p>{I18N.get('landing.elaToken.content.2')}</p>
                                <p>{I18N.get('landing.elaToken.content.3')}</p>
                            </div>
                        </div>
                    </div>

                </div>

                <div className="bg-squares">
                    <div className="square-ornament left">
                        <div className="sq fill sm"></div>
                        <div className="sq fill med"></div>
                        <div className="sq fill lrg"></div>
                    </div>
                    <div className="square-ornament right">
                        <div className="sq fill sm"></div>
                        <div className="sq fill med"></div>
                        <div className="sq fill lrg"></div>
                        <div className="sq diags"><img src="assets/images/diags-blue.svg"/></div>
                    </div>
                </div>
            </section>

            <section id="cyber" className="hasAnim">
                <div className="contentContainer">
                    <div className="callout-text">
                        <div className="contentContainer">
                            <div className="bg-square"></div>
                            <div className="shield part"><img src="assets/images/shield-cyber.svg"/></div>
                            <div className="txt">
                                <h2 className="hasStatic">{I18N.get('landing.elastos')}<br/> {I18N.get('landing.cr')}</h2>

                                <div className="strike-text dsk">
                                    <div className="strike-line"></div>
                                    <p>{I18N.get('landing.contribute.1')}</p>
                                </div>
                                <div className="strike-text dsk">
                                    <div className="strike-line"></div>
                                    <p>{I18N.get('landing.contribute.2')}</p>
                                </div>

                                <div className="strike-text mob">
                                    <div className="strike-line"></div>
                                    <p>{I18N.get('landing.contribute.mob.1')}</p>
                                </div>

                                <div className="strike-text mob">
                                    <div className="strike-line"></div>
                                    <p>{I18N.get('landing.contribute.mob.2')}</p>
                                </div>

                                <div className="strike-text mob">
                                    <div className="strike-line"></div>
                                    <p>{I18N.get('landing.contribute.mob.3')}</p>
                                </div>

                                <div className="strike-text mob filler"></div>

                                <div className="strike-text mob">
                                    <div className="strike-line"></div>
                                    <p>{I18N.get('landing.contribute.mob.4')}</p>
                                </div>

                                <div className="strike-text mob filler"></div>

                                <div className="strike-text mob">
                                    <div className="strike-line"></div>
                                    <p>{I18N.get('landing.contribute.mob.5')}</p>
                                </div>

                            </div>
                        </div>
                        <div className="cta-btn">
                            <p>{I18N.get('landing.contribute.action.1')} <strong>{I18N.get('landing.contribute.action.2')}</strong></p>
                            <div className="arrow sized"><img src="assets/images/arrow.svg"/></div>
                            <a href="/register"></a>
                        </div>
                    </div>
                </div>

                <div className="bg-squares">
                    <div className="square-ornament top">
                        <div className="sq fill sm"></div>
                        <div className="sq fill med"></div>
                        <div className="sq diags"><img src="assets/images/diags-green.svg"/></div>
                    </div>
                    <div className="square-ornament left">
                        <div className="sq fill sm"></div>
                        <div className="sq fill med"></div>
                        <MediaQuery minWidth={MIN_WIDTH_PC}>
                            <div className="sq fill lrg"></div>
                            <div className="sq diags"><img src="assets/images/diags-blue.svg"/></div>
                        </MediaQuery>
                    </div>
                    <div className="square-ornament right">
                        <div className="sq fill sm"></div>
                        <div className="sq fill lrg"></div>
                    </div>
                </div>

                <div className="notch bot rt"></div>
                <div className="notch bot cn"></div>
                <div className="notch bot lt"></div>
            </section>

            <section id="team">

                <div className="contentContainer">
                    <header>
                        <div className="tri-square sized"><img src="assets/images/tri-square-dark.svg"/></div>
                        <h3>{I18N.get('landing.empower35.header')}</h3>
                    </header>

                    <div className="team-grid spaced">
                        <div className="team-box">
                            <div className="team-photo sized"><img src="assets/images/team-placeholder@2x.png"/></div>
                            <div className="txt">
                                <h4>{I18N.get('landing.empower35.partnership')}</h4>
                            </div>
                        </div>
                        <div className="team-box">
                            <div className="team-photo sized"><img src="assets/images/team-placeholder@2x.png"/></div>
                            <div className="txt">
                                <h4>{I18N.get('landing.empower35.marketer')}</h4>
                            </div>
                        </div>
                        <div className="team-box">
                            <div className="team-photo sized"><img src="assets/images/team-placeholder@2x.png"/></div>
                            <div className="txt">
                                <h4>{I18N.get('landing.empower35.legal')}</h4>
                            </div>
                        </div>
                        <div className="team-box">
                            <div className="team-photo sized"><img src="assets/images/team-placeholder@2x.png"/></div>
                            <div className="txt">
                                <h4>{I18N.get('landing.empower35.video')}</h4>
                            </div>
                        </div>
                        <div className="team-box">
                            <div className="team-photo sized"><img src="assets/images/team-placeholder@2x.png"/></div>
                            <div className="txt">
                                <h4>{I18N.get('landing.empower35.designer')}</h4>
                            </div>
                        </div>
                        <div className="team-box">
                            <div className="team-photo sized"><img src="assets/images/team-placeholder@2x.png"/></div>
                            <div className="txt">
                                <h4>{I18N.get('landing.empower35.writer')}</h4>
                            </div>
                        </div>
                        <div className="team-box">
                            <div className="team-photo sized"><img src="assets/images/team-placeholder@2x.png"/></div>
                            <div className="txt">
                                <h4>{I18N.get('landing.empower35.dapp')}</h4>
                            </div>
                        </div>
                        <div className="team-box">
                            <div className="team-photo sized"><img src="assets/images/team-placeholder@2x.png"/></div>
                            <div className="txt">
                                <h4>{I18N.get('landing.empower35.evangelist')}</h4>
                            </div>
                        </div>

                        <div className="team-box cta">
                            <div className="cta-btn">
                                <p>{I18N.get('landing.empower35.action.1')} <strong>{I18N.get('landing.empower35.action.2')}</strong></p>
                                <div className="arrow sized"><img src="assets/images/arrow.svg"/></div>
                                <a href="/crcles"></a>
                            </div>
                        </div>
                    </div>

                </div>

            </section>

            <footer id="globalFooter" className="global hasAnim">

                <div className="footer-illus">
                    <div className="footer-illus-bg part dsk">
                        <img src="assets/images/footer-illus-bg.svg"/>

                        <div className="glow-line" data-num="1">
                            <div className="glow-ball"></div>
                        </div>
                        <div className="glow-line" data-num="2">
                            <div className="glow-ball"></div>
                        </div>
                        <div className="glow-line" data-num="3">
                            <div className="glow-ball"></div>
                        </div>
                        <div className="glow-line" data-num="4">
                            <div className="glow-ball"></div>
                        </div>

                    </div>
                    <div className="footer-illus-bg part mob"><img src="assets/images/footer-illus-bg-mob.svg"/></div>

                    <div className="footer-city">
                        <img src="assets/images/footer-city.svg" className="base"/>

                        <div className="cardfly-wrap part-wrap" data-num="1">
                            <div className="cardfly part" data-num="1"><img src="assets/images/parts/token-card2.svg"/></div>
                            <div className="cardfly part" data-num="2"><img src="assets/images/parts/token-card3.svg"/></div>
                            <div className="cardfly part" data-num="3"><img src="assets/images/parts/token-card1.svg"/></div>
                        </div>
                        <div className="cardfly-wrap part-wrap" data-num="2">
                            <div className="cardfly part" data-num="1"><img src="assets/images/parts/token-card3.svg"/></div>
                            <div className="cardfly part" data-num="2"><img src="assets/images/parts/token-card1.svg"/></div>
                            <div className="cardfly part" data-num="3"><img src="assets/images/parts/token-card2.svg"/></div>
                        </div>

                        <div className="radio-group" data-num="1"></div>
                        <div className="radio-group" data-num="2"></div>

                        <div className="glow-line" data-num="1">
                            <div className="glow-ball"></div>
                        </div>
                        <div className="glow-line" data-num="2">
                            <div className="glow-ball"></div>
                        </div>

                    </div>
                    <div className="bg-square"></div>
                </div>

                <div className="contentContainer">

                    <h2 className="hasStatic">{I18N.get('landing.footer.header.1')}<br/> {I18N.get('landing.footer.header.2')}</h2>

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

                    <div className="footer-row spaced">
                        <div className="col" data-num="1">
                            <img src="assets/images/footer-shield.svg" className="spacer"/>
                        </div>

                        <div className="col" data-num="2">
                            <ul className="resources">
                                <li className="title">{I18N.get('landing.footer.resources')}</li>
                                <li><a href="/vision" target="_blank">{I18N.get('vision.00')}</a></li>
                                <li><a href="https://wallet.elastos.org/" target="_blank">{I18N.get('landing.footer.wallet')}</a></li>
                                <li><a href="https://blockchain.elastos.org/status" target="_blank">{I18N.get('landing.footer.explorer')}</a></li>
                                <li><a href="https://github.com/elastos" target="_blank">{I18N.get('landing.footer.github')}</a></li>
                                <li><a href="https://github.com/elastos/Elastos.Community/tree/master/CyberRepublicLogoAssets" target="_blank">{I18N.get('landing.footer.assets')}</a></li>
                                <li><a href="https://elanews.net/">{I18N.get('landing.footer.elaNews')}</a></li>
                                <li><a href="/privacy">{I18N.get('landing.footer.privacyPolicy')}</a></li>
                                <li><a href="/terms">{I18N.get('landing.footer.termsAndConditions')}</a></li>
                            </ul>
                        </div>

                        <div className="vdiv"></div>

                        <div className="col contact" data-num="3">
                            <ul>
                                <li className="title">{I18N.get('landing.footer.contact')}</li>
                                <li>{I18N.get('landing.cr')}: <a href="mailto:cyberrepublic@elastos.org">cyberrepublic@elastos.org</a></li>
                                <li>{I18N.get('landing.footer.community')}: <a href="mailto:global-community@elastos.org">global-community@elastos.org</a></li>
                                <li>{I18N.get('landing.footer.support')}: <a href="mailto:support@elastos.org">support@elastos.org</a></li>
                                <li>{I18N.get('landing.footer.contacts')}: <a href="mailto:contact@elastos.org">contact@elastos.org</a></li>
                            </ul>
                        </div>

                        <div className="vdiv"></div>

                        <div className="col" data-num="4">
                            <ul className="social">
                                <li className="title">{I18N.get('landing.footer.join')}</li>
                            </ul>

                            <div className="social-icons">
                                <a href="https://t.me/elastosgroup" target="_blank"><i className="fab fa-telegram fa-2x"/></a>
                                <a href="https://github.com/cyber-republic" target="_blank"><i className="fab fa-github fa-2x"/></a>
                                <a href="https://discord.gg/UG9j6kh" target="_blank"><i className="fab fa-discord fa-2x"/></a>
                                <br/>
                                <a href="https://twitter.com/cyber__republic" target="_blank"><i className="fab fa-twitter fa-2x"/></a>
                                <a href="https://www.facebook.com/ElastosCyberRepublic" target="_blank"><i className="fab fa-facebook fa-2x"/></a>
                                <a href="https://www.reddit.com/r/CyberRepublic/" target="_blank"><i className="fab fa-reddit fa-2x"/></a>
                                <br/>
                                <a href="https://www.youtube.com/channel/UCjHthS-zJr0axZF5Iw8En-w" target="_blank"><i className="fab fa-youtube fa-2x"/></a>
                                <a href="https://www.instagram.com/cyberrepublic/" target="_blank"><i className="fab fa-instagram fa-2x"/></a>
                                <a href="https://www.linkedin.com/company/cyber-republic/" target="_blank"><i className="fab fa-linkedin fa-2x"/></a>
                            </div>
                        </div>
                    </div>

                    <div className="credit"><p>Design: <a href="http://www.griflan.com" target="_blank">Griflan</a></p></div>

                </div>
            </footer>

            <div id="video-overlay">
                <div className="blanket"></div>

                <div className="overlayWrap">
                    <div className="contentContainer">
                        <div id="video-contents">
                            <img src="assets/images/lbvideo-spacer.png" className="video-spacer"/>
                            <div className="close-btn">
                                <div className="close-line right"></div>
                                <div className="close-line left">
                                    <div className="close-hover"></div>
                                </div>
                            </div>

                            <video controls id="lbvid"></video>
                        </div>
                    </div>
                </div>

            </div>
        </div>
    }
}
