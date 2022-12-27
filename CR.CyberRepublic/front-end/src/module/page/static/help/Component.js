import React from 'react'
import Footer from '@/module/layout/Footer/Container'
import './style.scss'

import { Col, Row, Menu } from 'antd'

import MediaQuery from 'react-responsive'
import StandardPage from '../../StandardPage'

export default class extends StandardPage {

  constructor (props) {
    super(props)

    this.state = {
      selectedHelpTopic: 'gettingStarted'
    }
  }

  componentDidMount() {
    if (this.props.location && this.props.location.search) {
      const qry = this.props.location.search.match(/[\\?&]topic=(\w+)/)
      if (qry.length > 1) {
        this.setState({
          selectedHelpTopic: qry[1]
        })
      }
    }
  }

  ord_renderContent () {
    return (
      <div className="p_help">
        <div className="ebp-header-divider" />

        <div className="ebp-page-title">
          <Row className="d_row d_rowGrey">
            <h3 className="page-header">
                            Help & Documentation
            </h3>
          </Row>
        </div>
        <div className="ebp-page">
          <Row className="d_row">
            <MediaQuery maxWidth={720}>
              <Col span={24}>
                <Menu
                  selectedKeys={[this.state.selectedHelpTopic]}
                  onClick={(item) => (this.setState({selectedHelpTopic: item.key}))}
                  mode="inline"
                >
                  <Menu.Item key="gettingStarted">
                                        Getting Started
                  </Menu.Item>
                  <Menu.Item key="developers">
                                        Developers
                  </Menu.Item>
                  <Menu.Item key="nonDevelopers">
                                        Non-Developers
                  </Menu.Item>
                  <Menu.Item key="organizers">
                                        Organizers
                  </Menu.Item>
                  <Menu.Item key="events">
                                        Creating Events/Tasks
                  </Menu.Item>
                  <Menu.Item key="meetupGuide">
                                        Meetup Guide
                  </Menu.Item>
                  <Menu.Item key="community">
                                        Community
                  </Menu.Item>
                </Menu>
              </Col>
            </MediaQuery>
            <Col xs={{span: 24}} md={{span: 19}} style={{paddingRight: '40px'}}>
              {this.renderMain()}
            </Col>
            <MediaQuery minWidth={720}>
              <Col xs={{span: 24}} md={{span: 5}}>
                <Menu
                  selectedKeys={[this.state.selectedHelpTopic]}
                  onClick={(item) => (this.setState({selectedHelpTopic: item.key}))}
                  mode="inline"
                >
                  <Menu.Item key="gettingStarted">
                                        Getting Started
                  </Menu.Item>
                  <Menu.Item key="developers">
                                        Developers
                  </Menu.Item>
                  <Menu.Item key="nonDevelopers">
                                        Non-Developers
                  </Menu.Item>
                  <Menu.Item key="organizers">
                                        Organizers
                  </Menu.Item>
                  <Menu.Item key="events">
                                        Creating Events/Tasks
                  </Menu.Item>
                  <Menu.Item key="meetupGuide">
                                        Meetup Guide
                  </Menu.Item>
                  <Menu.Item key="community">
                                        Community
                  </Menu.Item>
                </Menu>
              </Col>
            </MediaQuery>
          </Row>
        </div>
        <Footer/>
      </div>
    )
  }

  renderMain() {
    switch (this.state.selectedHelpTopic) {

      case 'gettingStarted':
        return this.renderGettingStarted()

      case 'developers':
        return this.renderDevelopers()

      case 'nonDevelopers':
        return this.renderNonDevelopers()

      case 'organizers':
        return this.renderOrganizers()

      case 'events':
        return this.renderEvents()

      case 'meetupGuide':
        return this.renderMeetupGuide()

      case 'community':
        return this.renderCommunity()
    }
  }

  renderGettingStarted() {
    return (
      <div>
        <h3>
                Getting Started
        </h3>

        <Row>
          <Col>
            <h4>
                        Create an Account
            </h4>

            <p>
                        The first thing you want to do is register, as a member you can only apply for existing events/task but
                        we'll also invite you to the Cyber Republic Slack channel.
              <br/>
            </p>
          </Col>
        </Row>
        <br/>
        <Row>
          <Col>
            <h4>
                        Join Discord
            </h4>

            <p>
                        After joining Cyber Republic please join our Discord channel at
              {' '}
              <a href="https://discord.gg/UG9j6kh">https://discord.gg/UG9j6kh</a>
              {' '}
and
                        Telegram at
              {' '}
              <a href="https://t.me/elastosgroup">https://t.me/elastosgroup</a>
            </p>
          </Col>
        </Row>
        <br/>
        <Row>
          <Col>
            <h4>
                        Watch Our Tutorial Videos
            </h4>

            <h5>Organizers Tutorial</h5>
            <iframe width="400" height="275" src="https://www.youtube.com/embed/AXtElROGXzA" frameBorder="0" allow="autoplay; encrypted-media" allowFullScreen={true} />

            <h5>General Task Tutorial</h5>
            <iframe width="400" height="275" src="https://www.youtube.com/embed/-90B2qzwOc8" frameBorder="0" allow="autoplay; encrypted-media" allowFullScreen={true} />
          </Col>
        </Row>
      </div>
    )
  }

  renderDevelopers() {
    return (
      <div>
        <h3>
                Developers
        </h3>

        <Row>
          <Col>
            <h4>
                        Read Our Documentation
            </h4>

            <p>
                        Elastos has many open source projects to learn from on Github and the documentation is also located there
              <br/>
              <br/>
              <a target="_blank" href="https://github.com/elastos">https://github.com/elastos</a>
            </p>
          </Col>
        </Row>
        <br/>
        <Row>
          <Col>
            <h4>
                        Elastos Developer's Beginners Guide
            </h4>

            <p>
                        One of the best resources to start with is the Developer's Beginners Guide
              <br/>
              <a target="_blank" href="https://github.com/elastos/Elastos/blob/master/DeveloperGuide/README.md">
                            https://github.com/elastos/Elastos/blob/master/DeveloperGuide/README.md
              </a>
            </p>
          </Col>
        </Row>
        <br/>
        <Row>
          <Col>
            <h4>
                        Ask Us Questions in Slack
            </h4>

            <p>
                        We're always here to help, in Slack there are developer specific channels you can come to ask questions.
              <br/>
              <br/>
                        There is no need to try to learn Elastos yourself, drop by Slack and share your ideas or ask us how to get started.
            </p>

            <p>
                        Still missing access to Slack? If you have registered your email on Cyber Republic,
              <br/>
                        send us an email at
              {' '}
              <a href="mailto:slack@elastos.org">slack@elastos.org</a>
              {' '}
and we will send an invite.
            </p>
          </Col>
        </Row>
      </div>
    )
  }

  renderNonDevelopers() {
    return (
      <div>
        <h3>
                Non-Developers
        </h3>

        <Row>
          <Col>
            <h4>
                        Join Discord and be part of the community!
            </h4>

            <p>
                        You don't have to be a developer to participate in the Elastos community, some examples of things
                        you can do even if you're not technical are helping us to find bugs, test our apps, writing articles, organizing events
                        or just being active in the community.
              <br/>
              <br/>
                        If your participation in the community becomes recognized you can let us know and if our existing organizers/staff
                        vouches for you, we'll send you ELA too.
            </p>
          </Col>
        </Row>
        <br/>
        <Row>
          <Col span={12} style={{paddingRight: '12px'}}>
            <h4>
                        Apply to be an Organizer
            </h4>

            <p>
                        We are always looking for organizers, especially in countries or communities that do not have one yet.
              <br/>
              <br/>
                        To see the current list of communities go to
              {' '}
              <a href="/community">the community page</a>
, even if there are organizers for your community
                        we can accept more than one in many cases. Or if there is a nearby community or smaller community such as a school, city or state that you
                        would like to represent, let us know at
              {' '}
              <a href="mailto:cyberrepublic@elastos.org">cyberrepublic@elastos.org</a>
.
            </p>

            <br/>

            <h4>
                        Elastos Non-Developer's Beginners Guide
            </h4>

            <p>
                        One of the best resources to start with is the Non-Developer's Beginners Guide
              <br/>
              <a target="_blank" href="https://github.com/elastos/Elastos/blob/master/NonDeveloperGuide/README.md">
                            https://github.com/elastos/Elastos/blob/master/NonDeveloperGuide/README.md
              </a>
            </p>
          </Col>
          <Col span={12}>
            <img src="/assets/help/non-developers-1.png" style={{width: '100%'}}/>
          </Col>
        </Row>
      </div>
    )
  }

  renderOrganizers() {
    return (
      <div>
        <h3>
                Organizers
        </h3>

        <Row>
          <Col className="center">
            <iframe width="560" height="315" src="https://www.youtube.com/embed/AXtElROGXzA" frameBorder="0" allow="autoplay; encrypted-media" allowFullScreen={true} />
          </Col>
        </Row>

        <br/>

        <Row>
          <Col span={12} style={{paddingRight: '12px'}}>
            <h4>
                        Apply to be an Organizer
            </h4>

            <p>
                        To become an organizer you must apply using the form found under each country in the
              {' '}
              <a href="/community">Community page</a>
.
              <br/>
              <br/>
                        After applying you can view the application under your Profile / Submissions, by viewing the submission you can also
                        see any comments by Elastos admins. You will also receive an email for any comments, so be sure to check your email.
            </p>
          </Col>
          <Col span={12}>
            <img src="/assets/help/non-developers-1.png" style={{width: '100%'}}/>
          </Col>
        </Row>
        <Row>
          <Col>
            <h4>
                        Getting Approval to Host a Meetup / Event
            </h4>

            <p>
                        Any events/tasks where you request funding or compensation of any sort need to be approved by
                        an Elastos admin. The video above describes this in better detail.
            </p>
          </Col>
        </Row>
      </div>
    )
  }

  renderEvents() {
    return (
      <div>
        <h3>
                Events / Tasks
        </h3>

        <Row>
          <Col span={16} style={{paddingRight: '12px'}}>
            <h4>
                        Creating
            </h4>

            <p>
                        Not to be confused with developer events/tasks which can only be created by Elastos admins, as an organizer
                        you can create tasks by going to your profile and clicking Tasks in the menu.
              <br/>
              <br/>
                        If you are an organizer you will see the
              {' '}
              <span style={{fontWeight: 600}}>Create Task</span>
              {' '}
button in the top right.
            </p>
          </Col>
          <Col span={8}>
            <img src="/assets/help/create-event-1.png" style={{width: '100%'}}/>
          </Col>
        </Row>
        <Row>
          <Col>
            <h4>Creating Tasks Tutorial</h4>

            <h5>Please see the following video for steps and information</h5>

            <iframe width="510" height="315" src="https://www.youtube.com/embed/AXtElROGXzA" frameBorder="0" allow="autoplay; encrypted-media" allowFullScreen={true} />
          </Col>
        </Row>
      </div>
    )
  }

  renderMeetupGuide() {
    return (
      <div>
        <h3>
                Meetup Guide
        </h3>

        <Row>
          <Col>
            <h4>
                        Elastos - First Time Meetup Guide
            </h4>

            <p>
                        The first meetup can be daunting if you're an organizer, thankfully a couple experienced
                        organizers have created this helpful guide on how to get started.
              <br/>
              <br/>
              <a target="_blank" href="https://docs.google.com/document/d/1f4fBvyXvNrdjxRMHAJTPMdPrKiUeozXsUt4TGxxLBnM">
                            https://docs.google.com/document/d/1f4fBvyXvNrdjxRMHAJTPMdPrKiUeozXsUt4TGxxLBnM
              </a>
            </p>
          </Col>
        </Row>
      </div>
    )
  }

  renderCommunity() {
    return (
      <div>
        <h3>
                Community
        </h3>

        <Row>
          <Col>
            <h4>
                        Joining a Community
            </h4>

            <p>
                        You will automatically be assigned to the country community that is defined in your profile, you can change this
                        at any time.
              <br/>
              <br/>
                        But you can join as many communities as you want,
            </p>
          </Col>
        </Row>

        <br/>

        <Row>
          <Col span={16}>
            <h4>
                        Contacting Members in Your Community
            </h4>

            <p>
                        You can directly contact other members by clicking their username in the community and sending them an email through Cyber Republic.
              <br/>
              <br/>
                        For privacy reasons the emails of other members is not accessible unless they reply to your message, when you send them a message
                        through Cyber Republic the reply-to address will be set to your email.
            </p>

            <p>
                        We encourage you to rather chat with your fellow community members in the corresponding channels in Slack setup for your community.
              <br/>
              <br/>
                        If one does not exist yet please contact one of the administrators in Slack and we'll create them for you.
            </p>
          </Col>
          <Col span={8}>
            <img src="/assets/help/community-1.png" style={{width: '100%'}}/>
            <br/>
            <br/>
            <img src="/assets/help/community-2.png" style={{width: '100%'}}/>
          </Col>
        </Row>
      </div>
    )
  }
}
