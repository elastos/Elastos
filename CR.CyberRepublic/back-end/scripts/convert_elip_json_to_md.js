// MAKE SURE YOU RUN BUILD BEFORE THIS
// this should be run from the parent back-end folder, not scripts
// this is what sets the process.env
// node scripts/convert_elip_json_to_md.js

require('../dist/src/config')
const convertJsonToMd = require('./convert_json_to_md')

;(async () => {
  const db = await require('../dist/src/db').default
  const DB = await db.create()
  const db_elip = DB.getModel('Elip')
  try {
    let elips = await db_elip.find()
    for (const elip of elips) {
      try {
        const md = convertJsonToMd(elip.description)
        if (md) {
          await db_elip.update(
            { _id: elip._id },
            { $set: { description: md } }
          )
        }
      } catch (err) {
        console.log(`elip ${elip._id} error`, err)
      }
    }
  } catch (err) {
    console.error(err)
    process.exit(1)
  }
  console.log('done!')
  process.exit(1)
})()
