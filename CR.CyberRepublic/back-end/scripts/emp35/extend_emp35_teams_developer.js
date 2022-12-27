let e35_teams = {
    "22": { name: "DevOps", category: 'DEVELOPER' },
    "23": { name: "Frontend", category: 'DEVELOPER' },
    "24": { name: "Backend", category: 'DEVELOPER' },
    "25": { name: "Testing", category: 'DEVELOPER' },
    "26": { name: "Technical Writing", category: 'DEVELOPER' }
};

for (let id in e35_teams) {
    db.teams.update({ name: e35_teams[id].name }, { $set: {
            profile: {
            description: ""
        },
        tags: [],
        domain: [],
        recruitedSkillsets: [],
        members: [],
        metadata: {},
        owner: null,
        pictures: [],
        comments: [],
        type: 'CRCLE',
        name: e35_teams[id].name,
        subcategory: e35_teams[id].category,
        eid: Number(id)
    } }, { upsert: true })
}
