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
  getUserDisplayName(user) {
    const firstName = _.get(user, 'profile.firstName', '')
    const lastName = _.get(user, 'profile.lastName', '')
    const username = _.get(user, 'username', '');
    const email = _.get(user, 'email', '');

    return _.trim(`${firstName} ${lastName}`) ||
      _.trim(username) ||
      _.trim(email)
  }
}
