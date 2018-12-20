import {mail} from "../../utility"
import * as uuid from 'uuid'

declare var global, describe, test, require, process, beforeAll, afterAll
import {expect, assert} from 'chai'

const sinon = require('sinon')

import db from '../../db'
import '../../config'
import CVoteService from '../CVoteService'
import UserService from "../UserService"
import TaskService from "../TaskService"

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
    user.council = await userService.registerNewUser(global.DB.COUNCIL_USER)
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
            expect(err).to.be.equal('cvoteservice.create - not council')
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
            expect(err).to.be.equal('cvoteservice.create - not council')
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
