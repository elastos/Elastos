import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import { Menu, SubMenu } from 'antd'
import MediaQuery from "react-responsive"
import I18N from '@/I18N'
import { Link } from 'react-router-dom';
import { Affix, Radio, Badge, Tooltip } from 'antd';
import './style.scss'
import {MAX_WIDTH_MOBILE, MIN_WIDTH_PC} from "../../../../config/constant"

export default class extends BaseComponent {

    ord_states() {
        return {
            navItem: 1
        };
    }

    handleMenuClick(item, key, keyPath) {
        const lookup = {
            communities: '/admin/community'
        }

        const route = lookup[item.key]
        route && this.props.history.push(route)
    }

    ord_render () {
        // TODO check why we can not use redirect use this.props.history
        return (
            <div className="navigator">
                <MediaQuery minWidth={MIN_WIDTH_PC}>
                    <Menu
                        className="no-padding-items"
                        defaultSelectedKeys={[this.props.selectedItem]}
                        onClick={this.handleMenuClick.bind(this)}
                        mode="inline"
                    >
                        <Menu.Item key="">
                            {I18N.get('counstitution.menu1')}
                        </Menu.Item>
                        <Menu.Item key="">
                            {I18N.get('counstitution.menu2')}
                        </Menu.Item>
                        <Menu.Item key="">
                            {I18N.get('counstitution.menu3')}
                        </Menu.Item>
                        <Menu.Item key="">
                            {I18N.get('counstitution.menu4')}
                        </Menu.Item>
                    </Menu>
                </MediaQuery>
                <MediaQuery maxWidth={MAX_WIDTH_MOBILE}>
                    <Menu
                        defaultSelectedKeys={[this.props.selectedItem]}
                        onClick={this.handleMenuClick.bind(this)}
                        mode="horizontal"
                    >
                        <Menu.Item key="">
                            {I18N.get('counstitution.menu1')}
                        </Menu.Item>
                        <Menu.Item key="">
                            {I18N.get('counstitution.menu2')}
                        </Menu.Item>
                        <Menu.Item key="">
                            {I18N.get('counstitution.menu3')}
                        </Menu.Item>
                        <Menu.Item key="">
                            {I18N.get('counstitution.menu4')}
                        </Menu.Item>
                    </Menu>
                </MediaQuery>
            </div>
        )
    }
}
