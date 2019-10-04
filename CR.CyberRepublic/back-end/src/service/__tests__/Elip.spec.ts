import { expect } from 'chai'
import { constant } from '../../constant'
import db from '../../db'
import '../../config'
import ElipService from '../ElipService'
import UserService from '../UserService'

declare var global, describe, test, beforeAll

const user: any = {}
let DB: any

beforeAll(async () => {
  DB = await db.create()
  await Promise.all([
    DB.getModel('User').remove({
      username: {
        $in: [global.DB.MEMBER_USER.username, global.DB.SECRETARY_USER.username]
      }
    }),
    DB.getModel('Elip').remove({})
  ])

  const userService = new UserService(DB, {})
  const result = await Promise.all([
    userService.registerNewUser(global.DB.MEMBER_USER),
    userService.registerNewUser(global.DB.SECRETARY_USER),
    userService.getDBModel('User').findOne({ role: constant.USER_ROLE.ADMIN })
  ])
  user.member = result[0]
  user.secretary = result[1]
  user.admin = result[2]

  // add a SECRETARY role
  const adminService = new UserService(DB, {
    user: user.admin
  })
  await adminService.updateRole({
    userId: user.secretary._id,
    role: constant.USER_ROLE.SECRETARY
  })
})

describe('Tests for ELIP', () => {
  test('A member create an ELIP', async () => {
    const elipService = new ElipService(DB, {
      user: user.member
    })
    const rs: any = await elipService.create(global.DB.ELIP_1)
    expect(rs.createdBy.toString()).to.be.equal(user.member._id.toString())
    expect(rs.status).to.be.equal(constant.ELIP_STATUS.WAIT_FOR_REVIEW)
  })

  test('An ELIP is updated by its author', async () => {
    const elipService = new ElipService(DB, {
      user: user.member
    })
    const elip_2: any = await elipService.create(global.DB.ELIP_2)

    // An ELIP can not be updated If its status is WAIT_FOR_REVIEW.
    try {
      await elipService.update({
        _id: elip_2._id,
        title: 'update elip'
      })
    } catch (error) {
      expect(error).to.be.equal(
        `ElipService.update - cannot update a WAIT_FOR_REVIEW elip`
      )
    }

    // An ELIP can not be updated If its status is REJECTED.
    await DB.getModel('Elip').update(
      { _id: elip_2._id },
      { status: constant.ELIP_STATUS.REJECTED }
    )
    const rs: any = await elipService.update({
      _id: elip_2._id,
      title: 'update title',
      description: 'update description'
    })
    expect(rs.nModified).to.be.equal(1)
    const rs1 = await DB.getModel('Elip').findOne({ _id: elip_2._id })
    expect(rs1.status).to.be.equal(constant.ELIP_STATUS.WAIT_FOR_REVIEW)

    // The author of ELIP can change ELIP's status from DRAFT to SUBMITTED.
    await DB.getModel('Elip').update(
      { _id: elip_2._id },
      { status: constant.ELIP_STATUS.DRAFT }
    )
    const rs2: any = await elipService.update({
      _id: elip_2._id,
      status: constant.ELIP_STATUS.SUBMITTED
    })
    expect(rs2.nModified).to.be.equal(1)
  })
})
