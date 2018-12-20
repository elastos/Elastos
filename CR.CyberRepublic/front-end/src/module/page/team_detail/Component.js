import React from 'react'
import StandardPage from '../StandardPage'
import Footer from '@/module/layout/Footer/Container'
import TeamDetail from './detail/Container'
import I18N from '@/I18N'
import './style.scss'
import _ from 'lodash'
import { Row, Breadcrumb, Icon } from 'antd'

export default class extends StandardPage {
    ord_renderContent () {
        return (
            <div className="p_TeamDetail">
                <div className="ebp-header-divider" />
                <div className="p_admin_index ebp-wrap">
                    <div className="d_box">
                        <div className="p_admin_content">
                            <TeamDetail teamId={this.props.match.params.teamId}/>
                        </div>
                    </div>
                </div>
                <Footer/>
            </div>
        )
    }
}
