// MAKE SURE YOU RUN BUILD BEFORE THIS
// this should be run from the parent back-end folder, not scripts
// this is what sets the process.env
require('../dist/src/config')
const _ = require('lodash')

;(async () => {
  const db = await require('../dist/src/db').default
  const DB = await db.create()
  const db_user = DB.getModel('User')
  try {
    let users = await db_user.find()
    console.log('users.length', users.length)
    for (const user of users) {
      try {
        if (user.dids) {
          const did = user.dids.find(el => el.active === true)
          if (_.get(did, 'id')) {
            console.log('user did email', user.email)
            const data = { id: did.id }
            await db_user.update(
              { _id: user._id },
              { $set: { did: data }, $unset: { dids: [] } }
            )
          } else {
            await db_user.update(
              { _id: user._id },
              { $unset: { dids: [] } }
            )
          }
        }
      } catch (err) {
        console.log(`user ${user.email} error`, err)
      }
    }
  } catch (err) {
    console.error(err)
  }
  process.exit(1)
})()
