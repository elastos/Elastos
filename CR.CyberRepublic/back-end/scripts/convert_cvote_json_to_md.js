// MAKE SURE YOU RUN BUILD BEFORE THIS
// this should be run from the parent back-end folder, not scripts
// this is what sets the process.env
// node scripts/convert_cvote_json_to_md.js
require('../dist/src/config')
const convertJsonToMd = require('./convert_json_to_md')

;(async () => {
  const db = await require('../dist/src/db').default
  const DB = await db.create()
  const db_cvote = DB.getModel('CVote')
  const sections = [
    'abstract',
    'goal',
    'motivation',
    'relevance',
    'budget',
    'plan'
  ]
  try {
    let docs = await db_cvote.find()
    console.log('docs.length', docs.length)
    let count = 0
    for (const doc of docs) {
      try {
        let obj = {}
        for (const section of sections) {
          if (doc[section]) {
            const md = convertJsonToMd(doc[section])
            if (md) {
              obj[section] = md
            }
          }
        }
        if (Object.keys(obj).length) {
          await db_cvote.update({ _id: doc._id }, { $set: obj })
        }
      } catch (err) {
        count = count + 1
        console.log(`cvote ${doc._id} error`, err)
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
