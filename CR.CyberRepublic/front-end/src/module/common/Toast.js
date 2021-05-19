import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import {
  Icon,
} from 'antd'
import _ from 'lodash'
import styled from 'styled-components'
import { text, bg } from '@/constants/color'


export default class extends BaseComponent {
  constructor(props) {
    super()

    this.state = {
      dismissed: props.defaultDismiss || false,
    }
  }

  ord_render() {
    return this.renderToast()
  }


  dismissToast = () => {
    this.setState({
      dismissed: true,
    })

    localStorage.setItem([this.props.storageKey], 'true')
  }

  isPermanentlyDismissed() {
    return localStorage.getItem(this.props.storageKey) === 'true'
  }

  renderToast() {
    if (!_.has(this.props, 'storageKey')) {
      console.error('storegeKey in props is needed')
      return null
    }
    const isShow = !this.state.dismissed
      && !this.isPermanentlyDismissed()

    return isShow && (
      <Container>
        {/* <a onClick={this.completeProfile}> */}
          {this.props.message}
          {/* <Icon type="right" style={{ marginLeft: 8 }} /> */}
        {/* </a> */}
        <CloseIcon onClick={this.dismissToast}>
          <Icon type="close" />
        </CloseIcon>
      </Container>
    )
  }
}

const Container = styled.div`
  background: ${bg.primaryDark};
  color: ${text.primary};
  height: 48px;
  line-height: 48px;
  text-align: center;
`

const CloseIcon = styled.a`
  float: right;
  margin-right: 16px;
  font-size: 18px;
`
