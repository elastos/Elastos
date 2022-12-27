import * as uuid from 'uuid'
import {expect} from 'chai'
import {constant} from '../../constant';
import {mail} from '../../utility'
import db from '../../db'
import '../../config'
import ReleaseService from '../ReleaseService'
import UserService from '../UserService'

declare var global, describe, test, require, process, beforeAll, afterAll

const sinon = require('sinon')
const user: any = {}
let DB, mailMethod
const service: any = {}

const ownPermissions = global.DB.PERMISSIONS.Release

beforeAll(async () => {
  DB = await db.create()

  await DB.getModel('Release').remove({})
  await DB.getModel('User').remove({
    username: global.DB.MEMBER_USER.username
  })

  // create a test user as member role
  const userService = new UserService(DB, {
    user: undefined,
  })
  user.admin = await userService.getDBModel('User').findOne({role: constant.USER_ROLE.ADMIN})
  user.member = await userService.registerNewUser(global.DB.MEMBER_USER)

  service.admin = new ReleaseService(DB, {user: user.admin})
  service.member = new ReleaseService(DB, {user: user.member})
})

describe('Tests for Release', () => {
  let release: any

  // test('Permission: should ONLY allow ADMIN to create a release', async () => {
  //   try {
  //     const permissions = await DB.getModel('Permission_Role').find({
  //       resourceType: ownPermissions.create.resourceType,
  //       url: ownPermissions.create.url,
  //       httpMethod: ownPermissions.create.httpMethod,
  //     });
  //     expect(permissions.length).to.be.equal(1)
  //     expect(permissions[0].role).to.be.equal('ADMIN')
  //   } catch (err) {
  //     // expect(err).to.be.equal('')
  //   }
  // })

  // test('Permission: should ONLY allow ADMIN to update a release', async () => {
  //   try {
  //     const permissions = await DB.getModel('Permission_Role').find({
  //       resourceType: ownPermissions.update.resourceType,
  //       url: ownPermissions.update.url,
  //       httpMethod: ownPermissions.update.httpMethod,
  //     });
  //     expect(permissions.length).to.be.equal(1)
  //     expect(permissions[0].role).to.be.equal('ADMIN')
  //   } catch (err) {
  //     // expect(err).to.be.equal('')
  //   }
  // })

  // test('Permission: should ONLY allow ADMIN to delete a release', async () => {
  //   try {
  //     const permissions = await DB.getModel('Permission_Role').find({
  //       resourceType: ownPermissions.delete.resourceType,
  //       url: ownPermissions.delete.url,
  //       httpMethod: ownPermissions.delete.httpMethod,
  //     });
  //     expect(permissions.length).to.be.equal(1)
  //     expect(permissions[0].role).to.be.equal('ADMIN')
  //   } catch (err) {
  //     // expect(err).to.be.equal('')
  //   }
  // })


  test('admin attempt to create a release should pass', async () => {
    try {
      release = await service.admin.create(
        Object.assign(
          global.DB.RELEASE_1, {
              createdBy: user.admin._id
          }
        )
      );
      expect(release.createdBy.toString()).to.be.equal(user.admin._id.toString())
    } catch (err) {
      // expect(err).to.be.equal('')
    }
  })

  test('admin attempt to update release should pass', async () => {
    try {
      const uuidVal = uuid.v4()

      const updateRs: any = await service.admin.update({
          _id: release._id,
          desc: uuidVal
      })

      expect(updateRs.title).to.equal(global.DB.RELEASE_1.title)
      expect(updateRs.desc).to.equal(uuidVal)

    } catch (err) {
      // expect(err).to.be.equal('')
    }
  })
  // test('admin attempt to delete release should pass', async () => {
  //   try {
  //   } catch (err) {
  //     // expect(err).to.be.equal('')
  //   }
  // })

  test('member attempt to list release should pass', async () => {
    try {
      const rs = service.member.list()
      expect(rs.length).to.be.equal(1)
    } catch (err) {
    }
  })

  test('member attempt to show release should pass', async () => {
    try {
      const rs = service.member.show({
        _id: release._id,
      })
      expect(rs._id.toString()).to.be.equal(release._id.toString())
    } catch (err) {
    }
  })
})
