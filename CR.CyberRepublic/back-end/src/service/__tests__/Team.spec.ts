declare var global, describe, test, expect, assert, require, process, beforeAll, afterAll;

import db from '../../db';
import '../../config';
import UserService from '../UserService';
import TeamService from '../TeamService';

const user: any = {};
let DB;
beforeAll(async ()=>{
    // DB = await db.create();
    // await DB.getModel('User').remove({
    //     role : 'MEMBER'
    // });

    // // create a test user as member role
    // const userService = new UserService(DB, {
    //     user: undefined
    // });
    // user.member = await userService.registerNewUser(global.DB.MEMBER_USER);
    // user.admin = await userService.getDBModel('User').findOne(global.DB.ADMIN_USER);

    // // empty team collection
    // await DB.getModel('Team').remove({});
});

describe('test for team', ()=>{
    test('create team and update team', async ()=>{
        // const teamService = new TeamService(DB, {
        //     user : user.member
        // });

        // // create team
        // const team: any = await teamService.create(global.DB.TEAM_1);
        // expect(team.owner).toBe(user.member._id);

        // // update team
        // const rs1: any = await teamService.update({
        //     ...global.DB.TEAM_UPDATE,
        //     id : team._id
        // });
        // expect(rs1.name).not.toBeundefined();

    });
});
