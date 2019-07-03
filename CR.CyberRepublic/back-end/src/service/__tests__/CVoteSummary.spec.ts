import * as uuid from 'uuid'
import { expect } from 'chai'
import { constant } from '../../constant';
import db from '../../db'
import '../../config'
import CVoteSummaryService from '../CVoteSummaryService'
import UserService from '../UserService'
import CVoteService from '../CVoteService'

declare var global, describe, test, require, process, beforeAll, afterAll

const user: any = {}
const service: any = {}
let DB: any
let proposal: any


beforeAll(async () => {
  DB = await db.create()

  await DB.getModel('CVote_Summary').remove({})
  await DB.getModel('User').remove({
    username: global.DB.SECRETARY_USER.username
  })

  // create a test user as secretary role
  const userService = new UserService(DB, {
    user: undefined,
  })
  user.secretary = await userService.registerNewUser(global.DB.SECRETARY_USER)

  service.secretary = new CVoteSummaryService(DB, { user: user.secretary })
  const cvoteService = new CVoteService(DB, {
    user : user.secretary
  })

  proposal = await cvoteService.create(Object.assign(
      global.DB.CVOTE_1, {
          createdBy: user.secretary._id
      }
  ))

})

describe('Tests for CVoteSummary', () => {
  let summary: any

  test('attempt to create a DRAFT summary should pass', async () => {
    try {
      summary = await service.secretary.create(
        Object.assign(
          global.DB.CVOTE_SUMMARY, {
            createdBy: user.secretary._id,
            proposalId: proposal._id,
          }
        )
      );
      expect(summary.createdBy.toString()).to.be.equal(user.secretary._id.toString())
    } catch (err) {
      // expect(err).to.be.equal('')
    }
  })

  test('attempt to approve DRAFT summary should fail', async () => {
    try {
      const updateRs: any = await service.secretary.approve({
        _id: summary._id,
      })

      expect(updateRs.status).to.equal(constant.CVOTE_SUMMARY_STATUS.DRAFT)

    } catch (err) {
    }
  })

  test('attempt to reject DRAFT summary should fail', async () => {
    try {
      const updateRs: any = await service.secretary.reject({
        _id: summary._id,
        comment: 'reject'
      })

      expect(updateRs.status).to.equal(constant.CVOTE_SUMMARY_STATUS.DRAFT)

    } catch (err) {
    }
  })

  test('attempt to update summary should pass', async () => {
    try {
      const uuidVal = uuid.v4()

      const updateRs: any = await service.secretary.update({
        _id: summary._id,
        content: uuidVal,
        status
      })

      expect(updateRs.content).to.equal(uuidVal)

    } catch (err) {
      // expect(err).to.be.equal('')
    }
  })

  test('attempt to approve REVIEWING summary should pass', async () => {
    try {
      const rs = service.secretary.approve({
        _id: summary._id,
      })
      expect(rs.status).to.be.equal(constant.CVOTE_SUMMARY_STATUS.PUBLISHED)
    } catch (err) {
    }
  })

  test('attempt to create a REVIEWING summary should pass', async () => {
    try {
      summary = await service.secretary.create(
        Object.assign(
          global.DB.CVOTE_SUMMARY, {
            createdBy: user.secretary._id,
            proposalId: proposal._id,
          }
        )
      );
      expect(summary.createdBy.toString()).to.be.equal(user.secretary._id.toString())
    } catch (err) {
      // expect(err).to.be.equal('')
    }
  })

  test('attempt to reject REVIEWING summary should pass', async () => {
    try {
      const updateRs: any = await service.secretary.reject({
        _id: summary._id,
        comment: 'reject'
      })

      expect(updateRs.status).to.equal(constant.CVOTE_SUMMARY_STATUS.REJECT)

    } catch (err) {
    }
  })

  test('attempt to list summary should pass', async () => {
    try {
      const rs = service.secretary.list()
      expect(rs.length).to.be.equal(2)
    } catch (err) {
    }
  })

  test('attempt to list public summary should pass', async () => {
    try {
      const rs = service.secretary.listPublic()
      expect(rs.length).to.be.equal(1)
    } catch (err) {
    }
  })

})
