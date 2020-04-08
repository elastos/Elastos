import * as uuid from 'uuid'
import { expect } from 'chai'
import { constant } from '../../constant'
import { mail } from '../../utility'
import db from '../../db'
import '../../config'
import CVoteService from '../CVoteService'
import UserService from '../UserService'
import SuggestionService from '../SuggestionService'

declare var global, describe, test, require, process, beforeAll, afterAll

const sinon = require('sinon')
const user: any = {}
let DB, mailMethod
let suggestion
const suggestionService: any = {}

beforeAll(async () => {
  DB = await db.create()

  // stub mail
  mailMethod = sinon.stub(mail, 'send', options => {
    return Promise.resolve()
  })

  await DB.getModel('User').remove({
    username: global.DB.MEMBER_USER.username
  })
  await DB.getModel('User').remove({
    username: global.DB.COUNCIL_USER.username
  })

  await DB.getModel('Task').remove({})
  await DB.getModel('CVote').remove({})

  // create a test user as member role
  const userService = new UserService(DB, {
    user: undefined
  })
  user.admin = await userService
    .getDBModel('User')
    .findOne({ role: constant.USER_ROLE.ADMIN })
  user.member = await userService.registerNewUser(global.DB.MEMBER_USER)
  const council = await userService.registerNewUser(global.DB.COUNCIL_USER)
  // added COUNCIL role to council
  const adminService = new UserService(DB, {
    user: user.admin
  })

  await adminService.updateRole({
    userId: council._id,
    role: constant.USER_ROLE.COUNCIL
  })
  user.council = await userService
    .getDBModel('User')
    .findOne({ _id: council._id })

  // create suggestion service
  suggestionService.council = new SuggestionService(DB, {
    user: user.council
  })
  // create a suggestion instance
  suggestion = await suggestionService.council.create(
    Object.assign(global.DB.SUGGESTION_1, {
      createdBy: user.member._id
    })
  )
})

describe('Tests for CVote', () => {
  let cvote1
  // DO_NOT_TEST: We have accessControl module, need to refactor with that code
  //   test('member attempt to create a proposal should fail', async () => {
  //     const cvoteService = new CVoteService(DB, {
  //       user: user.member
  //     })

  //     try {
  //       const rs: any = await cvoteService.create(
  //         Object.assign(global.DB.CVOTE_1, {
  //           createdBy: user.member._id
  //         })
  //       )
  //     } catch (err) {
  //       expect(err).to.be.equal('cvoteservice.create - not council or secretary')
  //     }
  //   })

  //   test('admin attempt to create a proposal should fail', async () => {
  //     const cvoteService = new CVoteService(DB, {
  //       user: user.admin
  //     })

  //     try {
  //       const rs: any = await cvoteService.create(
  //         Object.assign(global.DB.CVOTE_1, {
  //           createdBy: user.admin._id
  //         })
  //       )
  //     } catch (err) {
  //       expect(err).to.be.equal('cvoteservice.create - not council or secretary')
  //     }
  //   })

  test('council attempt to create a proposal should pass', async () => {
    const cvoteService = new CVoteService(DB, {
      user: user.council
    })

    const rs: any = await cvoteService.create(
      Object.assign(global.DB.CVOTE_1, {
        createdBy: user.council._id
      })
    )

    cvote1 = rs

    expect(rs.createdBy.toString()).to.be.equal(user.council._id.toString())

    // published should always be false by default unless otherwise specified
    expect(rs.published).to.be.false
  })

  test('member attempt to list unpublished proposals should fail', async () => {
    const cvoteService = new CVoteService(DB, {
      user: user.member
    })

    try {
      const rs: any = await cvoteService.list({
        published: false
      })
    } catch (err) {
      expect(err).to.be.equal(
        'cvoteservice.list - unpublished proposals only visible to council/secretary'
      )
    }

    try {
      const rs: any = await cvoteService.list({})
    } catch (err) {
      expect(err).to.be.equal(
        'cvoteservice.list - unpublished proposals only visible to council/secretary'
      )
    }
  })

  test('admin attempt to publish a proposal should fail', async () => {
    const cvoteService = new CVoteService(DB, {
      user: user.admin
    })

    try {
      const rs: any = await cvoteService.update({
        published: true
      })
    } catch (err) {
      expect(err).to.be.equal('cvoteservice.update - not council')
    }
  })

  test('council attempt to update a proposal should pass', async () => {
    const cvoteService = new CVoteService(DB, {
      user: user.council
    })

    const updateRs: any = await cvoteService.update({
      _id: cvote1._id,
      published: true
    })

    expect(updateRs.published).to.be.equal(true)

    expect(updateRs.title).to.equal(global.DB.CVOTE_1.title)

    const uuidVal = uuid.v4()
    const updateObj = {
      _id: cvote1._id,
      abstract: `${uuidVal} - abstract`,
      goal: `${uuidVal} - goal`,
      motivation: `${uuidVal} - motivation`,
      relevance: `${uuidVal} - relevance`,
      // budget: `${uuidVal} - budget`,
      plan: `${uuidVal} - plan`
    }
    const updateRs2: any = await cvoteService.update(updateObj)

    expect(updateRs2.title).to.equal(global.DB.CVOTE_1.title)
    expect(updateRs2.abstract).to.equal(updateObj.abstract)
    expect(updateRs2.goal).to.equal(updateObj.goal)
    expect(updateRs2.motivation).to.equal(updateObj.motivation)
    expect(updateRs2.relevance).to.equal(updateObj.relevance)
    // expect(updateRs2.budget).to.equal(updateObj.budget)
    expect(updateRs2.plan).to.equal(updateObj.plan)
  })

  // TODO: council changing vote of other member should fail

  test('member attempt to list published proposals should pass', async () => {
    const cvoteService = new CVoteService(DB, {
      user: user.member
    })

    const rs: any = await cvoteService.list({
      published: true
    })

    const {list, total} = rs

    expect(list.length).to.be.equal(1)
  })

  test('council attempt to make suggestion to proposal should pass', async () => {
    // get suggestion
    // create proposal with suggestion info
    const cvoteService = new CVoteService(DB, {
      user: user.council
    })

    const rs: any = await cvoteService.create(
      Object.assign(global.DB.CVOTE_1, {
        createdBy: user.council._id,
        suggestionId: suggestion._id
      })
    )
    expect(rs.reference.toString()).to.be.equal(suggestion._id.toString())

    suggestion = await DB.getModel('Suggestion').findById(suggestion._id)

    expect(rs._id.toString()).to.be.equal(
      suggestion.reference[suggestion.reference.length - 1].toString()
    )
  })

  test('council attempt to delete a draft proposal should pass', async () => {
    const cvoteService = new CVoteService(DB, {
      user: user.council
    })
    // find a draft proposal document
    const cvote = await DB.getModel('CVote').findOne({
      status: constant.CVOTE_STATUS.DRAFT
    })
    // make sure the draft proposal exists
    expect(cvote.status).to.be.equal(constant.CVOTE_STATUS.DRAFT)
    const rs: any = await cvoteService.deleteDraft({
      _id: cvote._id
    })
    expect(rs.deletedCount).to.be.equal(1)
  })
})
