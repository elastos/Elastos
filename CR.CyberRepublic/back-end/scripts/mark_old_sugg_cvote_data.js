require('../dist/src/config')
;(async () => {
  const db = await require('../dist/src/db').default
  const DB = await db.create()
  const db_sugg = DB.getModel('Suggestion')
  const db_cvote = DB.getModel('CVote')
  try {
    const suggs = await db_sugg.find()
    console.log('suggs.length', suggs.length)
    await db_sugg.update({}, { $set: { old: true } }, { multi: true })

    const proposals = await db_cvote.find()
    console.log('proposals.length', proposals.length)
    await db_cvote.update({}, { $set: { old: true } }, { multi: true })
  } catch (err) {
    console.error(err)
  }
  process.exit(1)
})()
