require('../dist/src/config')

const _ = require('lodash')
const UserService = require('../dist/src/service/UserService')

;(async () => {
  const db = await require('../dist/src/db').default
  const DB = await db.create()
  const db_user = DB.getModel('User')
  try {
    const userService = new UserService.default(DB, {
      user: undefined
    })
    for (let i = 1; i < 13; i++) {
      let username = i < 10 ? 'test_cr0' + i : 'test_cr' + i
      const user = await userService.registerNewUser({
        username,
        email: username + '@gmail.com',
        password: process.env.MEMBER_PASSWORD
      })
      await db_user.update({ _id: user._id }, { $set: { role: 'COUNCIL' } })
    }
    const user = await userService.registerNewUser({
      username: 'secretary',
      email: 'secretary@gmail.com',
      password: process.env.MEMBER_PASSWORD
    })
    await db_user.update({ _id: user._id }, { $set: { role: 'SECRETARY' } })
  } catch (err) {
    console.error('err...', err)
  }

  process.exit(1)
})()
