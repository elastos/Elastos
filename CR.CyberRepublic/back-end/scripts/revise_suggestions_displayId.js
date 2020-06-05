
// MAKE SURE YOU RUN BUILD BEFORE THIS

// this is what sets the process.env
const _ = require('lodash')
require('../dist/src/config');

(async () => {

    const db = await require('../dist/src/db').default
    const DB = await db.create()

    const db_suggestions = DB.getModel('Suggestion')

    try {
      const resetResult = await db_suggestions.getDBInstance().resetCount()
      console.log('reset count ', resetResult)
    } catch (err) {
        console.error(err)
    }

    process.exit(1)
})()