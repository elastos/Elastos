
const {constant} = require('../../src/constant')

global.DB = {
    MEMBER_USER : {
        username: `test_member`,
        password: 'ebp12345',
        email: 'clarence+test_member@elastosjs.com',
        firstName: 'Hello',
        lastName: 'World',
        country: 'ca',
        city: 'Vancouver'
    },
    ORGANIZER_USER : {
        username: `test_organizer`,
        password: 'ebp12345',
        email: 'clarence+test_organizer@elastosjs.com',
        firstName: 'Clarence',
        lastName: 'Liu',
        country: 'ca',
        city: 'Vancouver'
    },
    COUNCIL_USER: {
        // _id: {"$oid": '5b28be2784f6f900350d30b9'},
        username: `kevinzhang`,
        password: 'ebp12345',
        email: 'clarence+kevinzhang@elastosjs.com',
        firstName: 'Kevin',
        lastName: 'Zhang',
        country: 'us',
        city: 'Fremont'
    },
    SECRETARY_USER: {
        username: `rebeccazhu`,
        password: 'ebp12345',
        email: 'rebeccazhu@elastosjs.com',
        firstName: 'Rebecca',
        lastName: 'Zhu',
        country: 'cn',
        city: 'Beijing'
    },
    ADMIN_USER : {
        username: 'ebpadmin'
    },

    TASK_1 : {
        name: 'Test Social Event',
        description: 'This is a test campaign, user must put their requirements and rewards here',
        category: constant.TASK_CATEGORY.SOCIAL,
        type: constant.TASK_TYPE.EVENT,
        candidateLimit: 10,
        candidateSltLimit: 3,
        reward: {
            ela: 1000
        },
        rewardUpfront: {
            ela: 0
        }
    },

    TASK_2 : {
        name: 'Test Social Task',
        description: 'This is a test task',
        category: constant.TASK_CATEGORY.SOCIAL,
        type: constant.TASK_TYPE.TASK,
        candidateLimit: 10,
        candidateSltLimit: 3,
        reward: {
            ela: 0
        },
        rewardUpfront: {
            ela: 750
        }
    },

    TEAM_1 : {
        name : 'test team 1',
        description : 'this is test team 1',
        type : 'DEVELOP',
        metadata : 'key|value,k1|v1',
        tags : 'key1,key2',
        logo : 'logo_url'
    },
    TEAM_UPDATE : {
        name : 'test team 1111',
        description : 'this is test team 1',
        tags : 'key1,key2',
        logo : 'logo_url111',
        recruiting : false
    }
}

require('./data/cvote')
require('./data/suggestion')
require('./data/permission')
require('./data/release')
require('./data/elip')
