import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import { Menu, SubMenu } from 'antd'
import MediaQuery from 'react-responsive'
import I18N from '@/I18N'
import { Link } from 'react-router-dom'
import './style.scss'
import {MAX_WIDTH_MOBILE, MIN_WIDTH_PC} from '../../../../config/constant'

const ITEM_LIST = ['1', '2', '3', '4', '5', '6', '7', '8', '9', '10', 'A', 'B'];

export default class extends BaseComponent {

  ord_states() {
    return {
      navItem: 1
    }
  }

  handleMenuClick(item, key, keyPath) {
    this.props.history.replace({
      state: { id: item.key }
    })
  }

  ord_render () {
    const id = _.get(this.props, 'location.state.id', '1');

    // TODO check why we can not use redirect use this.props.history
    return (
      <div className="navigator">
        <MediaQuery minWidth={MIN_WIDTH_PC}>
          <Menu
            className="no-padding-items"
            defaultSelectedKeys={[id]}
            onClick={this.handleMenuClick.bind(this)}
            mode="inline"
          >
            {ITEM_LIST.map(key => (
              <Menu.Item key={key}>
                {I18N.get(`council.list.${key}`)}
              </Menu.Item>
            ))}
          </Menu>
        </MediaQuery>
        <MediaQuery maxWidth={MAX_WIDTH_MOBILE}>
          <Menu
            defaultSelectedKeys={[id]}
            onClick={this.handleMenuClick.bind(this)}
            mode="horizontal"
          >
            {ITEM_LIST.map(key => (
              <Menu.Item key={key}>
                {I18N.get(`concil.list.${key}`)}
              </Menu.Item>
            ))}
          </Menu>
        </MediaQuery>
      </div>
    )
  }
}
