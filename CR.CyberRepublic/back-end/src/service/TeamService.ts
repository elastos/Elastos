import Base from './Base'
import {Document, Types} from 'mongoose'
import * as _ from 'lodash'
import {validate, mail} from '../utility'
import {constant} from '../constant'
import LogService from './LogService'
import {DataList} from './interface'

const sanitize = '-password -salt -email'
const ObjectId = Types.ObjectId

const restrictedFields = {
    update: [
        '_id',
        'status',
        'password'
    ]
}


export default class extends Base {
    private model
    private ut_model
    protected init(){
        this.model = this.getDBModel('Team')
        this.ut_model = this.getDBModel('User_Team')
    }

    public async create(param): Promise<Document>{
        const db_team = this.getDBModel('Team')
        const db_user_team = this.getDBModel('User_Team')

        // validate
        this.validate_name(param.name)

        const doc = {
            name: param.name,
            domain: param.domain,
            type: param.type || constant.TEAM_TYPE.TEAM,
            metadata: this.param_metadata(param.metadata),
            tags: this.param_tags(param.tags),
            profile: {
                logo: param.logo,
                description: param.description
            },
            recruitedSkillsets: param.recruitedSkillsets,
            owner: this.currentUser,
            pictures: param.pictures
        }

        console.log('create team => ', doc)
        const res = await db_team.save(doc)
        const team = await db_team.findOne({_id: res._id})

        // save to user team
        const doc_user_team = {
            user: this.currentUser,
            team: res,
            status: constant.TEAM_USER_STATUS.NORMAL,
            role: constant.TEAM_ROLE.LEADER
        }

        console.log('create user_team => ', doc_user_team)
        const res1 = await db_user_team.save(doc_user_team)

        team.members = [ res1._id ]
        await team.save()

        await db_team.getDBInstance().populate(team, {
            path: 'owner',
            select: sanitize
        })

        return team
    }

    public async update(param): Promise<Document>{
        const {
            teamId
        } = param

        const db_team = this.getDBModel('Team')
        const team = await db_team.findById(teamId)
        await db_team.getDBInstance().populate(team, {
            path: 'owner',
            select: sanitize
        })

        if (this.currentUser._id.toString() !== team.owner._id.toString() &&
            this.currentUser.role !== constant.USER_ROLE.ADMIN) {
            throw 'Access Denied'
        }

        const doc = {
            name: param.name,
            domain: param.domain,
            type: param.type,
            metadata: this.param_metadata(param.metadata),
            tags: this.param_tags(param.tags),
            profile: {
                logo: param.logo,
                description: param.description
            },
            recruitedSkillsets: param.recruitedSkillsets,
            pictures: param.pictures
        }

        this.validate_name(doc.name)

        await db_team.update({_id: teamId}, doc)

        return db_team.findById(teamId)
    }

    public async deleteTeam(param): Promise<boolean> {
        const {teamId} = param

        if (!teamId) {
            throw 'no team id'
        }

        const db_team = this.getDBModel('Team')
        const team = await db_team.findOne({_id: teamId})
        if (!team) {
            throw 'invalid team id'
        }

        await db_team.getDBInstance().populate(team, {
            path: 'owner',
            select: sanitize
        })

        if (this.currentUser._id.toString() !== team.owner._id.toString() &&
            this.currentUser.role !== constant.USER_ROLE.ADMIN) {
            throw 'Access Denied'
        }

        const db_ut = this.getDBModel('User_Team')

        for (let member of team.members) {
            await db_ut.remove({_id: member})
        }

        const res = await db_team.remove({_id: teamId})

        return res
    }

    public async addCandidate(param): Promise<boolean>{
        const {teamId, userId, applyMsg} = param
        const doc: any = {
            teamId,
            team: teamId,
            userId,
            user: userId,
            apply_reason: applyMsg,
            role: constant.TEAM_ROLE.MEMBER,
            level: ''
        }
        const db_user = this.getDBModel('User')
        const db_team = this.getDBModel('Team')

        if (!teamId) {
            throw 'no team id'
        }

        doc.team = teamId
        const team = await db_team.findOne({_id: teamId})
        if (!team) {
            throw 'invalid team id'
        }

        if (!userId) {
            throw 'no user id'
        }

        doc.user = userId
        const user = await db_user.findOne({_id: userId})
        if (!user) {
            throw 'invalid user id'
        }

        const db_ut = this.getDBModel('User_Team')
        if (await db_ut.findOne(doc)) {
            throw 'candidate already exists'
        }

        const userTeams = await db_ut.find({ user: userId })
        const userCrcles = _.filter(userTeams, { type: constant.TEAM_TYPE.CRCLE })
        const MAX_USER_CIRCLES = 2

        if (_.size(userCrcles) >= MAX_USER_CIRCLES) {
            throw 'maximum number of CRcles reached'
        }

        // Accept CRcle applications automatically
        doc.status = team.type === constant.TEAM_TYPE.CRCLE
            ? constant.TEAM_USER_STATUS.NORMAL
            : constant.TEAM_USER_STATUS.PENDING

        console.log('add team candidate =>', doc)
        const teamCandidate = await db_ut.save(doc)

        // add the candidate to the team too
        team.members = team.members || []
        team.members.push(teamCandidate._id)

        await team.save()

        if (team.type === constant.TEAM_TYPE.CRCLE) {
            user.circles = user.circles || []
            user.circles.push(team._id)
            await user.save()
        }

        await db_ut.db.populate(teamCandidate, ['team', 'user'])

        if (teamCandidate.team) {
            await db_ut.db.populate(teamCandidate.team, ['owner'])
        }

        const teamOwner = await db_user.findById(team.owner)

        if (team.type !== constant.TEAM_TYPE.CRCLE) {
            await this.sendAddCandidateEmail(this.currentUser, teamOwner, team)
        }

        return teamCandidate
    }

    public async sendAddCandidateEmail(curUser, teamOwner, team) {
        let ownerSubject = `A candidate has applied for your team - ${team.name}`
        let ownerBody = `
            ${curUser.profile.firstName} ${curUser.profile.lastName} has applied for your team ${team.name}
            <br/>
            Please review their application
            <br/>
            <br/>
            <a href="${process.env.SERVER_URL}/profile/team-detail/${team._id}">Click here to view the team</a>
            `
        let ownerTo = teamOwner.email
        let ownerToName = `${teamOwner.profile.firstName} ${teamOwner.profile.lastName}`

        await mail.send({
            to: ownerTo,
            toName: ownerToName,
            subject: ownerSubject,
            body: ownerBody
        })

        let candidateSubject = `Your application for team ${team.name} has been received`
        let candidateBody = `Thank you, the team owner ${teamOwner.profile.firstName} ${teamOwner.profile.lastName} will review your application and be in contact`
        let candidateTo = curUser.email
        let candidateToName = `${curUser.profile.firstName} ${curUser.profile.lastName}`

        await mail.send({
            to: candidateTo,
            toName: candidateToName,
            subject: candidateSubject,
            body: candidateBody
        })
    }

    /*
    * only team owner or admin accept the apply request
    * */
    public async acceptApply(param): Promise<Document>{
        const {teamCandidateId} = param

        const db_team = this.getDBModel('Team')
        const db_ut = this.getDBModel('User_Team')

        let doc = await db_ut.findById(teamCandidateId)

        if (!doc || doc.status !== constant.TEAM_USER_STATUS.PENDING) {
            throw 'Invalid status'
        }

        let team = await db_team.getDBInstance().findOne({_id: doc.team})
            .populate('owner', sanitize)

        if (this.currentUser.role !== constant.USER_ROLE.ADMIN &&
            (team.owner && team.owner._id.toString() !== this.currentUser._id.toString())) {
            throw 'Access Denied'
        }

        await db_ut.update({
            _id: teamCandidateId
        }, {
            status: constant.TEAM_USER_STATUS.NORMAL
        })

        return db_ut.getDBInstance().findOne({_id: teamCandidateId})
            .populate('team')
            .populate('user', sanitize)
    }

    /*
    * only team owner or admin reject the apply request
    *
    * */
    public async rejectApply(param): Promise<Document>{
        const {teamCandidateId} = param

        const db_team = this.getDBModel('Team')
        const db_ut = this.getDBModel('User_Team')

        let doc = await db_ut.findById(teamCandidateId)

        if (!doc) {
            throw 'Invalid status'
        }

        let team = await db_team.getDBInstance().findOne({_id: doc.team})
            .populate('owner', sanitize)

        if (this.currentUser.role !== constant.USER_ROLE.ADMIN &&
            (team.owner && team.owner._id.toString() !== this.currentUser._id.toString())) {
            throw 'Access Denied'
        }

        await db_ut.update({
            _id: teamCandidateId
        }, {
            status: constant.TEAM_USER_STATUS.REJECTED
        })

        return db_ut.getDBInstance().findOne({_id: teamCandidateId})
            .populate('team')
            .populate('user', sanitize)
    }

    public async withdrawApply(param): Promise<Document>{
        const {teamCandidateId} = param

        const db_ut = this.getDBModel('User_Team')
        const db_team = this.getDBModel('Team')

        let doc = await db_ut.findById(teamCandidateId)

        if (!doc || doc.role === constant.TEAM_ROLE.OWNER) {
            throw 'Invalid status'
        }

        if (doc.user.toString() !== this.currentUser._id.toString()) {
            throw 'Access Denied'
        }

        await db_ut.remove({
            _id: teamCandidateId
        })

        const team = await db_team.getDBInstance().findOne({_id: doc.team})

        if (team.type === constant.TEAM_TYPE.CRCLE) {
            const db_user = this.getDBModel('User')
            await db_user.db.update({
                _id: doc.user
            }, {
                $pull: {
                    circles: new ObjectId(team._id)
                }
            })
        }

        const result = await db_team.db.update({
            _id: team._id
        }, {
            $pull: {
                members: new ObjectId(teamCandidateId)
            }
        })

        return result
    }

    /*
    * get whole team data
    * include all of members
    *
    * */
    public async show(param): Promise<Document>{
        const {teamId, status} = param
        const db_team = this.getDBModel('Team')
        const db_user = this.getDBModel('User')

        const team = await db_team.getDBInstance().findOne({_id : teamId})
            .populate('members', sanitize)
            .populate('owner', sanitize)

        if (team) {
            for (let member of team.members) {
                await db_team.getDBInstance().populate(member, {
                    path: 'team',
                    select: sanitize
                })

                await db_user.getDBInstance().populate(member, {
                    path: 'user',
                    select: sanitize
                })

                for (let comment of member.comments) {
                    for (let thread of comment) {
                        await db_user.getDBInstance().populate(thread, {
                            path: 'createdBy',
                            select: sanitize
                        })
                    }
                }
            }

            for (let comment of team.comments) {
                for (let thread of comment) {
                    await db_user.getDBInstance().populate(thread, {
                        path: 'createdBy',
                        select: sanitize
                    })
                }
            }
        }

        return team
    }

    /*
    * list teams
    *
    * */
    public async list(param): Promise<DataList>{
        const db_team = this.getDBModel('Team')
        const db_user = this.getDBModel('User')
        const query: any = {}

        if (param.archived) {
            query.archived = param.archived
        }

        if (param.name) {
            query.name = param.name
        }

        if (param.domain) {
            query.domain = { $in: param.domain.split(',') }
        }

        if (param.skillset) {
            query.recruitedSkillsets = { $in: param.skillset.split(',') }
        }

        if (param.owner) {
            query.owner = param.owner
        }

        if (param.type) {
            query.type = param.type
        }

        if (param.teamHasUser) {
            const db_user_team = this.getDBModel('User_Team')
            let listObj: any = {
                user: param.teamHasUser
            }

            if (param.teamHasUserStatus) {
                listObj.status = { $in: param.teamHasUserStatus.split(',') }
            }

            const userTeams = await db_user_team.list(listObj)
            query.$or = [
                { _id: {$in: _.map(userTeams, 'team')} }
            ]

            query.type = constant.TEAM_TYPE.TEAM
        }

        if (param.type) {
            query.type = param.type
        }

        const cursor = db_team.getDBInstance().find(query)

        if (param.sortBy) {
            const sortObject = {}
            sortObject[param.sortBy] = _.get(constant.SORT_ORDER, param.sortOrder, constant.SORT_ORDER.DESC)
            cursor.sort(sortObject)
        }

        if (param.results) {
            const results = parseInt(param.results, 10)
            const page = parseInt(param.page, 10)
            cursor.skip(results * (page - 1)).limit(results)
        }

        const teams = await cursor

        for (const team of teams) {
            await db_team.getDBInstance().populate(team, {
                path: 'owner',
                select: sanitize,
            })

            await db_team.getDBInstance().populate(team, ['members'])

            for (const comment of team.comments) {
                for (const thread of comment) {
                    await db_user.getDBInstance().populate(thread, {
                        path: 'createdBy',
                        select: sanitize
                    })
                }
            }

            if (param.includeTasks) {
                const db_task = this.getDBModel('Task')
                const tasks = await db_task.list({
                    circle: team,
                    status: constant.TASK_STATUS.APPROVED
                })

                const count = _.size(tasks)
                const budgetUsd = _.sum(_.map(tasks, (task) => task.reward.usd || 0))
                const budgetEla = _.sum(_.map(tasks, (task) => task.reward.ela || 0))

                team.tasks = {
                    count,
                    budget: {
                        usd: budgetUsd,
                        ela: budgetEla
                    }
                }
            }
        }

        return teams
    }

    public async listMember(param): Promise<Document[]>{
        const {teamId} = param
        const db_team = this.getDBModel('Team')
        const aggregate = db_team.getAggregate()

        const rs = await aggregate.match({_id : Types.ObjectId(teamId)})
            .unwind('$members')
            .lookup({
                from : 'users',
                localField : 'members.userId',
                foreignField : '_id',
                as : 'members.user'
            })
            .unwind('$members.user')
            .group({
                _id : '$_id',
                list : {
                    $push : '$members'
                }
            })
            .project({'list.user.password' : 0, 'list._id' : 0})

        return rs[0].list
    }

    public validate_name(name){
        if(!validate.valid_string(name, 4)){
            throw 'invalid team name'
        }
    }
    public param_metadata(meta: string){
        const rs = {}
        if(meta){
            const list = meta.split(',')

            _.each(list, (str)=>{
                const tmp = str.split('|')
                if(tmp.length === 2){
                    rs[tmp[0]] = tmp[1]
                }
            })
        }
        return rs
    }
    public param_tags(tags: string){
        let rs = []
        if(tags){
            rs = tags.split(',')
        }
        return rs
    }
}
