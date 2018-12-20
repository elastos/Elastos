import BaseService from '../model/BaseService'
import _ from 'lodash'
import {api_request} from '@/util'


export default class extends BaseService {
    async postComment(type, reduxType, detailReducer, returnUrl, id, commentData, headline) {
        const redux = this.store.getRedux(reduxType || type)
        const data = {
            comment: commentData,
            headline,
            createdBy: this.store.getState().user,
            createdAt: new Date().toISOString(),
            returnUrl
        }

        this.dispatch(redux.actions.loading_update(true))

        const rs = await api_request({
            path: `/api/${type}/${id}/comment`,
            method: 'post',
            data
        })
        const curDetail = this.store.getState()[reduxType || type] && this.store.getState()[reduxType || type].detail;

        if (!curDetail) {
            return;
        }

        let subDetail = curDetail
        if (detailReducer) {
            subDetail = detailReducer(curDetail)
        }

        subDetail.comments = subDetail.comments || [];
        subDetail.comments.push([data]);

        subDetail.subscribers = subDetail.subscribers || [];
        subDetail.subscribers.push({
            user: {
                ...this.store.getState().user,
                _id: this.store.getState().user.current_user_id
            },
            lastSeen: new Date()
        })

        this.dispatch(redux.actions.detail_update(curDetail))
        this.dispatch(redux.actions.loading_update(false))

        return rs
    }

    async subscribe(type, id, reduxType) {
        reduxType = reduxType || type
        const redux = this.store.getRedux(reduxType)

        this.dispatch(redux.actions.subscribing_update
            ? redux.actions.subscribing_update(true)
            : redux.actions.loading_update(true))

        const rs = await api_request({
            path: `/api/${type}/${id}/subscribe`,
            method: 'post',
            data: {}
        })
        const curDetail = this.store.getState()[reduxType] && this.store.getState()[reduxType].detail

        if (!curDetail) {
            return;
        }

        curDetail.subscribers = curDetail.subscribers || []
        curDetail.subscribers.push({
            user: {
                ...this.store.getState().user,
                _id: this.store.getState().user.current_user_id
            },
            lastSeen: new Date()
        })

        this.dispatch(redux.actions.detail_update(curDetail))
        this.dispatch(redux.actions.subscribing_update
            ? redux.actions.subscribing_update(false)
            : redux.actions.loading_update(false))

        return rs
    }

    async unsubscribe(type, id, reduxType) {
        reduxType = reduxType || type
        const redux = this.store.getRedux(reduxType)

        this.dispatch(redux.actions.subscribing_update
            ? redux.actions.subscribing_update(true)
            : redux.actions.loading_update(true))

        const rs = await api_request({
            path: `/api/${type}/${id}/unsubscribe`,
            method: 'post',
            data: {}
        })
        const curDetail = this.store.getState()[reduxType] && this.store.getState()[reduxType].detail

        if (!curDetail) {
            return;
        }

        curDetail.subscribers = curDetail.subscribers || []
        curDetail.subscribers = _.filter(curDetail.subscribers, (subscriber) => {
            return subscriber.user && subscriber.user._id.toString() !==
                this.store.getState().user.current_user_id.toString()
        })

        this.dispatch(redux.actions.detail_update(curDetail))
        this.dispatch(redux.actions.subscribing_update
            ? redux.actions.subscribing_update(false)
            : redux.actions.loading_update(false))

        return rs
    }
}
