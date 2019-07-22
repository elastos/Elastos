import _ from 'lodash'

export default {
  formatUsername(user) {
    if (!user) return ''
    const firstName = user.profile && user.profile.firstName
    const lastName = user.profile && user.profile.lastName

    if (_.isEmpty(firstName) && _.isEmpty(lastName)) {
      return user.username
    }

    return [firstName, lastName].join(' ')
  },
}
