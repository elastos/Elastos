import React from 'react';
import StandardPage from '../../StandardPage';
import CVoteForm from '@/module/form/CVoteForm/Container';
import {Breadcrumb, Icon} from 'antd';

import './style.scss'

export default class extends StandardPage {
    ord_renderContent(){
        return (
            <div className="p-cvote">
                <div className="d_box">
                    <CVoteForm />
                </div>
            </div>
        );
    }

    ord_checkLogin(isLogin){
        if(!isLogin){
            this.props.history.replace('/login');
        }
    }

    goList(){
        this.props.history.push('/council');
    }

}
