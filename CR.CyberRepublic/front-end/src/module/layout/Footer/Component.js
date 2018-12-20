import React from 'react';
import BaseComponent from '@/model/BaseComponent';
import { Col, Row, Avatar } from 'antd'
import I18N from '@/I18N'

import './style.scss'

export default class extends BaseComponent {
    ord_render() {
        return (
            <div className="c_Footer">
                <div className="horizGap">

                </div>
                <div className="footer-box">
                    <Row className="d_rowFooter d_footerSection">
                        <Col className="shield" xs={24} sm={12} md={5}>
                            <img className="logo_own" src="/assets/images/footer-shield.svg"/>
                        </Col>
                        <Col className="resources" xs={24} sm={12} md={5}>
                            <div className="links footer-vertical-section">
                                <div className="title brand-color">
                                    {I18N.get('landing.footer.resources')}
                                </div>
                                <div className="footer-color-dark"><a href="/vision" target="_blank">{I18N.get('vision.00')}</a></div>
                                <div className="footer-color-dark"><a href="https://wallet.elastos.org/">{I18N.get('landing.footer.wallet')}</a></div>
                                <div className="footer-color-dark"><a href="https://blockchain.elastos.org/status">{I18N.get('landing.footer.explorer')}</a></div>
                                <div className="footer-color-dark"><a href="https://github.com/elastos" target="_blank">{I18N.get('landing.footer.github')}</a></div>
                                <div className="footer-color-dark"><a href="https://github.com/elastos/Elastos.Community/tree/master/CyberRepublicLogoAssets" target="_blank">{I18N.get('landing.footer.assets')}</a></div>
                                <div className="footer-color-dark"><a href="https://elanews.net/">{I18N.get('landing.footer.elaNews')}</a></div>
                                <div className="footer-color-dark"><a href="/privacy">{I18N.get('landing.footer.privacyPolicy')}</a></div>
                                <div className="footer-color-dark"><a href="/terms">{I18N.get('landing.footer.termsAndConditions')}</a></div>
                            </div>
                        </Col>
                        <Col className="vdiv"></Col>
                        <Col className="contact-container" xs={24} sm={24} md={7}>
                            <div className="contact footer-vertical-section">
                                <div className="title brand-color">
                                    {I18N.get('landing.footer.contact')}
                                </div>
                                <div className="footer-color-dark">{I18N.get('landing.cr')}: <a href="mailto:cyberrepublic@elastos.org">cyberrepublic@elastos.org</a></div>
                                <div className="footer-color-dark">{I18N.get('landing.footer.community')}: <a href="mailto:global-community@elastos.org">global-community@elastos.org</a></div>
                                <div className="footer-color-dark">{I18N.get('landing.footer.support')}: <a href="mailto:support@elastos.org">support@elastos.org</a></div>
                                <div className="footer-color-dark">{I18N.get('landing.footer.contacts')}: <a href="mailto:contact@elastos.org">contact@elastos.org</a></div>
                            </div>
                        </Col>
                        <Col xs={24} sm={24} md={7}>
                            <div className="join footer-vertical-section">
                                <div className="title brand-color">
                                    Join Us On
                                </div>
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
                        </Col>
                    </Row>
                </div>
            </div>
        );
    }
}
