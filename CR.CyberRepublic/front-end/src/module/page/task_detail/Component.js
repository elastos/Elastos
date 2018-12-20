import React from 'react';
import StandardPage from '../StandardPage';
import I18N from '@/I18N'
import Footer from '@/module/layout/Footer/Container'

// TODO: this is backwards and confusing
import TaskDetail from '@/module/task/Container'

import _ from 'lodash'
import './style.scss'
import { Link } from 'react-router-dom'
import { Breadcrumb, Icon } from 'antd'

import {TASK_CATEGORY} from '@/constant'

export default class extends StandardPage {

    componentDidMount() {
        const taskId = this.props.match.params.taskId
        this.props.getTaskDetail(taskId)
    }

    componentWillUnmount() {
        this.props.resetTaskDetail()
    }

    ord_renderContent () {
        return (
            <div className="p_TaskDetail">
                <div className="ebp-header-divider">

                </div>
                <div className="ebp-page">
                    <TaskDetail task={this.props.task}/>
                </div>
                <Footer/>
            </div>
        )
    }
}
