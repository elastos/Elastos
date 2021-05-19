declare var global, describe, test, require, process, beforeAll, afterAll;
import {expect, assert} from 'chai'

const sinon = require('sinon')

import db from '../../db';
import '../../config';
import UserService from '../UserService';
import TaskService from '../TaskService';
import {mail} from '../../utility'
import { constant } from '../../constant';

const user: any = {};
let DB, mailMethod;


beforeAll(async ()=>{
    DB = await db.create();

    // stub mail
    mailMethod = sinon.stub(mail, 'send', (options) => {
        console.log('mail suppressed', options)
        return Promise.resolve();
    });

    await DB.getModel('User').remove({
        username: global.DB.MEMBER_USER.username
    });
    await DB.getModel('Task').remove({});

    // create a test user as member role
    const userService = new UserService(DB, {
        user: undefined
    });
    user.member = await userService.registerNewUser(global.DB.MEMBER_USER);
    user.admin = await userService.getDBModel('User').findOne({ role: constant.USER_ROLE.ADMIN });
});

describe('Tests for Tasks', () => {
    let ts_admin: TaskService, task:any;

    // test('prepare test', async ()=>{
    //     // add a task
    //     ts_admin = new TaskService(DB, {
    //         user : user.admin
    //     });
    //     task = await ts_admin.create(global.DB.TASK_1);
    // });

    // test('should create task success as admin user', async ()=>{
    //     expect(task.createdBy).to.be.equal(user.admin._id)
    // });

    // test('Should create event and fail because reward is over user budget limit', async () => {
    //     const taskService = new TaskService(DB, {
    //         user : user.admin
    //     });
    //     await expect(taskService.create(global.DB.TASK_2)).rejects.toThrowError(/budget/);
    // });

    test('Should create event with status SUCCESS because reward + upfront is under budget limit', async () => {

        // TODO: make sure the elaBudget is deducted from and there is a transaction log
    })

    test('Should create event with status SUCCESS because there is no upfront/reward and limit is 0', async () => {

        // expect user budget to be 0

        // TODO: make sure the elaBudget is deducted from and there is a transaction log
    })

    // test('member should apply to be a task candidate', async () => {
    //     const taskService = new TaskService(DB, {
    //         user : user.member
    //     });

    //     const rs: any = await taskService.addCandidate({
    //         taskId: task._id,
    //         userId: user.member._id
    //     });

    //     expect(rs.status).to.be.equal('PENDING');
    // });

    // test('Should allow removing candidate from task if you are the candidate', async () => {
    //     const task = await ts_admin.create({
    //         ...global.DB.TASK_1,
    //         name : 'task_111'
    //     });

    //     const candidate = await ts_admin.addCandidate({
    //         taskId: task._id,
    //         userId: user.member._id
    //     });

    //     const ts1 = new TaskService(DB, {user: user.member});
    //     const rs: any = await ts1.removeCandidate({
    //         taskId: task._id,
    //         taskCandidateId: user.member._id
    //     });
    //     expect(rs.ok).to.be.equal(1);
    // })

    test('Should allow removing candidate from task if you are the task owner (leader)', async () => {
        // task owner must be admin for now.
        // same to next test
    })

    // test('Should allow removing candidate from task if you are an admin/council', async () => {
    //     const task = await ts_admin.create({
    //         ...global.DB.TASK_1,
    //         name : 'task_222'
    //     });

    //     const ts1 = new TaskService(DB, {user: user.member});
    //     await ts1.addCandidate({
    //         taskId: task._id,
    //         userId: user.member._id
    //     });

    //     const rs: any = await ts_admin.removeCandidate({
    //         taskId: task._id,
    //         taskCandidateId: user.member._id
    //     });
    //     expect(rs.ok).to.be.equal(1);
    // });

    // test('only admin and council could approve a task', async () => {
    //     const taskService = new TaskService(DB, {
    //         user : user.admin
    //     });

    //     const task: any = await taskService.create(global.DB.TASK_1);

    //     let rs: any = await taskService.approve({id : task._id});
    //     expect(rs.ok).to.be.equal(1);

    //     const expectedError = 'Access Denied'
    //     const ts1 = new TaskService(DB, {user : user.member});
    //     // member role could not approve
    //     try{
    //         await ts1.approve({id : task._id})
    //         assert.fail(`Should fail with ${expectedError}`)
    //     } catch (err) {
    //         expect(err).to.be.equal(expectedError)
    //     }
    // });

    test('Should approve event with upfront over budget and create an ELA owed transaction', async () => {
        // expect user to be admin/council
    })

    test('Should veto task proposal as council', async () => {
        // expect user to be council
    })

    test('Should set task to SUCCESS as admin/council', async () => {
        // expect user to be admin/council

        // expect ELA transactions for task & all sub-tasks to be logged

        // expect EVP to be logged for all allocations
    })

    test('Approval should send an email to owner + all admins too', async () => {
        // TODO
    })
})

afterAll(async () => {
    await DB.disconnect()
})

