let e35_teams = {
    "1": { name: "Marketing", category: 'ESSENTIAL' },
    "2": { name: "Product", category: 'ESSENTIAL' },
    "3": { name: "Support", category: 'ESSENTIAL' },
    "4": { name: "QA", category: 'ESSENTIAL' },
    "5": { name: "Writing", category: 'ESSENTIAL' },
    "6": { name: "Media", category: 'ESSENTIAL' },
    "7": { name: "dApp Analyst", category: 'ESSENTIAL' },
    "8": { name: "Administration", category: 'ESSENTIAL' },
    "9": { name: "Project Management", category: 'ADVANCED' },
    "10": { name: "Design", category: 'ADVANCED' },
    "11": { name: "dApp Consultant", category: 'ADVANCED' },
    "13": { name: "Operations", category: 'ADVANCED' },
    "14": { name: "Legal", category: 'SERVICES' },
    "15": { name: "Security", category: 'SERVICES' },
    "16": { name: "Translation", category: 'SERVICES' },
    "17": { name: "HR", category: 'SERVICES' },
    "18": { name: "Finance", category: 'SERVICES' },
    "19": { name: "Business Development", category: 'SERVICES' },
    "20": { name: "Partnership", category: 'SERVICES' },
    "21": { name: "Investment", category: 'SERVICES' },
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
