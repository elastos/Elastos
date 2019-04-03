import URI from 'urijs'
import _ from 'lodash'

export const loginRedirectWithQuery = ({ query, url }) => {
  const uri = URI(url || window.location.href)
  if (_.isObject(query)) uri.addQuery(query)
  sessionStorage.setItem('loginRedirect', uri.resource())
}

export default {
  loginRedirectWithQuery,
}
