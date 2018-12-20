import React from 'react'
import StandardPage from '../../StandardPage'
import I18N from '@/I18N'
import './style.scss'
import { Col, Row, Card, Button, Breadcrumb, Icon, List, Spin, Avatar, Modal } from 'antd'
import _ from 'lodash'

export default class extends StandardPage {

    ord_renderContent(){
        return (
            <div className="p_council">
                <div className="ebp-header-divider"></div>
                <div className="p_admin_index ebp-wrap">
                    <div className="d_box">
                        <div className="p_content">
                            {this.renderTitle()}
                            {this.renderContent()}
                        </div>
                    </div>
                </div>
            </div>
        );
    }

    renderTitle(){
        const id = this.props.id;
        const title = I18N.get('council.list.'+id);
        return (
            <div>
                <h2 style={{paddingBottom:0}}>{title}</h2>
                <h3 style={{paddingTop:0}}>{I18N.get('council.0003')}</h3>
            </div>
            
        );
    }

    renderContent(){
        const id = this.props.id;
        const content = I18N.get('council.article.'+id);
        return (
            <p className="f_box" dangerouslySetInnerHTML={{__html : content}}>
                
            </p>
        );
    }
    
}
