import router from './router'
import data from './data'
import dict from './dict'

export default {
  data,
  router,
  ...process.env,
  dict,

  FORMAT: {
    DATE: 'MM/DD/YYYY',
    TIME: 'MM/DD/YYYY hh:mm:ss'
  }
}
