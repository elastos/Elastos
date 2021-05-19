// MAKE SURE YOU RUN BUILD BEFORE THIS
// this should be run from the parent back-end folder, not scripts
// this is what sets the process.env
require('../dist/src/config')

const _ = require('lodash')
const UserService = require('../dist/src/service/UserService')
const letters = 'a b c d e f g h i j k l m n o p q r s t u v w x y z'.split(' ')

;(async () => {
  const db = await require('../dist/src/db').default
  const DB = await db.create()
  try {
    for (let i = 0; i < letters.length; i++) {
      const userService = new UserService.default(DB, {
        user: undefined
      })
      await userService.registerNewUser({
        username: letters[i] + '_test',
        email: letters[i] + 'test@gmail.com',
        password: process.env.MEMBER_PASSWORD
      })
    }
  } catch (err) {
    console.error(err)
  }

  process.exit(1)
})()
