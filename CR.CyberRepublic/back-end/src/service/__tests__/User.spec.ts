declare var global, describe, test, require, process, beforeAll, afterAll

import {expect, assert} from 'chai'

import {constant} from '../../constant';
import db from '../../db';
import '../../config';
import UserService from '../UserService';

let DB, userService

const _generateMember = (uid) => {
    let timeMs = new Date().valueOf()
    return {
        username: `user+dev.test.${timeMs}.${uid}@elastosjs.com`,
        password: 'elastos821',
        email: `user+dev.test.${timeMs}.${uid}@elastosjs.com`,
        firstName: 'John',
        lastName: 'Doe',
        country: 'ca',
        city: 'Vancouver'
    }
}

const member = _generateMember('1')
const anotherMember = _generateMember('2')

let resultMember, resultAnother

describe('Tests for User', () => {
    beforeAll(async () => {
        DB = await db.create();

        userService = new UserService(DB, {});

        resultMember = await userService.registerNewUser(member);
        expect(resultMember.role).to.be.equal(constant.USER_ROLE.MEMBER)
        resultAnother = await userService.registerNewUser(anotherMember);
        expect(resultAnother.role).to.be.equal(constant.USER_ROLE.MEMBER)
    })

    test('It should not be possible to change other users email', async () => {
        const expectedError = 'Access Denied'

        try {
            await userService.update({
                userId: resultAnother._id,
                email: 'foo@bar.com'
            })

            assert.fail(`Should fail with ${expectedError}`)
        } catch (err) {
            expect(err).to.be.equal(expectedError)
        }
    })

    test('It should not be possible to change other users password', async () => {
        const expectedError = 'Access Denied'

        try {
            await userService.changePassword({
                username: anotherMember.username,
                oldPassword: anotherMember.password,
                password: 'elastos832'
            })

            assert.fail(`Should fail with ${expectedError}`)
        } catch (err) {
            expect(err).to.be.equal(expectedError)
        }
    })

    test('It should not be possible to view other users in admin mode', async () => {
        const expectedError = 'Access Denied'

        try {
            await userService.show({
                userId: resultAnother._id,
                admin: true
            })

            assert.fail(`Should fail with ${expectedError}`)
        } catch (err) {
            expect(err).to.be.equal(expectedError)
        }
    })

    test('It should not be possible to view other users sensitive information', async () => {
        const sensitiveFields = ['salt', 'password']
        const result = await userService.show({
            userId: resultAnother._id
        })

        for (let field of sensitiveFields) {
            expect(result[field]).to.be.undefined
        }
    })

    test('It should not be possible to selfpromote to admin', async () => {

        const expectedError = 'Access Denied'

        const memberService = new UserService(DB, {
            user: resultMember
        })
        try {
            await memberService.updateRole({
                userId: resultMember._id,
                role: constant.USER_ROLE.ADMIN
            })

            assert.fail(`Should fail with ${expectedError}`)
        } catch (err) {
            expect(err).to.be.equal(expectedError)
        }

    })

    afterAll(async () => {
        await DB.disconnect()
    })
});
