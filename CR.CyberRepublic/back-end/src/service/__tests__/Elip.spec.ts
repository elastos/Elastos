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
  await DB.getModel('User').remove({
    username: global.DB.MEMBER_USER.username
  })
  await DB.getModel('Elip').remove({})
  // create a test user as MEMBER role
  const userService = new UserService(DB, {
    user: undefined
  })
  user.member = await userService.registerNewUser(global.DB.MEMBER_USER)
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
})
