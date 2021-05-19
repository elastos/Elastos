import BaseService from '../model/BaseService'
import I18N from '@/I18N'

export default class extends BaseService {
  changeLanguage(lang) {
    I18N.setLang(lang)

    const languageRedux = this.store.getRedux('language')
    localStorage.setItem('lang', lang)
    this.dispatch(languageRedux.actions.language_update(lang))
  }
}
