import React from 'react'
import Footer from '@/module/layout/Footer/Container'
import I18N from '@/I18N'
import './style.scss'
import { Col, Row, Card, Button, Breadcrumb, Icon, List, Spin, Avatar, Modal } from 'antd'
import _ from 'lodash'
import StandardPage from '../StandardPage'

export default class extends StandardPage {

  componentDidMount() {
    // localStorage.setItem('popup-update', 'false')
    this.setState({ loading: true })
    this.props.getTasks().then(() => {
      this.setState({ loading: false })
    })
  }

  componentWillUnmount() {
    this.props.resetTasks()
  }

  checkForLoading(followup) {
    return this.state.loading
      ? <Spin className="spinner" size="large"/>
      : _.isFunction(followup) && followup()
  }

  ord_states() {
    return {
      loading: false
    }
  }

  showDetailModal(id) {
    this.props.history.push(`/project-detail/${id}`)
  }

  // TODO: what's up with these admin CSS classes?
  ord_renderContent () {
    return (
      <div className="p_Cr100">
        <div className="ebp-header-divider" />
        <div className="p_admin_index ebp-wrap">
          <div className="d_box">
            <div className="p_admin_content">
              {this.buildHeader()}
              {this.checkForLoading(this.buildList.bind(this))}
              {this.buildDisclaimer()}
              {this.buildFooter()}
            </div>
          </div>
        </div>
        <Footer/>
      </div>
    )
  }

  buildHeader() {
    return (
      <div className="cr100-header">
        <div className="welcomeBox">
          <div className="title">
            {I18N.get('developer.cr100.welcome.title')}
          </div>
          <div className="content">
            <p>{I18N.get('developer.cr100.welcome.1')}</p>
            <p>{I18N.get('developer.cr100.welcome.2')}</p>
            <p>{I18N.get('developer.cr100.welcome.3')}</p>
          </div>

          <img className="cr100_logo" src="/assets/images/CR100_Logo.png"/>
          <img className="oomph_box" src="/assets/images/oomph.png"/>
        </div>
      </div>
    )
  }

  breakTextOn(text, delim) {
    const texts = text.split(delim)
    const elements = []
    for (let i = 0; i < texts.length; i++) {
      elements.push(
        <span key={i}>
          {texts[i]}
          { i < texts.length - 1 && ' / ' }
          <wbr/>
        </span>
      )
    }
    return elements
  }

  buildList() {
    // Note, the project can be in multiple domains, but categorizing by the top one
    const categorizedList = _.groupBy(this.props.all_tasks, (task) => _.first(task.domain))
    const link = 'https://s3-us-west-1.amazonaws.com/ebp-staging-files/cr100/'

    let list = _.map(categorizedList, (list, category) => {
      list = _.sortBy(_.values(list), ['dAppId'])
      const sanitizedCategory = (category || 'uncategorized').toLowerCase()
      return (
        <div key={sanitizedCategory}>
          <h3 className="category-title brand-color">
            {I18N.get(`team.spec.${sanitizedCategory}`)}
          </h3>
          <div className="c_projectList">
            {_.map(list, (project, ind) => (
              <div key={ind} className="c_project">
                <div className="project-icon">
                  <div className="base-icon"/>
                  <a href={`/project-detail/${project._id}`}>
                    <img className="overlay-icon" src={project.thumbnail}/>
                  </a>
                </div>
                <div className="caption">{this.breakTextOn(project.name, '/')}</div>
              </div>
            ))}
          </div>
        </div>
      )
    })

    list = _.sortBy(list, ['key'])

    return (
      <div className="c_list_wrapper">
        <div className="c_list">
          <div className="inner-container">
            <img className="cr100-logo-text" src="/assets/images/cr100_logo_text.png"/>
            <h2 className="project-title komu-a">
              {I18N.get('developer.cr100.projects')}
            </h2>
            {list}
          </div>
        </div>
      </div>
    )
  }

  buildDisclaimer() {
    return (
      <div className="disclaimer-box">
        <div className="welcomeBox">
          <div className="title">
            {I18N.get('developer.cr100.disclaimer.title')}
          </div>
          <div className="content">
            {I18N.get('developer.cr100.disclaimer')}
          </div>
          <div className="content">
            {I18N.get('developer.cr100.dontseeProject.title')}
          </div>
          <div className="content">
            <a href="mailto:cyberrepublic@elastos.org?subject=CR100 Project Proposal">
              <Button className="submit-idea-button">
                {/* onClick={this.handleSubmitProjectProposal.bind(this)}> */}
                {I18N.get('developer.cr100.dontseeProject')}
              </Button>
            </a>
          </div>
        </div>

        <img className="footer_enrich" src="/assets/images/footer_enrich.png"/>
      </div>
    )
  }

  buildFooter() {

  }

  handleSubmitProjectProposal() {

  }
}
