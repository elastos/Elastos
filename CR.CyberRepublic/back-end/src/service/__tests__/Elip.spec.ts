import { expect } from 'chai'
import { constant } from '../../constant'
import db from '../../db'
import '../../config'
import ElipService from '../ElipService'
import UserService from '../UserService'
import ElipReviewService from '../ElipReviewService'

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
    DB.getModel('Elip').remove({}),
    DB.getModel('Elip_Review').remove({})
  ])

  const userService = new UserService(DB, {})
  const result = await Promise.all([
    userService.registerNewUser(global.DB.MEMBER_USER),
    userService.registerNewUser(global.DB.SECRETARY_USER),
    userService.getDBModel('User').findOne({ role: constant.USER_ROLE.ADMIN })
  ])
  user.member = result[0]
  user.member1 = result[1]
  user.admin = result[2]

  // add a SECRETARY role
  const adminService = new UserService(DB, {
    user: user.admin
  })
  await adminService.updateRole({
    userId: user.member1._id,
    role: constant.USER_ROLE.SECRETARY
  })
  user.secretary = await userService
    .getDBModel('User')
    .findOne({ _id: user.member1._id })
})

describe('Tests for ELIP', () => {
  test('A member create an ELIP', async () => {
    await DB.getModel('Elip').remove({})
    const elipService = new ElipService(DB, {
      user: user.member
    })
    const rs: any = await elipService.create(global.DB.ELIP_1)
    expect(rs.createdBy.toString()).to.be.equal(user.member._id.toString())
    expect(rs.status).to.be.equal(constant.ELIP_STATUS.PERSONAL_DRAFT)
  })

  test('An ELIP is updated by its author', async () => {
    await DB.getModel('Elip').remove({})
    const elipService = new ElipService(DB, {
      user: user.member
    })
    const elip_2: any = await elipService.create(global.DB.ELIP_2)

    // An ELIP can not be updated If its status is WAIT_FOR_REVIEW.
    try {
      await elipService.update({
        _id: elip_2._id,
        title: 'update elip',
        status: constant.ELIP_STATUS.WAIT_FOR_REVIEW
      })
    } catch (error) {
      expect(error).to.be.equal(
        `ElipService.update - cannot update a WAIT_FOR_REVIEW elip`
      )
    }

    // An ELIP can be updated If its status is REJECTED.
    await DB.getModel('Elip').update(
      { _id: elip_2._id },
      { status: constant.ELIP_STATUS.REJECTED}
    )
    const rs: any = await elipService.update({
      _id: elip_2._id,
      title: 'update title',
      status: constant.ELIP_STATUS.WAIT_FOR_REVIEW
    })
    expect(rs.nModified).to.be.equal(1)
    const rs1 = await DB.getModel('Elip').findOne({ _id: elip_2._id })
    expect(rs1.status).to.be.equal(constant.ELIP_STATUS.WAIT_FOR_REVIEW)

    // The author of ELIP can change ELIP's status from DRAFT to FINAL REVIEW.
    await DB.getModel('Elip').update(
      { _id: elip_2._id },
      { status: constant.ELIP_STATUS.DRAFT }
    )
    const rs2: any = await elipService.update({
      _id: elip_2._id,
      status: constant.ELIP_STATUS.FINAL_REVIEW
    })
    expect(rs2.nModified).to.be.equal(1)
  })

  test('Users with different roles get an ELIP', async () => {
    await DB.getModel('Elip').remove({})
    const elipService = new ElipService(DB, {
      user: user.member
    })
    const elip_3: any = await elipService.create({
      title: 'elip_3',
      status: 'PERSONAL_DRAFT'
    })

    // The ELIP's author can get this elip
    const rs = await elipService.getById(elip_3._id)
    expect(rs.elip._id.equals(elip_3._id)).to.be.equal(true)

    // A secretary can not access this elip with PERSONAL DRAFT status
    const elipService1 = new ElipService(DB, { user: user.secretary })
    const rs1 = await elipService1.getById(elip_3._id)
    expect(rs1.elip.empty).to.be.equal(true)

    // A guest can not get this elip
    const elipService2 = new ElipService(DB, {})
    const rs2 = await elipService2.getById(elip_3._id)
    expect(rs2.elip.empty).to.be.equal(true)

    // A guest can get elips with DRAFT status
    await DB.getModel('Elip').update(
      { _id: elip_3._id },
      { status: constant.ELIP_STATUS.DRAFT }
    )
    const rs3 = await elipService2.getById(elip_3._id)
    expect(rs3.elip._id.equals(elip_3._id)).to.be.equal(true)
  })

  test('Users with different roles get ELIP list', async () => {
    await DB.getModel('Elip').remove({})
    const elipService = new ElipService(DB, {
      user: user.member
    })

    for (const key in constant.ELIP_STATUS) {
      await elipService.create({
        title: `title ${key}`,
        status: key
      })
    }

    // ELIP's author
    const rs = await elipService.list({})
    expect(rs.length).to.be.equal(1)

    // A guest
    const elipService1 = new ElipService(DB, {})
    const rs1 = await elipService1.list({})
    expect(rs1.length).to.be.equal(0)

    // A secretary
    const elipService2 = new ElipService(DB, { user: user.secretary })
    const rs2 = await elipService2.list({})
    expect(rs2.length).to.be.equal(0)
  })

  test('A secretary review an ELIP', async () => {
    await DB.getModel('Elip').remove({})
    await DB.getModel('Elip_Review').remove({})
    const elipService = new ElipService(DB, {
      user: user.member
    })
    const elip = await elipService.create({
      title: 'title',
      status: constant.ELIP_STATUS.WAIT_FOR_REVIEW
    })

    const elipReviewService = new ElipReviewService(DB, {
      user: user.secretary
    })
    await elipReviewService.create({
      elipId: elip._id,
      status: constant.ELIP_REVIEW_STATUS.REJECTED,
      comment: 'need more info'
    })

    const rs = await DB.getModel('Elip_Review').find({})
    expect(rs.length).to.be.equal(1)

    const rs1 = await elipService.getById(elip._id)
    expect(rs1.elip.status === constant.ELIP_STATUS.REJECTED).to.be.equal(true)
  })
})
