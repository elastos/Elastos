// MAKE SURE YOU RUN BUILD BEFORE THIS
// this should be run from the parent back-end folder, not scripts
// this is what sets the process.env
// node scripts/convert_suggestion_json_to_md.js
require('../dist/src/config')
const convertJsonToMd = require('./convert_json_to_md')

;(async () => {
  const db = await require('../dist/src/db').default
  const DB = await db.create()
  const db_sugg = DB.getModel('Suggestion')
  const sections = [
    'abstract',
    'goal',
    'motivation',
    'relevance',
    'budget',
    'plan'
  ]
  try {
    let docs = await db_sugg.find()
    console.log('docs.length', docs.length)
    let count = 0
    for (const doc of docs) {
      let obj = {}
      try {
        for (const section of sections) {
          if (doc[section]) {
            const md = convertJsonToMd(doc[section])
            if (md) {
              obj[section] = md
            }
          }
        }
        if (Object.keys(obj).length) {
          await db_sugg.update({ _id: doc._id }, { $set: obj })
        }
      } catch (err) {
        count = count + 1
        console.log(`suggestion ${doc._id} error`, err)
      }
    }
    console.log('total errors', count)
  } catch (err) {
    console.error(err)
    process.exit(1)
  }
  console.log('done!')
  process.exit(1)
})()
