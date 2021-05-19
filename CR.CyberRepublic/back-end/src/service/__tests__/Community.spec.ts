declare var global, describe, test, expect, require, process, beforeAll, afterAll, sinon

import * as chai from 'chai'

const assert = chai.assert

import db from '../../db'
import '../../config'
import {constant} from '../../constant'

import CommunityService from '../CommunityService'
const testData: any = {}
let DB, communityService

beforeAll(async ()=>{
    // DB = await db.create()

    // communityService = new CommunityService(DB, {
    //     user: undefined
    // })
})

describe('Test for Community', () => {
    test('Should create community success as admin user', async ()=> {
    })
})
