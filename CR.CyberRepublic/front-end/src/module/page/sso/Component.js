import React from 'react'
import StandardPage from '../StandardPage';
import Footer from '@/module/layout/Footer/Container';
import { Spin } from 'antd';
import URI from 'urijs';

import './style.scss';

export default class extends StandardPage {
    ord_states() {
        return {
            loading: false
        };
    }

    ord_renderContent () {
        return (
            <div className="p_sso">
                <div className="ebp-header-divider">

                </div>
                <div className="ebp-page">
                    {
                        this.renderLoading()
                    }
                </div>
                <Footer />
            </div>
        )
    }
    renderLoading() {
        return (
            <div className="flex-center">
                <Spin size="large" />
            </div>
        )
    }

    async componentDidMount() {
        const params = new URI(this.props.location.search || '').search(true);
        const { SSO_URL, FORUM_URL } = process.env;
        let loginStr;

        this.setState({ loading: true });

        if (!params.sso || !params.sig) {
            loginStr = FORUM_URL;
        } else {
            try {
                const result = await this.props.getLoginStr(params);
                loginStr = SSO_URL + '?' + result.url;
                console.log('loginstr: ', loginStr, SSO_URL, FORUM_URL);
            } catch (error) {
                loginStr = FORUM_URL;
            }
        }
        window.location.href = loginStr;
    }
}
