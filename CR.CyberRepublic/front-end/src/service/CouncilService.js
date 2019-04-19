import BaseService from '../model/BaseService'

const councilTabKeys = ['COUNCIL', 'SECRETARIAT']

export default class extends BaseService {
  async changeTab(tabKey) {
    if (councilTabKeys.indexOf(tabKey) < 0) {
      throw new Error(`Council tab "${tabKey}" unrecognizeed`)
    }

    const councilRedux = this.store.getRedux('council')

    await this.dispatch(councilRedux.actions.tab_update(tabKey))
  }
}
