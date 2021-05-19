// MAKE SURE YOU RUN BUILD BEFORE THIS
// this should be run from the parent back-end folder, not scripts
// this is what sets the process.env
// node scripts/separate_suggestion_edit_history.js

require('../dist/src/config')
const _ = require('lodash')

;(async () => {
  const db = await require('../dist/src/db').default
  const DB = await db.create()
  const db_suggestion = DB.getModel('Suggestion')
  const db_sugg_edit_history = DB.getModel('Suggestion_Edit_History')
  try {
    let suggestions = await db_suggestion.find()
    for (let suggestion of suggestions) {
      if (_.isEmpty(suggestion.editHistory)) {
        console.log('no edit history', suggestion._id)
        continue
      }
      let promises = []
      for (let history of suggestion.editHistory) {
        promises.push(
          db_sugg_edit_history.save({
            ...history,
            createdAt: suggestion.createdAt,
            suggestion: suggestion._id
          })
        )
      }
      try {
        await Promise.all(promises)
        await db_suggestion.update(
          { _id: suggestion._id },
          { $set: { editHistory: [] } }
        )
      } catch (err) {
        console.log('cannot handel suggestion', suggestion._id)
        console.log(err)
        process.exit(1)
      }
    }
  } catch (err) {
    console.error(err)
    process.exit(1)
  }
  console.log('done!')
  process.exit(1)
})()
