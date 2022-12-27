declare var global, describe, test, expect, require, process, beforeAll, afterAll

const sinon = require('sinon')

import * as chai from 'chai'

const assert = chai.assert

import db from '../../db'
import '../../config'

import {mail} from '../../utility'

import UserService from '../UserService'
import TaskService from '../TaskService'

const testData: any = {}
let DB, taskServiceMember, taskServiceOrganizer, taskServiceAdmin, mailMethod

import {constant} from '../../constant';

/**
 * global.DB is declared in test/unit/setup.js
 *
 * TODO: find a reload function for a sequelize object
 */
beforeAll(async ()=>{
    DB = await db.create()

    // stub mail
    mailMethod = sinon.stub(mail, 'send', (options) => {
        console.log('mail suppressed', options)
        return Promise.resolve();
    });

    // remove test user
    await DB.getModel('User').remove({
        role: constant.USER_ROLE.MEMBER
    });
    await DB.getModel('User').remove({
        username: global.DB.ORGANIZER_USER.username
    });

    // create a test user as member role
    const userService = new UserService(DB, {
        user: undefined
    })
    const organizer = await userService.registerNewUser(global.DB.ORGANIZER_USER)
    testData.admin = await userService.getDBModel('User').findOne({ role: constant.USER_ROLE.ADMIN });


    // set member to organizer
    await userService.getDBModel('User').update({_id: organizer._id}, { role: constant.USER_ROLE.LEADER })

    testData.organizer = await userService.getDBModel('User').findOne({username: global.DB.ORGANIZER_USER.username})

    // create social task
    taskServiceOrganizer = new TaskService(DB, {
        user: testData.organizer
    })

    testData.taskSocialEvent = await taskServiceOrganizer.create(global.DB.TASK_1)

    // initialize this
    taskServiceAdmin = new TaskService(DB, {
        user: testData.admin
    })

    testData.member = await userService.registerNewUser(global.DB.MEMBER_USER)

    taskServiceMember = new TaskService(DB, {
        user: testData.member
    })
})

describe('Tests for Task Update', () => {

    // TODO: add another test for leaders it should not be APPROVED
    // test('Make sure task is initially PENDING for non-admin', async () => {
    //     expect(testData.taskSocialEvent.status).toBe(constant.TASK_STATUS.PENDING)
    // })

    // test('Member cannot change anything', async () => {

    //     try {
    //         await taskServiceMember.update({
    //             taskId: testData.taskSocialEvent._id,
    //             status: constant.TASK_STATUS.SUCCESS
    //         })

    //         assert.fail('Should fail with Access Denied')

    //     } catch (err) {
    //         expect(err).toBe('Access Denied')
    //     }
    // })

    // test.skip('Task cannot be set from PENDING to APPROVED by organizer', async () => {
    //     // however admin can change it to anything
    //     await taskServiceOrganizer.update({
    //         taskId: testData.taskSocialEvent._id,
    //         status: constant.TASK_STATUS.APPROVED
    //     })

    //     testData.taskSocialEvent = await DB.getModel('Task').findOne({
    //         _id: testData.taskSocialEvent._id
    //     })

    //     expect(testData.taskSocialEvent.status).toBe(constant.TASK_STATUS.PENDING)
    // })

    // test('Make sure only admin can set to APPROVED', async () => {

    //     try {

    //         await taskServiceOrganizer.update({
    //             taskId: testData.taskSocialEvent._id,
    //             status: constant.TASK_STATUS.APPROVED
    //         })

    //         assert.fail('Should fail with Access Denied')

    //     } catch (err) {
    //         expect(err).toBe('Access Denied - Status')
    //     }

    //     mailMethod.reset()

    //     await taskServiceAdmin.update({
    //         taskId: testData.taskSocialEvent._id,
    //         status: constant.TASK_STATUS.APPROVED
    //     })

    //     expect(mailMethod.calledOnce).toBe(true)

    //     testData.taskSocialEvent = await DB.getModel('Task').findOne({
    //         _id: testData.taskSocialEvent._id
    //     })

    //     expect(testData.taskSocialEvent.status).toBe(constant.TASK_STATUS.APPROVED)
    // })

    // test.skip('Both organizer/admin can set to ASSIGNED/SUBMITTED', async () => {

    //     await taskServiceOrganizer.update({
    //         taskId: testData.taskSocialEvent._id,
    //         status: constant.TASK_STATUS.APPROVED
    //     })

    //     testData.taskSocialEvent = await DB.getModel('Task').findOne({
    //         _id: testData.taskSocialEvent._id
    //     })

    //     expect(testData.taskSocialEvent.status).toBe(constant.TASK_STATUS.APPROVED)

    //     // admin can change back to approved
    //     await taskServiceAdmin.update({
    //         taskId: testData.taskSocialEvent._id,
    //         status: constant.TASK_STATUS.APPROVED
    //     })
    //     testData.taskSocialEvent = await DB.getModel('Task').findOne({
    //         _id: testData.taskSocialEvent._id
    //     })
    //     expect(testData.taskSocialEvent.status).toBe(constant.TASK_STATUS.APPROVED)

    //     // admin can assign too
    //     await taskServiceAdmin.update({
    //         taskId: testData.taskSocialEvent._id,
    //         status: constant.TASK_STATUS.APPROVED
    //     })

    //     testData.taskSocialEvent = await DB.getModel('Task').findOne({
    //         _id: testData.taskSocialEvent._id
    //     })

    //     expect(testData.taskSocialEvent.status).toBe(constant.TASK_STATUS.APPROVED)

    //     // reset to APPROVED
    //     await taskServiceAdmin.update({
    //         taskId: testData.taskSocialEvent._id,
    //         status: constant.TASK_STATUS.APPROVED
    //     })

    //     // test SUBMITTED status - but only if assigned
    //     try {
    //         await taskServiceOrganizer.update({
    //             taskId: testData.taskSocialEvent._id,
    //             status: constant.TASK_STATUS.SUBMITTED
    //         })

    //         assert.fail('Should fail with Invalid Action')

    //     } catch (err) {
    //         expect(err).toBe('Invalid Action')
    //     }

    //     await taskServiceOrganizer.update({
    //         taskId: testData.taskSocialEvent._id,
    //         status: constant.TASK_STATUS.APPROVED
    //     })

    //     await taskServiceOrganizer.update({
    //         taskId: testData.taskSocialEvent._id,
    //         status: constant.TASK_STATUS.SUBMITTED
    //     })

    //     testData.taskSocialEvent = await DB.getModel('Task').findOne({
    //         _id: testData.taskSocialEvent._id
    //     })

    //     expect(testData.taskSocialEvent.status).toBe(constant.TASK_STATUS.SUBMITTED)
    // })

    // test('Only admin can set to APPROVED / CANCELED / DISTRIBUTED', async () => {

    //     // reset to PENDING
    //     await taskServiceAdmin.update({
    //         taskId: testData.taskSocialEvent._id,
    //         status: constant.TASK_STATUS.PENDING
    //     })

    //     // for organizers we already tested APPROVED
    //     try {

    //         await taskServiceOrganizer.update({
    //             taskId: testData.taskSocialEvent._id,
    //             status: constant.TASK_STATUS.DISTRIBUTED
    //         })

    //         assert.fail('Should fail with Access Denied')

    //     } catch (err) {
    //         expect(err).toBe('Access Denied - Status')
    //     }

    //     try {

    //         await taskServiceOrganizer.update({
    //             taskId: testData.taskSocialEvent._id,
    //             status: constant.TASK_STATUS.CANCELED
    //         })

    //         assert.fail('Should fail with Access Denied')

    //     } catch (err) {
    //         expect(err).toBe('Access Denied - Status')
    //     }

    //     // now try for admins, should work
    //     await taskServiceAdmin.update({
    //         taskId: testData.taskSocialEvent._id,
    //         status: constant.TASK_STATUS.DISTRIBUTED
    //     })

    //     testData.taskSocialEvent = await DB.getModel('Task').findOne({
    //         _id: testData.taskSocialEvent._id
    //     })

    //     expect(testData.taskSocialEvent.status).toBe(constant.TASK_STATUS.DISTRIBUTED)

    // })

    test('Organizer cannot change budget/rewards after APPROVED status', async () => {
        // await new Promise((resolve) => {
        //     setTimeout(() => {
        //         resolve()
        //     }, 6000)
        // })
    })

})

afterAll(async () => {
    // remove test task
    testData.taskSocialEvent && await DB.getModel('Task').remove({
        _id: testData.taskSocialEvent._id
    });

    await DB.disconnect()
})
