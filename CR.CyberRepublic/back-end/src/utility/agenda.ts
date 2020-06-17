import db from '../db'
import { getProposalState } from '../utility'
import CVoteServive from '../service/CVoteService'
import CouncilService from '../service/CouncilService'
import UserService from '../service/UserService'
const Agenda = require('agenda')
const agenda = new Agenda({ db: { address: process.env.DB_URL } })

const JOB_NAME = {
  INTOPROPOSAL: 'make into proposal',
  CVOTEJOB: 'cvote poll proposal',
  COUNCILJOB: 'council poll change',
  USERJOB: 'user poll did infomation',
  UPDATEMILESTONE: 'update milestone status'
}

agenda.define(JOB_NAME.UPDATEMILESTONE, async(job: any) => {
  console.log('------begin update milestone status------')
  try {
    const DB = await db.create()
    const cvoteService = new CVoteServive(DB, { user: undefined })
    await cvoteService.updateProposalBudget()
  } catch(err) {
    console.log('update milestone status cron job err...', err)
  }
})

agenda.define(JOB_NAME.INTOPROPOSAL, async (job: any) => {
  console.log('------begin polling proposal state------')
  try {
    const DB = await db.create()
    const cvote = await DB.getModel('CVote')
    const cvoteService = new CVoteServive(DB, { user: undefined })

    const suggestions = await DB.getModel('Suggestion').find({
      'proposers.did': { $exists: true }
    })
    console.log('suggestions', suggestions.length)
    if (!suggestions.length) {
      return
    }

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
agenda.define(JOB_NAME.CVOTEJOB, async (job: any) => {
  try {
    const DB = await db.create()
    const cvoteService = new CVoteServive(DB, { user: undefined })
    await cvoteService.pollProposal()
    console.log(JOB_NAME.CVOTEJOB, 'at working')
  } catch (err) {
    console.log('', err)
  }
})
agenda.define(JOB_NAME.COUNCILJOB, async (job: any) => {
  try {
    const DB = await db.create()
    const councilService = new CouncilService(DB, { user: undefined })
    await councilService.eachSecretariatJob()
    await councilService.eachCouncilJobPlus()
    console.log(JOB_NAME.COUNCILJOB, 'at working')
  } catch (err) {
    console.log('', err)
  }
})
agenda.define(JOB_NAME.USERJOB, async (job: any) => {
  try {
    const DB = await db.create()
    const userService = new UserService(DB, { user: undefined })
    await userService.eachJob()
    console.log(JOB_NAME.USERJOB, 'at working')
  } catch (err) {
    console.log('', err)
  }
})
;(async function () {
  console.log('------cron job starting------')
  await agenda.start()
  await agenda.every('2 minutes', JOB_NAME.INTOPROPOSAL)
  await agenda.every('2 minutes', JOB_NAME.CVOTEJOB)
  await agenda.every('5 minutes', JOB_NAME.COUNCILJOB)
  await agenda.every('30 minutes', JOB_NAME.USERJOB)
  await agenda.every('2 minutes', JOB_NAME.UPDATEMILESTONE)
})()
