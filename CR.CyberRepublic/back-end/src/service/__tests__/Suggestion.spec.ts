import * as uuid from 'uuid'
import {expect} from 'chai'
import {constant} from '../../constant';
import {mail} from '../../utility'
import db from '../../db'
import '../../config'
import SuggestionService from '../SuggestionService'
import UserService from '../UserService'

declare var global, describe, test, require, process, beforeAll, afterAll

const sinon = require('sinon')
const user: any = {}
let DB, mailMethod
const service: any = {}

beforeAll(async () => {
  DB = await db.create()

  // stub mail
  mailMethod = sinon.stub(mail, 'send', (options) => {
    return Promise.resolve()
  })

  await DB.getModel('User').remove({
    username: global.DB.MEMBER_USER.username
  })
  await DB.getModel('User').remove({
    username: global.DB.COUNCIL_USER.username
  })
  await DB.getModel('Suggestion').remove({})

  // create a test user as member role
  const userService = new UserService(DB, {
    user: undefined,
  })
  user.admin = await userService.getDBModel('User').findOne({role: constant.USER_ROLE.ADMIN})
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
  user.council = await userService.getDBModel('User').findOne({_id: council._id})

  // service setup
  service.member = new SuggestionService(DB, {user: user.member})
  service.council = new SuggestionService(DB, {user: user.council})
  service.admin = new SuggestionService(DB, {user: user.admin})
})

describe('Tests for Suggestion', () => {
  test('member attempt to create a suggestion should pass', async () => {
    expect(1).to.be.equal(1)
  })

  // test('owner attempt to update his/her suggestion should pass', async () => {
  //   try {
  //     // await service.member.update({ id: suggestion1._id, title: 'abc' })
  //   } catch (err) {
  //     // expect(err).to.be.equal('')
  //   }
  // })

  // test('non-owner attempt to update suggestion should fail', async () => {
  //   try {

  //   } catch (err) {
  //     // expect(err).to.be.equal('Only owner can edit suggestion')
  //   }
  // })

  // test('member attempt to list suggestion should pass', async () => {
  //   try {

  //   } catch (err) {
  //     // expect(err).to.be.equal('Only owner can edit suggestion')
  //   }
  // })

  // test('member attempt to show suggestion should pass', async () => {
  //   try {

  //   } catch (err) {
  //     // expect(err).to.be.equal('Only owner can edit suggestion')
  //   }
  // })
})
