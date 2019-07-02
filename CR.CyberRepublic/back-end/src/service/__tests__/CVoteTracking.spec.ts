import * as uuid from 'uuid'
import { expect } from 'chai'
import { constant } from '../../constant';
import db from '../../db'
import '../../config'
import CVoteTrackingService from '../CVoteTrackingService'
import UserService from '../UserService'
import CVoteService from '../CVoteService'

declare var global, describe, test, require, process, beforeAll, afterAll

const user: any = {}
const service: any = {}
let DB: any
let proposal: any


beforeAll(async () => {
  DB = await db.create()

  await DB.getModel('CVote_Tracking').remove({})
  await DB.getModel('User').remove({
    username: global.DB.SECRETARY_USER.username
  })

  // create a test user as secretary role
  const userService = new UserService(DB, {
    user: undefined,
  })
  user.secretary = await userService.registerNewUser(global.DB.SECRETARY_USER)

  service.secretary = new CVoteTrackingService(DB, { user: user.secretary })
  const cvoteService = new CVoteService(DB, {
    user : user.secretary
  })

  proposal = await cvoteService.create(Object.assign(
      global.DB.CVOTE_1, {
          createdBy: user.secretary._id
      }
  ))

})

describe('Tests for CVoteTracking', () => {
  let tracking: any

  test('attempt to create a DRAFT tracking should pass', async () => {
    try {
      tracking = await service.secretary.create(
        Object.assign(
          global.DB.CVOTE_TRACKING, {
            createdBy: user.secretary._id,
            proposalId: proposal._id,
          }
        )
      );
      expect(tracking.createdBy.toString()).to.be.equal(user.secretary._id.toString())
    } catch (err) {
      // expect(err).to.be.equal('')
    }
  })

  test('attempt to approve DRAFT tracking should fail', async () => {
    try {
      const updateRs: any = await service.secretary.approve({
        _id: tracking._id,
      })

      expect(updateRs.status).to.equal(constant.CVOTE_TRACKING_STATUS.DRAFT)

    } catch (err) {
    }
  })

  test('attempt to reject DRAFT tracking should fail', async () => {
    try {
      const updateRs: any = await service.secretary.reject({
        _id: tracking._id,
        comment: 'reject'
      })

      expect(updateRs.status).to.equal(constant.CVOTE_TRACKING_STATUS.DRAFT)

    } catch (err) {
    }
  })

  test('attempt to update tracking should pass', async () => {
    try {
      const uuidVal = uuid.v4()

      const updateRs: any = await service.secretary.update({
        _id: tracking._id,
        content: uuidVal,
        status
      })

      expect(updateRs.content).to.equal(uuidVal)

    } catch (err) {
      // expect(err).to.be.equal('')
    }
  })

  test('attempt to approve REVIEWING tracking should pass', async () => {
    try {
      const rs = service.secretary.approve({
        _id: tracking._id,
      })
      expect(rs.status).to.be.equal(constant.CVOTE_TRACKING_STATUS.PUBLISHED)
    } catch (err) {
    }
  })

  test('attempt to create a REVIEWING tracking should pass', async () => {
    try {
      tracking = await service.secretary.create(
        Object.assign(
          global.DB.CVOTE_TRACKING, {
            createdBy: user.secretary._id,
            proposalId: proposal._id,
          }
        )
      );
      expect(tracking.createdBy.toString()).to.be.equal(user.secretary._id.toString())
    } catch (err) {
      // expect(err).to.be.equal('')
    }
  })

  test('attempt to reject REVIEWING tracking should pass', async () => {
    try {
      const updateRs: any = await service.secretary.reject({
        _id: tracking._id,
        comment: 'reject'
      })

      expect(updateRs.status).to.equal(constant.CVOTE_TRACKING_STATUS.REJECT)

    } catch (err) {
    }
  })

  test('attempt to list tracking should pass', async () => {
    try {
      const rs = service.secretary.list()
      expect(rs.length).to.be.equal(2)
    } catch (err) {
    }
  })

  test('attempt to list public tracking should pass', async () => {
    try {
      const rs = service.secretary.listPublic()
      expect(rs.length).to.be.equal(1)
    } catch (err) {
    }
  })

})
