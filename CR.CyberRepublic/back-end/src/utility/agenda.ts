import db from '../db'
import { getProposalState } from '../utility'
import CVoteServive from '../service/CVoteService'
const Agenda = require('agenda')
const agenda = new Agenda({ db: { address: process.env.DB_URL } })

agenda.define('make into proposal', async (job: any) => {
  try {
    const DB = await db.create()
    const cvote = await DB.getModel('CVote')
    const suggestions = await DB.getModel('Suggestion').find({ proposed: true })
    console.log('suggestions', suggestions.length)
    const cvoteService = new CVoteServive(DB, { user: undefined })
    let count = 0
    for (let i = 0; i < suggestions.length; i++) {
      const doc = suggestions[i]
      console.log('doc display id', doc.displayId)
      const rs = await getProposalState({ drafthash: doc.draftHash })
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
  await agenda.every('1 minutes', 'make into proposal')
})()
