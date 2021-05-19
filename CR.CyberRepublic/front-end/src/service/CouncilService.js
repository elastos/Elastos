import BaseService from '../model/BaseService'
import { api_request } from '@/util'

const councilTabKeys = ['COUNCIL', 'SECRETARIAT', 'VOTING']

export default class extends BaseService {
  async changeTab(tabKey) {
    if (councilTabKeys.indexOf(tabKey) < 0) {
      throw new Error(`Council tab "${tabKey}" unrecognizeed`)
    }

    const councilRedux = this.store.getRedux('council')

    await this.dispatch(councilRedux.actions.tab_update(tabKey))
  }


  async getCouncilMembers() {
    const selfStore = this.store.getRedux('council')
    await this.dispatch(selfStore.actions.council_members_loading_update(true))

    const path = '/api/user/getCouncilMembers'
    this.abortFetch(path)

    let result
    try {
      result = await api_request({
        path,
        method: 'get',
        signal: this.getAbortSignal(path),
      })

      await this.dispatch(selfStore.actions.council_members_update(result.list))
      await this.dispatch(selfStore.actions.council_members_loading_update(false))
    } catch (e) {
      // Do nothing
    }

    return result
  }

  async getCandidates(param) {
    const path = '/api/cvote/listcrcandidates'

    const rs = await api_request({
      path,
      method: 'post',
      data: param
    })
    return JSON.parse(rs).data.result
  }

  async getCouncilsAndSecretariat() {
    const path = '/api/council/council_secretariat'

    const rs = await api_request({
      path,
      method: 'get'
    })
    
    return rs
  }
}
