import React from 'react';
import BaseComponent from '@/model/BaseComponent';
import { Col, Row, Avatar } from 'antd'
import I18N from '@/I18N'

import { CR_LINKS, ELASTOS_LINKS } from "@/constant";

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
                                <div className="footer-color-dark"><a href="/vision">{I18N.get('vision.00')}</a></div>
                                <div className="footer-color-dark"><a href={ELASTOS_LINKS.WALLET} target="_blank">{I18N.get('landing.footer.wallet')}</a></div>
                                <div className="footer-color-dark"><a href={ELASTOS_LINKS.EXPLORER} target="_blank">{I18N.get('landing.footer.explorer')}</a></div>
                                <div className="footer-color-dark"><a href={ELASTOS_LINKS.GITHUB} target="_blank">{I18N.get('landing.footer.github')}</a></div>
                                <div className="footer-color-dark"><a href={`${ELASTOS_LINKS.GITHUB}/Elastos.Community/tree/master/CyberRepublicLogoAssets`} target="_blank">{I18N.get('landing.footer.assets')}</a></div>
                                <div className="footer-color-dark"><a href={ELASTOS_LINKS.NEWS} target="_blank">{I18N.get('landing.footer.elaNews')}</a></div>
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
                                <div className="footer-color-dark">{I18N.get('landing.footer.support')}: <a href="mailto:support@cyberrepublic.org">support@cyberrepublic.org</a></div>
                                <div className="footer-color-dark">{I18N.get('landing.footer.contacts')}: <a href="mailto:contact@cyberrepublic.org">contact@cyberrepublic.org</a></div>
                            </div>
                        </Col>
                        <Col xs={24} sm={24} md={7}>
                            <div className="join footer-vertical-section">
                                <div className="title brand-color">
                                    Join Us On
                                </div>
                                <div className="social-icons">
                                    <a href={CR_LINKS.TELEGRAM} target="_blank"><i className="fab fa-telegram fa-2x"/></a>
                                    <a href={CR_LINKS.GITHUB} target="_blank"><i className="fab fa-github fa-2x"/></a>
                                    <a href={`${process.env.FORUM_URL}/login`} target="_blank"><i className="far fa-comments fa-2x"/></a>
                                    <br/>
                                    <a href={CR_LINKS.TWITTER} target="_blank"><i className="fab fa-twitter fa-2x"/></a>
                                    <a href={CR_LINKS.FACEBOOK}target="_blank"><i className="fab fa-facebook fa-2x"/></a>
                                    <a href={CR_LINKS.REDDIT} target="_blank"><i className="fab fa-reddit fa-2x"/></a>
                                    <br/>
                                    <a href={CR_LINKS.YOUTUBE} target="_blank"><i className="fab fa-youtube fa-2x"/></a>
                                    <a href={CR_LINKS.INSTAGRAM} target="_blank"><i className="fab fa-instagram fa-2x"/></a>
                                    <a href={CR_LINKS.LINKEDIN} target="_blank"><i className="fab fa-linkedin fa-2x"/></a>
                                </div>
                            </div>
                        </Col>
                    </Row>
                </div>
            </div>
        );
    }
}
