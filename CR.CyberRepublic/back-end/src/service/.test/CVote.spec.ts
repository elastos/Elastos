import * as uuid from 'uuid'
import { expect } from 'chai'
import { constant } from '../../constant';
import {mail} from '../../utility'
import db from '../../db'
import '../../config'
import CVoteService from '../CVoteService'
import UserService from '../UserService'

declare var global, describe, test, require, process, beforeAll, afterAll

const sinon = require('sinon')
const user: any = {}
let DB, mailMethod

beforeAll(async ()=>{
    DB = await db.create()

    // stub mail
    mailMethod = sinon.stub(mail, 'send', (options) => {
        console.log('mail suppressed', options)
        return Promise.resolve()
    })

    await DB.getModel('User').remove({
        role : 'MEMBER'
    })
    await DB.getModel('User').getDBInstance().findByIdAndRemove(global.DB.COUNCIL_USER._id.$oid)
    await DB.getModel('Task').remove({})
    await DB.getModel('CVote').remove({})

    // create a test user as member role
    const userService = new UserService(DB, {
        user: null
    })
    user.member = await userService.registerNewUser(global.DB.MEMBER_USER)
    user.admin = await userService.getDBModel('User').findOne(global.DB.ADMIN_USER)
    const council = await userService.registerNewUser(global.DB.COUNCIL_USER)
    // added COUNCIL role to council
    const adminService = new UserService(DB, {
        user: user.admin
    })
    await adminService.update({
        userId: council._id,
        role: constant.USER_ROLE.COUNCIL
    })
    user.council = await userService.getDBModel('User').findOne({ _id: council._id })
})

describe('Tests for CVote', () => {

    let cvote1

    test('member attempt to create a proposal should fail', async () => {

        const cvoteService = new CVoteService(DB, {
            user : user.member
        })

        try {
            const rs: any = await cvoteService.create(Object.assign(
                global.DB.CVOTE_1, {
                    createdBy: user.member._id
                }
            ))
        } catch (err) {
            expect(err).to.be.equal('cvoteservice.create - not council or secretary')
        }

    })


    test('admin attempt to create a proposal should fail', async () => {

        const cvoteService = new CVoteService(DB, {
            user : user.admin
        })

        try {
            const rs: any = await cvoteService.create(Object.assign(
                global.DB.CVOTE_1, {
                    createdBy: user.member._id
                }
            ))
        } catch (err) {
            expect(err).to.be.equal('cvoteservice.create - not council or secretary')
        }

    })

    test('council attempt to create a proposal should pass', async () => {

        const cvoteService = new CVoteService(DB, {
            user : user.council
        })

        const rs: any = await cvoteService.create(Object.assign(
            global.DB.CVOTE_1, {
                createdBy: user.member._id
            }
        ))

        cvote1 = rs

        expect(rs.createdBy.toString()).to.be.equal(user.council._id.toString())

        // published should always be false by default unless otherwise specified
        expect(rs.published).to.be.false
    })

    test('member attempt to list unpublished proposals should fail', async () => {

        const cvoteService = new CVoteService(DB, {
            user : user.member
        })

        try {
            const rs: any = await cvoteService.list({
                published: false
            })

        } catch (err) {
            expect(err).to.be.equal('cvoteservice.list - unpublished proposals only visible to admin')
        }

        try {
            const rs: any = await cvoteService.list({

            })

        } catch (err) {
            expect(err).to.be.equal('cvoteservice.list - unpublished proposals only visible to admin')
        }

    })

    test('admin attempt to publish a proposal should fail', async () => {

        const cvoteService = new CVoteService(DB, {
            user : user.admin
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
            user : user.council
        })

        const updateRs: any = await cvoteService.update({
            _id: cvote1._id,
            published: true
        })

        expect(updateRs.nModified).to.be.equal(1)

        const rs = await DB.getModel('CVote').findById(cvote1._id)

        expect(rs.title).to.equal(global.DB.CVOTE_1.title)

        const uuidVal = uuid.v4()

        await cvoteService.update({
            _id: cvote1._id,
            content: uuidVal
        })

        const rs2 = await DB.getModel('CVote').findById(cvote1._id)

        expect(rs2.title).to.equal(global.DB.CVOTE_1.title)
        expect(rs2.content).to.equal(uuidVal)
    })

    // TODO: council changing vote of other member should fail

    test('member attempt to list published proposals should pass', async () => {

        const cvoteService = new CVoteService(DB, {
            user: user.member
        })

        const rs: any = await cvoteService.list({
            published: true
        })

        expect(rs.length).to.be.equal(1)
    })
})
