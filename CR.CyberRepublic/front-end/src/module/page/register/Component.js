import React from 'react';
import StandardPage from '../StandardPage';
import RegisterForm from '@/module/form/RegisterForm/Container';

import './style.scss'

export default class extends StandardPage {
    ord_renderContent() {
        return (
            <div class="p_Register">
                <div className="ebp-header-divider">

                </div>
                <div className="ebp-wrap">
                    <div className="d_box">
                        <RegisterForm />
                    </div>
                </div>
                <br/>
                <br/>
            </div>
        );
    }
}
