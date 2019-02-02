import BaseService from '../model/BaseService'
import _ from 'lodash'
import { api_request } from '@/util'

export default class extends BaseService {
    constructor() {
        super()
        this.selfRedux = this.store.getRedux('suggestion')
    }
    async saveFilter(filter) {
        this.dispatch(this.selfRedux.actions.filter_update(filter))
    }

    async loadMore(qry) {
        this.list(qry)
    }

    async list(qry) {

        this.dispatch(this.selfRedux.actions.loading_update(true))

        const path = '/api/suggestion/list'
        this.abortFetch(path)

        let result
        try {
            result = await api_request({
                path,
                method: 'get',
                data: qry,
                signal: this.getAbortSignal(path)
            })

            this.dispatch(this.selfRedux.actions.loading_update(false))
            this.dispatch(this.selfRedux.actions.all_suggestions_reset())
            this.dispatch(this.selfRedux.actions.all_suggestions_total_update(result.total))
            this.dispatch(this.selfRedux.actions.all_suggestions_update(_.values(result.list)))
        } catch (e) {
            // Do nothing
        }

        return result
    }
    async myList(qry) {

        this.dispatch(this.selfRedux.actions.my_suggestions_loading_update(true))

        const path = '/api/suggestion/list'
        // this.abortFetch(path)

        let result
        try {
            result = await api_request({
                path,
                method: 'get',
                data: qry,
                signal: this.getAbortSignal(path)
            })

            this.dispatch(this.selfRedux.actions.my_suggestions_loading_update(false))
            this.dispatch(this.selfRedux.actions.my_suggestions_reset())
            this.dispatch(this.selfRedux.actions.my_suggestions_total_update(result.total))
            this.dispatch(this.selfRedux.actions.my_suggestions_update(_.values(result.list)))
        } catch (e) {
            // Do nothing
        }

        return result
    }
    resetAll() {
        this.dispatch(this.selfRedux.actions.all_suggestions_reset())
    }
    resetMyList() {
        this.dispatch(this.selfRedux.actions.my_suggestions_reset())
    }
    resetDetail() {
        this.dispatch(this.selfRedux.actions.detail_reset())
    }

    async create(doc) {
        this.dispatch(this.selfRedux.actions.loading_update(true))

        const res = await api_request({
            path: '/api/suggestion/create',
            method: 'post',
            data: doc
        })

        this.dispatch(this.selfRedux.actions.loading_update(false))

        return res
    }
    async getDetail({ id, incViewsNum }) {
        this.dispatch(this.selfRedux.actions.loading_update(true))

        const result = await api_request({
            path: `/api/suggestion/${id}/show`,
            method: 'get',
            data: { incViewsNum }
        })

        this.dispatch(this.selfRedux.actions.loading_update(false))
        this.dispatch(this.selfRedux.actions.detail_update(result))

        return result
    }

    async like(id) {
        const res = await api_request({
            path: `/api/suggestion/${id}/like`,
            method: 'post'
        })

        return res
    }
    async dislike(id) {
        const res = await api_request({
            path: `/api/suggestion/${id}/dislike`,
            method: 'post'
        })

        return res
    }
    async reportAbuse(id) {
        const res = await api_request({
            path: `/api/suggestion/${id}/reportabuse`,
            method: 'post'
        })

        return res
    }
    // ADMIN ONLY
    async abuse(id) {
        const res = await api_request({
            path: `/api/suggestion/${id}/abuse`,
            method: 'post'
        })

        return res
    }
    // ADMIN ONLY
    async archive(id) {
        const res = await api_request({
            path: `/api/suggestion/${id}/archive`,
            method: 'post'
        })

        return res
    }
    // ADMIN ONLY
    async delete(id) {
        const res = await api_request({
            path: `/api/suggestion/${id}/delete`,
            method: 'post'
        })

        return res
    }
}
