import _ from 'lodash'
import en from './en'
import zh from './zh'


const all = _.extend({}, {
  zh,
  en,
})

let lang = localStorage.getItem('lang') || 'en'
export default {
  setLang(str) {
    if (_.includes(['zh', 'en'], str)) {
      lang = str
    } else {
      throw new Error(`invalid lang : ${str}`)
    }
  },
  getLang() {
    return lang
  },

  get(key) {
    return _.get(all[lang], key, _.get(all.en, key, key))
  },
}
