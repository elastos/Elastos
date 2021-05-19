/* eslint-disable no-undef */
import React from 'react'
import {Helmet} from 'react-helmet'
import Footer from '@/module/layout/Footer/Container'
import I18N from '@/I18N'
import './style.scss'
import { Button, Spin, Icon } from 'antd'
import _ from 'lodash'
import StandardPage from '../../StandardPage'
import TelegramIcon from '@/module/common/TelegramIcon'

export default class extends StandardPage {
  componentDidMount() {
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

  linkRegister() {

    analytics.track('SIGN_UP_CLICKED', {
      buttonText: 'Join Us Now',
      url: location.href,
      link: `${location.origin}/login`
    })

    this.props.history.push('/login')
  }

  // TODO: what's up with these admin CSS classes?
  ord_renderContent () {
    return (
      <div className="p_crVideo">
        <div id="fb-root" />
        <Helmet>
          <script async={true} src="https://platform.twitter.com/widgets.js" charSet="utf-8" />
          <script>
            {`window.fbAsyncInit = function() {
                            FB.init({
                                appId            : '2317676415144125',
                                autoLogAppEvents : true,
                                xfbml            : true,
                                version          : 'v3.2'
                            });
                        }`}
          </script>
          <script>
            {`(function(d, s, id) {
                        var js, fjs = d.getElementsByTagName(s)[0];
                        if (d.getElementById(id)) return;
                        js = d.createElement(s); js.id = id;
                        js.src = 'https://connect.facebook.net/en_US/sdk.js#xfbml=1&version=v3.2';
                        fjs.parentNode.insertBefore(js, fjs);
                    }(document, 'script', 'facebook-jssdk'))`}
          </script>
          <script>
            {`
                        var tag = document.createElement('script');
                        tag.src = "https://www.youtube.com/iframe_api";
                        var firstScriptTag = document.getElementsByTagName('script')[0];
                        firstScriptTag.parentNode.insertBefore(tag, firstScriptTag);

                        var player;
                        function onYouTubeIframeAPIReady() {
                          player = new YT.Player('main_player', {
                            events: {
                              'onStateChange': onPlayerStateChange
                            }
                          });
                        }

                        function onPlayerStateChange(event) {
                          switch(event.data) {
                            case YT.PlayerState.PLAYING:
                              analytics.track('VIDEO_PLAYED', {
                                name: 'CR Promo - Nov 2018',
                                url: '${location.href}',
                                action: 'PLAY',
                                video: player.getVideoData().title,
                                id: player.getVideoData().video_id,
                                duration: player.getDuration()
                              })
                              break;
                            case YT.PlayerState.PAUSED:
                              analytics.track('VIDEO_PLAYED', {
                                name: 'CR Promo - Nov 2018',
                                url: '${location.href}',
                                action: 'PAUSE',
                                video: player.getVideoData().title,
                                id: player.getVideoData().video_id,
                                pausedAt: player.getCurrentTime()
                              })
                              break;
                            case YT.PlayerState.ENDED:
                              analytics.track('VIDEO_PLAYED', {
                                name: 'CR Promo - Nov 2018',
                                url: '${location.href}',
                                action: 'END',
                                video: player.getVideoData().title,
                                id: player.getVideoData().video_id,
                                duration: player.getDuration()
                              })
                              break;
                            default:
                              return;
                          }
                        }`}
          </script>
        </Helmet>
        <div className="ebp-header-divider" />
        <div className="p_admin_index ebp-wrap">
          <div className="d_box">
            <div className="p_admin_content">
              {this.buildHeader()}
              {this.buildContent()}
              {this.buildTasks()}
              <div className="share-icons">
                <span>SHARE</span>
                <div id="twitter_share" className="share-container" onClick={() => analytics.track('SOCIAL_SHARE_CLICKED', {
                  type: 'TWITTER',
                  url: location.href
                })}>
                  <a onClick={() => {
                    const win = window.open('https://twitter.com/share?ref_src=twsrc%5Etfw', 'ShareOnWitter', this.getWindowOptions())
                    win.opener = null
                  }}>
                    <Icon type="twitter" style={{ fontSize: 32}} />
                  </a>
                </div>
                <div id="facebook_share" className="share-container" onClick={() => analytics.track('SOCIAL_SHARE_CLICKED', {
                  type: 'FACEBOOK',
                  url: location.href
                })}>
                  <a onClick={() => {
                    FB.ui({
                      method: 'share',
                      display: 'popup',
                      href: 'https://www.facebook.com/sharer/sharer.php?u=https%3A%2F%2Fwww.facebook.com%2FElastosCyberRepublic%2F&amp;src=sdkpreparse'
                    }, (response) => { })
                  }}>
                    <Icon type="facebook" style={{ fontSize: 32}} />
                  </a>
                </div>
                <div className="share-container" onClick={() => analytics.track('SOCIAL_SHARE_CLICKED', {
                  type: 'TELEGRAM',
                  url: location.href
                })}>
                  <a href="https://t.me/elastosgroup" target="_blank">
                    <TelegramIcon style={{ fill: '#333333' }} />
                  </a>
                </div>
              </div>
            </div>
          </div>
        </div>
        <Footer/>
      </div>
    )
  }

  getWindowOptions() {
    const width = 500
    const height = 350
    const left = (window.innerWidth / 2) - (width / 2)
    const top = (window.innerHeight / 2) - (height / 2)

    return [
      'resizable,scrollbars,status',
      `height=${height}`,
      `width=${width}`,
      `left=${left}`,
      `top=${top}`
    ].join()
  }

  buildHeader() {
    return (
      <div className="header">
        <div className="background-box" />
        <div className="header-container">
          <div className="title komu-a">
            {I18N.get('cr-video.header.1')}
          </div>
          <div className="videoWrapper">
            <iframe id="main_player" src={`https://www.youtube.com/embed/vaPdh35elYc?enablejsapi=1&origin=${location.origin}`}
              frameBorder="0"
              allow="autoplay; encrypted-media;"
              allowFullScreen={true} />
          </div>
          {!this.props.is_login ? (
            <div>
              <Button className="earn-ela-btn" onClick={this.linkRegister.bind(this)}>{I18N.get('cr-video.join')}</Button>
            </div>
          ) : <div className="vertSpacer"/>}
          <div className="title sub-title">
            {I18N.get('cr-video.header.2')}
          </div>
          <div className="background-visuals">
            <img className="upper-right" src="/assets/images/quarter-circle-connected.svg"/>
            <img className="mid-right" src="/assets/images/training_circle.png"/>
            <img className="mid-left" src="/assets/images/training_green_slashed_box.png"/>
            <img className="upper-left" src="/assets/images/training_mini_connector.png"/>
          </div>
        </div>
      </div>
    )
  }

  buildContent() {
    return (
      <div>
        <div className="content">
          <div className="title">{I18N.get('cr-video.q6')}</div>
          <span>{this.props.is_login ? I18N.get('cr-video.q6.title.1_2') : I18N.get('cr-video.q6.title.1')}</span>
          {!this.props.is_login ? (
            <p>
              {I18N.get('cr-video.q6.paragraph.1')}
              <a target="_blank" onClick={this.linkRegister.bind(this)}>
                {' '}
                {I18N.get('cr-video.here')}
              </a>
.
            </p>
          ) : <p>{I18N.get('cr-video.q6.paragraph.1_2')}</p>}
          <span>{I18N.get('cr-video.q6.title.2')}</span>
          <p>{I18N.get('cr-video.q6.paragraph.2')}</p>
          <span>{I18N.get('cr-video.q6.title.3')}</span>
          <p>
            {I18N.get('cr-video.q6.paragraph.3_2')}
            <a target="_blank" href="https://discord.gg/UG9j6kh" onClick={() => analytics.track('DISCORD_CLICKED', {
              linkText: 'join the discord chat',
              url: location.href,
              link: 'https://discord.gg/UG9j6kh'
            })}>
              {I18N.get('cr-video.q6.paragraph.3.link')}
            </a>
            {I18N.get('cr-video.q6.paragraph.3_3')}
          </p>
          <span>{I18N.get('cr-video.q6.title.4')}</span>
          <p>
            {I18N.get('cr-video.q6.paragraph.4')}
            <a target="_blank" href="https://www.youtube.com/watch?v=D6lz889WyXQ" onClick={() => analytics.track('LINK_CLICKED', {
              linkText: 'earn ELA for your contribution',
              url: location.href,
              link: 'https://www.youtube.com/watch?v=D6lz889WyXQ'
            })}>
              {' '}
              {I18N.get('cr-video.q6.paragraph.4.link')}
            </a>
.
          </p>
          <span>{I18N.get('cr-video.q6.title.5')}</span>
          <p>{I18N.get('cr-video.q6.paragraph.5')}</p>
        </div>
        <div className="content">
          <div className="title">{I18N.get('cr-video.q1')}</div>
          <span>{I18N.get('cr-video.q1.title.1')}</span>
          <p>{I18N.get('cr-video.q1.paragraph.1')}</p>
          <span>{I18N.get('cr-video.q1.title.2')}</span>
          <p>{I18N.get('cr-video.q1.paragraph.2')}</p>
          <span>{I18N.get('cr-video.q1.title.3')}</span>
          <p>{I18N.get('cr-video.q1.paragraph.3')}</p>
        </div>
        <div className="content">
          <div className="title">{I18N.get('cr-video.q2')}</div>
          <p>{I18N.get('cr-video.q2.paragraph.1')}</p>
        </div>
        <div className="content">
          <div className="title">{I18N.get('cr-video.q3')}</div>
          <p>{I18N.get('cr-video.q3.paragraph.1')}</p>
          <div className="subtitle">{I18N.get('cr-video.q3.subtitle.1')}</div>
          <div className="links">
            <a target="_blank" onClick={() => analytics.track('WHITEPAPER_CLICKED', {
              url: location.href,
              link: 'https://www.elastos.org/wp-content/uploads/2018/White%20Papers/elastos_whitepaper_en.pdf?_t=1526235330',
              linkText: I18N.get('cr-video.q3.link.1')
            })} href="https://www.elastos.org/wp-content/uploads/2018/White%20Papers/elastos_whitepaper_en.pdf?_t=1526235330">
              {I18N.get('cr-video.q3.link.1')}
            </a>
            {' '}
|
            <a target="_blank" onClick={() => analytics.track('GUIDE_CLICKED', {
              url: location.href,
              name: 'Developer Guide to Elastos',
              link: 'https://github.com/elastos/Elastos/wiki/A-Developer-Guide-to-Elastos',
              linkText: I18N.get('cr-video.q3.link.2')
            })} href="https://github.com/elastos/Elastos/wiki/A-Developer-Guide-to-Elastos">
              {' '}
              {I18N.get('cr-video.q3.link.2')}
            </a>
            {' '}
|
            <a target="_blank" onClick={() => analytics.track('GUIDE_CLICKED', {
              url: location.href,
              name: 'Non-Developer Guide to Elastos',
              link: 'https://github.com/elastos/Elastos/wiki/A-Non-Developer-Guide-to-Elastos',
              linkText: I18N.get('cr-video.q3.link.3')
            })} href="https://github.com/elastos/Elastos/wiki/A-Non-Developer-Guide-to-Elastos">
              {' '}
              {I18N.get('cr-video.q3.link.3')}
            </a>
            {' '}
|
            <a target="_blank" onClick={() => analytics.track('ELASTOS.ORG_CLICKED', {
              url: location.href,
              link: 'http://Elastos.org/en',
              linkText: I18N.get('cr-video.q3.link.4')
            })} href="http://Elastos.org/en">
              {' '}
              {I18N.get('cr-video.q3.link.4')}
            </a>
          </div>
        </div>
        {!this.props.is_login && (
        <div>
          <Button className="earn-ela-btn" onClick={this.linkRegister.bind(this)}>{I18N.get('cr-video.join')}</Button>
        </div>
        )}
        <div className="content">
          <div className="title">{I18N.get('cr-video.q4')}</div>
          <p>{I18N.get('cr-video.q4.paragraph.1')}</p>
          <p>{I18N.get('cr-video.q4.paragraph.2')}</p>
          <p>{I18N.get('cr-video.q4.paragraph.3')}</p>
          <p>{I18N.get('cr-video.q4.paragraph.4')}</p>
          <p>{I18N.get('cr-video.q4.paragraph.5')}</p>
        </div>
        <div className="content">
          <div className="title">{I18N.get('cr-video.q5')}</div>
          <p>{I18N.get('cr-video.q5.paragraph.1')}</p>
        </div>
        <div className="content cr-learn-more">
          {/* Learn more about CR with these resources */}
          <div className="subtitle">{I18N.get('cr-video.q6.subtitle.1')}</div>
          <div className="links">
            <a target="_blank" onClick={() => analytics.track('VIDEO_CLICKED', {
              url: location.href,
              name: I18N.get('cr-video.q6.link.1'),
              video: 'https://www.youtube.com/watch?v=AXtElROGXzA'
            })} href="https://www.youtube.com/watch?v=AXtElROGXzA">
              {I18N.get('cr-video.q6.link.1')}
            </a>
            {' '}
|
            <a target="_blank" onClick={() => analytics.track('VIDEO_CLICKED', {
              url: location.href,
              name: I18N.get('cr-video.q6.link.2'),
              video: 'https://www.youtube.com/watch?v=90B2qzwOc8'
            })} href="https://www.youtube.com/watch?v=-90B2qzwOc8">
              {' '}
              {I18N.get('cr-video.q6.link.2')}
            </a>
          </div>
        </div>
        <div className="content email-contact">
          <div className="title center">{I18N.get('cr-video.q7.title')}</div>
          <p className="center">
            {I18N.get('cr-video.q7.subtitle')}
            {/*
                        <input id="email_mailer" type="text" size="32"/>
                        <Button className="earn-ela-btn" onClick={this.submitEmail.bind(this)}>{I18N.get('cr-video.q7.button_text')}</Button>
                        */}
          </p>
          <iframe width="0" height="0" border="0" name="dummyframe" id="dummyframe"/>
          <div className="form-wrap">
            <form id="footer-form" className="signup-form" name="mailing-list" onSubmit={() => analytics.track('EMAIL_NEWSLETTER', {
              action: 'entered input box',
              url: location.href,
            })} action="https://cyberrepublic.us19.list-manage.com/subscribe/post?u=acb5b0ce41bfe293d881da424&amp;id=8d3dc89cff" method="post">
              <div className="email-wrap">
                <input id="landing_footer_email" type="email" name="EMAIL" data-type="req" placeholder={I18N.get('landing.footer.email')} onClick={() => analytics.track('EMAIL_NEWSLETTER', {
                  action: 'submitted email',
                  emailValue: document.getElementById('landing_footer_email').val(),
                  url: location.href
                })}/>
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
      </div>
    )
  }

  buildTasks() {
    return (
      <div className="tasks" />
    )
  }

  submitEmail(email) {

  }
}
