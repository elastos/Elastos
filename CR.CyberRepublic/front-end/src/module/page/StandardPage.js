/*global location, analytics*/
import React from 'react';
import BasePage from '@/model/BasePage';
import {Layout} from 'antd';
import Header from '../layout/Header/Container';
import {BackTop} from 'antd'
import MobileMenu from './mobile/side_menu/Container';
import {spring, presets, Motion} from 'react-motion'

export default class extends BasePage {

    constructor(props) {
        super(props)

        this.state = {
            showMobile: false
        }

        analytics.page(location.pathname)
    }

    toggleMobileMenu() {
        this.setState({
            showMobile: !this.state.showMobile
        })
    }

    ord_renderPage() {

        const s = this.ord_animate()
        const mp = {
            defaultStyle: {
                left: 100
            },
            style : {
                left: spring(20, presets.noWobble)
            }
        }

        return (
            <Layout className="p_standardPage">
                {this.state.showMobile &&
                <Motion {...mp}>
                    {
                        (tar) => {
                            return <MobileMenu animateStyle={s.style_fn(tar)} toggleMobileMenu={this.toggleMobileMenu.bind(this)}/>
                        }
                    }
                </Motion>
                }
                <Header toggleMobileMenu={this.toggleMobileMenu.bind(this)}/>
                <Layout.Content>
                    {this.ord_renderContent()}
                </Layout.Content>
                <BackTop/>
            </Layout>
        );
    }

    ord_animate() {

        // the width of the menu is 80vw
        return {
            style_fn: (val) => {
                return {
                    left: val.left + 'vw'
                }
            }
        }
    }

    ord_renderContent() {
        return null;
    }

    ord_loading(f=false){
        this.setState({loading : f});
    }
}
