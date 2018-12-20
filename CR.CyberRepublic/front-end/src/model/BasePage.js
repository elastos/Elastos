import React from 'react'
import BaseComponent from './BaseComponent'
import {spring, Motion} from 'react-motion'
import _ from 'lodash'
import store from '@/store'

/**
 noWobble: {stiffness: 170, damping: 26}, // the default, if nothing provided
 gentle: {stiffness: 120, damping: 14},
 wobbly: {stiffness: 180, damping: 12},
 stiff: {stiffness: 210, damping: 20},
 */
const springConfig = {stiffness: 180, damping: 14}
export default class extends BaseComponent {
    ord_render(p) {

        /*
        const s = this.ord_animate()
        const defaultStyle = {}
        _.each(s.from, (v, i)=>{
            defaultStyle[`value_${i}`] = v
        })
        const toStyle = {}
        _.each(s.to, (v, i)=>{
            toStyle[`value_${i}`] = spring(v, springConfig)
        })

        const mp = {
            defaultStyle,
            style : toStyle
        }

        return (
            <Motion {...mp}>
                {
                    (tar)=>{
                        return (<div style={s.style_fn(_.values(tar))}>{this.ord_renderPage(p)}</div>)
                    }
                }
            </Motion>
        )
        */

        return (<div>{this.ord_renderPage(p)}</div>)
    }

    ord_animate() {
        return {
            from: [0, 50],
            to: [1, 0],
            style_fn: (values) => {
                return {
                    position: 'relative',
                    opacity: values[0],
                    left: values[1]
                }
            }
        }
    }

    ord_renderPage() {
        return null
    }

    componentDidMount() {
        const storeUser = store.getState().user

        if (!storeUser) {
            return
        }
        const is_login = storeUser.is_login
        const is_admin = storeUser.is_admin

        this.ord_checkLogin(is_login, is_admin)
    }

    ord_checkLogin() {
    }

    $getParam(key){
        return key ? this.props.match.params[key] : this.props.match.params[key];
    }
}
