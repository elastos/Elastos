
// MAKE SURE YOU RUN BUILD BEFORE THIS

// this is what sets the process.env
const _ = require('lodash')
require('../dist/src/config');

(async () => {

    const db = await require('../dist/src/db').default
    const DB = await db.create()

    const db_elip = DB.getModel('Elip')

    try {
      const docs = await db_elip.find()
      console.log('docs.length', docs.length)
      for (const doc of docs) {
        if (doc.vid >= 100) {
          const obj = _.update(doc, 'vid', value => value - 100 + 3)
          const rs = await db_elip.update(
            { _id: doc._id },
            { $set: obj }
          )
          if(rs.n !== 1 || rs.nModified !== 1) {
            console.log("Warning! Update expection.", rs)
          }
          console.log("Modified.")
        }
      }
      const resetResult = await db_elip.getDBInstance().resetCount()
      console.log('reset count ', resetResult)
    } catch (err) {
        console.error(err)
    }

    process.exit(1)
})()