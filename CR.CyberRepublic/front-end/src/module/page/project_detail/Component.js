import React from 'react'
import StandardPage from '../StandardPage'
import Footer from '@/module/layout/Footer/Container'
import ProjectDetail from './detail/Container'
import I18N from '@/I18N'
import './style.scss'
import _ from 'lodash'
import { Row, Breadcrumb, Icon } from 'antd'

export default class extends StandardPage {
    ord_renderContent () {
        return (
            <div className="p_ProjectDetail">
                <div className="ebp-header-divider" />
                <div>
                    <div className="d_box">
                        <div className="p_admin_content">
                            <ProjectDetail taskId={this.props.match.params.taskId}/>
                        </div>
                    </div>
                </div>
                <Footer/>
            </div>
        )
    }
}
