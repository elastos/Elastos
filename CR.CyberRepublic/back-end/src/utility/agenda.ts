import db from '../db'
import { getProposalState, getCurrentHeight } from '../utility'
import CVoteServive from '../service/CVoteService'
const Agenda = require('agenda')
const agenda = new Agenda({ db: { address: process.env.DB_URL } })

agenda.define('make into proposal', async (job: any) => {
  try {
    const DB = await db.create()
    const cvote = await DB.getModel('CVote')
    const cvoteService = new CVoteServive(DB, { user: undefined })

    const suggestions = await DB.getModel('Suggestion').find({ proposed: true })
    console.log('suggestions', suggestions.length)
    if (!suggestions.length) {
      return
    }

    const result = await getCurrentHeight()
    if (!result) {
      console.log('can not get current chain height')
      return
    }
    console.log('current chain height...', result.height)

    let count = 0
    for (let i = 0; i < suggestions.length; i++) {
      const doc = suggestions[i]
      console.log('doc display id', doc.displayId)
      if (!doc.chainHeight) {
        continue
      }

      console.log('saved chain height...', doc.chainHeight)
      if (result.height === parseInt(doc.chainHeight, 10)) {
        continue
      }

      const rs = await getProposalState({ drafthash: doc.draftHash })

      if (!rs && result.height - 6 > parseInt(doc.chainHeight, 10)) {
        console.log('not saved to chain doc._id', doc._id)
        await DB.getModel('Suggestion').update(
          { _id: doc._id },
          { $unset: { proposed: true } }
        )
      }

      if (rs && rs.success && rs.status === 'Registered') {
        console.log('registered doc.displayId', doc.displayId)
        count++
        const proposal = await cvote.findOne({ draftHash: doc.draftHash })
        if (proposal) {
          console.log('existing proposal vid', proposal.vid)
          continue
        }
        const newProposal = await cvoteService.makeSuggIntoProposal({
          suggestion: doc,
          proposalHash: rs.proposalHash,
          chainDid: rs.proposal.crcouncilmemberdid
        })
        if (newProposal) {
          console.log('newProposal.vid', newProposal.vid)
        }
      }
    }
    console.log('proposed suggestion count...', count)
  } catch (err) {
    console.log('make into proposal cron job err...', err)
  }
})
;(async function () {
  console.log('------make into proposal cron job starting------')
  await agenda.start()
  await agenda.every('2 minutes', 'make into proposal')
})()
