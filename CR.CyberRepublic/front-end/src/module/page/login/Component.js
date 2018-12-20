import React from 'react';
import StandardPage from '../StandardPage';
import LoginOrRegisterForm from '@/module/form/LoginOrRegisterForm/Container';

import './style.scss'

export default class extends StandardPage {
    ord_renderContent() {
        return (
            <div className="p_login ebp-wrap">
                <div className="d_box">
                    <div className="side-image">
                        <img src="/assets/images/login-left-extended.svg"/>
                    </div>
                    <div className="side-form">
                        <LoginOrRegisterForm />
                    </div>
                </div>
            </div>
        );
    }

    ord_checkLogin(isLogin) {
        if (isLogin) {
            this.props.history.replace('/profile/teams');
        }
    }
}
