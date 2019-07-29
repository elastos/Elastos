/* global location, analytics */
import React from 'react'
import BasePage from '@/model/BasePage'
import { Layout, BackTop } from 'antd'
import { spring, presets, Motion } from 'react-motion'
import { StickyContainer } from 'react-sticky'
import Meta from '@/module/common/Meta'
import Header from '../layout/Header/Container'
import MobileMenu from './mobile/side_menu/Container'

export default class extends BasePage {
  constructor(props) {
    super(props)

    this.state = {
      showMobile: false
    }

    analytics.page(location.pathname)
  }

  toggleMobileMenu() {
    this.setState({
      showMobile: !this.state.showMobile
    })
  }

  ord_renderPage() {
    const s = this.ord_animate()
    const mp = {
      defaultStyle: {
        left: 100
      },
      style: {
        left: spring(20, presets.noWobble)
      }
    }

    return (
      <StickyContainer>
        <Layout className="p_standardPage">
          {this.ord_renderMeta() && <Meta />}
          {this.state.showMobile && (
            <Motion {...mp}>
              {tar => {
                return (
                  <MobileMenu
                    animateStyle={s.style_fn(tar)}
                    toggleMobileMenu={this.toggleMobileMenu.bind(this)}
                  />
                )
              }}
            </Motion>
          )}
          <Header toggleMobileMenu={this.toggleMobileMenu.bind(this)} />
          <Layout.Content className="c_Content">
            {this.ord_renderContent()}
          </Layout.Content>
          <BackTop />
        </Layout>
      </StickyContainer>
    )
  }

  ord_animate() {
    // the width of the menu is 80vw
    return {
      style_fn: val => {
        return {
          left: `${val.left}vw`
        }
      }
    }
  }

  ord_renderContent() {
    return null
  }

  ord_renderMeta(f = true) {
    const { match } = this.props
    const flag = match && match.path === '/suggestion/:id'
    return flag ? false : f
  }

  ord_loading(f = false) {
    this.setState({ loading: f })
  }
}
