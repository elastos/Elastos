import BaseRedux from '@/model/BaseRedux'
import I18N from '@/I18N'

class LanguageRedux extends BaseRedux {
  defineTypes() {
    return ['language']
  }

  defineDefaultState() {
    return {
      language: I18N.getLang(),
    }
  }
}

export default new LanguageRedux()
