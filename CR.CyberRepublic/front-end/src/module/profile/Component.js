import React from 'react';
import BaseComponent from '@/model/BaseComponent'
import UserEditForm from '@/module/form/UserEditForm/Container'
import I18N from '@/I18N'
import { Col, Row, Icon, Popover, Button, Spin, Tabs, Tag, Modal, Upload } from 'antd'
import moment from 'moment-timezone'
import {upload_file} from '@/util'
import {USER_AVATAR_DEFAULT, LINKIFY_OPTION} from '@/constant'
import config from '@/config'
import MediaQuery from 'react-responsive'
import linkifyStr from 'linkifyjs/string';
import './style.scss'

const TabPane = Tabs.TabPane

/**
 * This has 3 views
 *
 * 1. Public
 * 2. Admin
 * 3. Edit
 *
 */
export default class extends BaseComponent {

    constructor(props) {
        super(props)

        this.state = {
            editing: false
        }
    }

    // TODO: add twitter, telegram, linkedIn, FB
    ord_render () {
        if (_.isEmpty(this.props.user) || this.props.user.loading) {
            return <div class="center"><Spin size="large" /></div>;
        }

        return (
            <div className="c_Member public">
                <div>
                    <MediaQuery maxWidth={800}>
                        <div className="member-content member-content-mobile">
                            {this.renderMobile()}
                        </div>
                    </MediaQuery>
                    <MediaQuery minWidth={801}>
                        <div className="member-content">
                            {this.renderDesktop()}
                        </div>
                    </MediaQuery>
                </div>
            </div>
        );
    }

    renderMobile() {
        return (
            <div>
                {this.renderBanner(true)}
                <div className="profile-info-container profile-info-container-mobile clearfix">
                    {this.renderAvatar(true)}
                    {this.renderFullName(true)}
                    {this.renderLocation(true)}
                    {this.renderLocalTime(true)}
                    {this.renderDescription(true)}
                </div>

                <div className="profile-info-container profile-info-container-mobile clearfix">
                    <div className="pull-left skillset-header">
                        <h4 className="komu-a">
                            {I18N.get('profile.skillset.header')}
                        </h4>
                    </div>
                    <div className="pull-right skillset-content">
                        <Row>
                            <Col span={24}>
                                {this.renderSkillsets(true)}
                            </Col>
                            <Col span={24}>
                                {this.renderProfession(true)}
                            </Col>
                        </Row>
                    </div>
                </div>

                <div className="profile-info-container profile-info-container-mobile clearfix">
                    <div className="pull-left skillset-header">
                        <h4 className="komu-a">
                            {I18N.get('profile.social.header')}
                        </h4>
                    </div>
                    <div className="pull-right skillset-content">
                        <Row>
                            <Col span={24}>
                                {this.renderSocialMedia(true)}
                            </Col>
                        </Row>
                    </div>
                </div>

                {this.renderMetrics()}
                {this.renderEditForm()}
            </div>
        )
    }

    renderEditForm() {
        return (
            <Modal
                className="project-detail-nobar"
                visible={this.state.editing}
                onOk={this.switchEditMode.bind(this, false)}
                onCancel={this.switchEditMode.bind(this, false)}
                footer={null}
                width="70%"
            >
                { this.state.editing &&
                    <UserEditForm user={this.props.user}
                        switchEditMode={this.switchEditMode.bind(this, false)} completing={false}/>
                }
            </Modal>
        )
    }

    renderDesktop() {
        return (
            <div>
                {this.renderBanner()}
                <div className="profile-info-container clearfix">
                    <div className="profile-left pull-left">
                        {this.renderAvatar()}
                    </div>
                    <div className="profile-right pull-left">
                        {this.renderFullName()}
                        <Row>
                            <Col span={24} className="profile-right-col">
                                {this.renderGender()}
                            </Col>
                            <Col span={24} className="profile-right-col">
                                {this.renderLocation()}
                            </Col>
                            <Col span={24} className="profile-right-col">
                                {this.renderLocalTime()}
                            </Col>
                        </Row>
                    </div>
                    <div className="clearfix"/>
                    {this.renderDescription()}
                </div>

                <div className="profile-info-container clearfix">
                    <div className="pull-left skillset-header">
                        <h4 className="komu-a">
                            {I18N.get('profile.skillset.header')}
                        </h4>
                    </div>
                    <div className="pull-right skillset-content">
                        <Row>
                            <Col span={14}>
                                {this.renderSkillsets()}
                            </Col>
                            <Col span={10}>
                                {this.renderProfession()}
                            </Col>
                        </Row>
                    </div>
                </div>

                <div className="profile-info-container clearfix">
                    <div className="pull-left skillset-header">
                        <h4 className="komu-a">
                            {I18N.get('profile.social.header')}
                        </h4>
                    </div>
                    <div className="pull-right skillset-content">
                        <Row>
                            <Col span={24}>
                                {this.renderSocialMedia()}
                            </Col>
                        </Row>
                    </div>
                </div>

                {this.renderMetrics()}
                {this.renderEditForm()}
            </div>
        )
    }

    renderMetrics() {
        return (
            <Row gutter={16} className="profile-metrics">
                <Col md={24}>
                    {this.renderMetricItem(I18N.get('profile.followers'), this.props.user.subscribers.length)}
                </Col>
            </Row>
        )
    }

    renderMetricItem(label, value) {
        return (
            <div className="item">
                <div className="value">
                    {value}
                </div>
                <div className="center">
                    {label}
                </div>
            </div>
        )
    }

    renderBanner(isMobile, url) {
        return (
            <div className={`profile-banner ${isMobile ? 'profile-banner-mobile' : ''}`}>
                <span style={{ backgroundImage: this.getBannerWithFallback(url || this.props.user.profile.banner) }}></span>
                <Icon className="profile-edit-btn" type="edit" onClick={this.switchEditMode.bind(this)}/>
            </div>
        )
    }

    renderAvatar(isMobile) {
        const p_avatar = {
            showUploadList: false,
            customRequest: (info) => {
                this.setState({
                    avatar_loading: true
                })

                upload_file(info.file).then(async (d) => {
                    await this.props.updateUser(this.props.currentUserId, {
                        profile: {
                            avatar: d.url,
                            avatarFilename: d.filename,
                            avatarFileType: d.type
                        }
                    })

                    await this.props.getCurrentUser()

                    this.setState({
                        avatar_loading: false
                    })
                })
            }
        }

        return (
            <div className={`profile-avatar-container ${isMobile ? 'profile-avatar-container-mobile' : ''}`}>
                <div className="profile-avatar">
                    <Upload name="avatar" listType="picture-card"
                        className="avatar-uploader"
                        showUploadList={false}
                        {...p_avatar}
                    >
                        {this.state.avatar_loading
                            ? (
                                <div>
                                    <Icon type="loading"/>
                                </div>
                            )
                            : (
                                <img src={this.getAvatarWithFallback(
                                    this.props.user.profile.avatar)} />
                            )
                        }
                    </Upload>
                </div>
            </div>
        )
    }

    renderFullName(isMobile) {
        return (
            <h1 className={`komu-a profile-general-title ${isMobile ? 'profile-general-title-mobile' : ''}`}>
                {this.props.user.profile.firstName}&nbsp;
                {this.props.user.profile.lastName}
            </h1>
        )
    }

    renderLocation(isMobile) {
        return (
            <div className={`profile-general-info ${isMobile ? 'profile-general-info-mobile' : ''}`}>
                <Icon type="pushpin"/>
                <span>
                    {this.getCountryName(this.props.user.profile.country)}
                </span>
            </div>
        )
    }

    renderGender(isMobile) {
        return (
            <div className={`profile-general-info ${isMobile ? 'profile-general-info-mobile' : ''}`}>
                <Icon type="user"/>
                <span>
                    {_.capitalize(this.props.user.profile.gender)}
                </span>
            </div>
        )
    }

    renderSkillsets(isMobile) {
        return (
            <div className="skillset-container">
                {_.map(this.props.user.profile.skillset || [], (skillset) =>
                    <div key={skillset}>
                        + {I18N.get(`user.skillset.${skillset}`)}
                    </div>
                )}
            </div>
        )
    }

    renderProfession(isMobile) {
        return (
            <div className="profession-container">
                {this.props.user.profile.profession &&
                    <div>
                        {I18N.get(`profile.profession.${this.props.user.profile.profession}`)}
                    </div>
                }
                {!_.isEmpty(this.props.user.profile.portfolio) &&
                    <div className="portfolio-container">
                        <a href={this.getFullUrl(this.props.user.profile.portfolio)} target="_blank" className="link-container">
                            <Icon type="link"/> <span> {I18N.get('profile.portfolio')} </span>
                        </a>
                    </div>
                }
            </div>
        )
    }

    getBannerWithFallback(banner) {
        return _.isEmpty(banner)
            ? `url('/assets/images/profile-banner.png')`
            : `url(${banner})`
    }

    getAvatarWithFallback(avatar) {
        return _.isEmpty(avatar)
            ? USER_AVATAR_DEFAULT
            : avatar
    }

    renderLocalTime(isMobile) {
        const now = moment(Date.now())
        const user = this.props.user
        const localTime = user.profile.timezone
            ? now.tz(user.profile.timezone).format('LT z')
            : 'Unknown'

        return (
            <div className={`profile-general-info ${isMobile ? 'profile-general-info-mobile' : ''}`}>
                <Icon type="clock-circle"/>
                <span>
                    {I18N.get('profile.localTime')} {localTime}
                </span>
            </div>
        )
    }

    getFullUrl(url = '') {
        if (url.indexOf('http') < 0) {
            return `https://${url}`
        }

        return url
    }

    renderSocialMedia(isMobile) {
        const { profile } = this.props.user

        return (
            <div className={`profile-social ${isMobile ? 'profile-social-mobile' : ''}`}>
                {profile.telegram && <a href={this.getFullUrl(profile.telegram)} target="_blank"><i className="fab fa-telegram fa-2x"/></a>}
                {profile.twitter && <a href={this.getFullUrl(profile.twitter)} target="_blank"><i className="fab fa-twitter fa-2x"/></a>}
                {profile.facebook && <a href={this.getFullUrl(profile.facebook)} target="_blank"><i class="fab fa-facebook-square fa-2x"></i></a>}
                {profile.reddit && <a href={this.getFullUrl(profile.reddit)} target="_blank"><i className="fab fa-reddit fa-2x"/></a>}
                {profile.linkedin && <a href={this.getFullUrl(profile.linkedin)} target="_blank"><i class="fab fa-linkedin fa-2x"></i></a>}
                {profile.github && <a href={this.getFullUrl(profile.github)} target="_blank"><i class="fab fa-github fa-2x"></i></a>}
            </div>
        )
    }

    renderDescription(isMobile) {
        const bio = _.get(this.props, 'user.profile.bio') || ''
        const content = linkifyStr(bio, LINKIFY_OPTION)
        return (
            <div className="profile-container">
                {
                    this.props.user.profile.bio &&
                    <div className={`profile-description ${isMobile ? 'profile-description-mobile' : ''}`} dangerouslySetInnerHTML={{__html: content}}></div>
                }
            </div>
        )
    }

    getCountryName(countryCode) {
        return config.data.mappingCountryCodeToName[countryCode]
    }

    switchEditMode() {
        this.setState({
            editing: !this.state.editing
        })
    }
}
