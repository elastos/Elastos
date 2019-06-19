import Base from './Base'
import {Document, Types} from 'mongoose'
import * as _ from 'lodash'
import {constant} from '../constant'
import {validate, utilCrypto, mail} from '../utility'
import * as moment from 'moment'
// import UserService from "./UserService";

const ObjectId = Types.ObjectId

const restrictedFields = {
    update: [
        '_id',
        'taskId',
        'status',
        'password'
    ]
}

const sanitize = '-password -salt -email -resetToken'

// TODO: we need some sort of status -> status permitted map

export default class extends Base {

    /**
     * TODO: We must ensure that bids are secret, and unless you are an admin we don't return the
     * bids on the task
     *
     * @param param
     * @returns {Promise<"mongoose".Document>}
     */
    public async show(param): Promise<Document> {
        const db_task = this.getDBModel('Task')
        const db_task_candidate = this.getDBModel('Task_Candidate')
        const db_user = this.getDBModel('User')
        const db_team = this.getDBModel('Team')

        const task = await db_task.getDBInstance().findOne({_id: param.taskId})
            .populate('candidates', sanitize)
            .populate('subscribers', sanitize)
            .populate('createdBy', sanitize)
            .populate('approvedBy', sanitize)
            .populate('community')
            .populate('communityParent')
            .populate('circle')

        if (task) {
            for (let subscriber of task.subscribers) {
                await db_user.getDBInstance().populate(subscriber, {
                    path: 'user',
                    select: sanitize
                })
            }

            for (let comment of task.comments) {
                for (let thread of comment) {
                    await db_task.getDBInstance().populate(thread, {
                        path: 'createdBy',
                        select: sanitize
                    })
                }
            }

            for (let candidate of task.candidates) {
                await db_user.getDBInstance().populate(candidate, {
                    path: 'user',
                    select: sanitize
                })
                await db_team.getDBInstance().populate(candidate, {
                    path: 'team'
                })

                if (candidate.team) {
                    await db_user.getDBInstance().populate(candidate.team, {
                        path: 'owner',
                        select: sanitize
                    })

                    await db_team.getDBInstance().populate(candidate.team, ['members'])

                    /* // should be user_team populate
                    for (let member of candidate.team.members) {
                        await db_team.getDBInstance().populate(member, {
                            path: 'team',
                            select: sanitize
                        })

                        await db_user.getDBInstance().populate(member, {
                            path: 'user',
                            select: sanitize
                        })
                    }
                    */
                }

                for (let comment of candidate.comments) {
                    for (let thread of comment) {
                        await db_task.getDBInstance().populate(thread, {
                            path: 'createdBy',
                            select: sanitize
                        })
                    }
                }
            }

            await this.markLastSeenComment(task, task.createdBy, db_task)
        }

        return task
    }

    public async markCandidateVisited(param): Promise<Document> {
        const {taskCandidateId, owner} = param

        const db_task_candidate = this.getDBModel('Task_Candidate')
        const updateObj = owner
            ? {lastSeenByOwner: new Date()}
            : {lastSeenByCandidate: new Date()}
        await db_task_candidate.update({_id: taskCandidateId}, updateObj)

        const updatedTask = db_task_candidate.findById(taskCandidateId)
        return updatedTask
    }

    // unused
    public async markComplete(param): Promise<Document> {
        const {taskCandidateId} = param

        const db_task_candidate = this.getDBModel('Task_Candidate')
        const updateObj = {complete: true}
        await db_task_candidate.update({_id: taskCandidateId}, updateObj)

        const updatedTask = db_task_candidate.findById(taskCandidateId)
        return updatedTask
    }

    public async list(param): Promise<Document> {
        const db_task = this.getDBModel('Task')
        const db_task_candidate = this.getDBModel('Task_Candidate')
        const db_user = this.getDBModel('User')
        const db_team = this.getDBModel('Team')
        const db_user_team = this.getDBModel('User_Team')

        const cursor = db_task.getDBInstance().find(_.omit(param, ['results', 'page', 'sortBy', 'sortOrder']))

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

        const tasks = await cursor

        if (tasks.length) {
            for (const task of tasks) {
                await db_task.getDBInstance().populate(task, {
                    path: 'createdBy',
                    select: sanitize,
                })

                await db_task.getDBInstance().populate(task, {
                    path: 'approvedBy',
                    select: sanitize,
                })

                await db_task.getDBInstance().populate(task, [
                    'community',
                    'communityParent',
                ])

                await db_task.getDBInstance().populate(task, {
                    path: 'candidates',
                    select: sanitize,
                })

                await db_team.getDBInstance().populate(task, {
                    path: 'circle',
                    select: sanitize,
                })

                for (const subscriber of task.subscribers) {
                    await db_user.getDBInstance().populate(subscriber, {
                        path: 'user',
                        select: sanitize
                    })
                }

                for (const comment of task.comments) {
                    for (const thread of comment) {
                        await db_task.getDBInstance().populate(thread, {
                            path: 'createdBy',
                            select: sanitize
                        })
                    }
                }

                for (const candidate of task.candidates) {
                    await db_task_candidate.getDBInstance().populate(candidate, {
                        path: 'user',
                        select: sanitize
                    })

                    await db_task_candidate.getDBInstance().populate(candidate, ['team'])

                    if (candidate.team) {
                        await db_team.getDBInstance().populate(candidate.team, ['members'])
                    }

                }
            }
        }

        return tasks
    }

    /**
     * This also handles creating sub tasks, if it's sub task
     * the parentTaskId must be set, and the user must be owner of the parent task
     *
     * Also check that the ELA reward is less than the parent task reward (v1.5)
     *
     * @param param
     * @returns {Promise<"mongoose".Document>}
     */
    public async create(param): Promise<Document> {

        const {
            name, description, descBreakdown, goals, circle,
            thumbnail, infoLink, community, communityParent, category, type, startTime, endTime,
            candidateLimit, candidateSltLimit, rewardUpfront, reward, assignSelf,

            applicationDeadline, completionDeadline,

            eventDateRange, eventDateRangeStart, eventDateRangeEnd, eventDateStatus,
            location,

            attachment, attachmentType, attachmentFilename, isUsd, readDisclaimer,

            domain, recruitedSkillsets, pictures, pitch, bidding, referenceBid
        } = param
        this.validate_name(name)
        this.validate_description(description)
        this.validate_type(type)
        // this.validate_reward_ela(reward_ela);
        // this.validate_reward_votePower(reward_votePower);

        let status = constant.TASK_STATUS.CREATED

        if (rewardUpfront.ela > 0 || reward.ela > 0 || rewardUpfront.usd > 0 || reward.usd > 0) {
            // there is ELA / USD involved so we start in PENDING unless we are an admin
            if (this.currentUser.role !== constant.USER_ROLE.ADMIN) {
                status = constant.TASK_STATUS.PENDING
            } else {
                status = constant.TASK_STATUS.APPROVED
            }

        } else {
            // if there is no ELA and you are assigning yourself,
            // it'll automatically go to APPROVED
            if (assignSelf) {
                status = constant.TASK_STATUS.APPROVED
            }
        }

        const doc = {
            name, description, descBreakdown, goals, infoLink, category, type, circle,
            startTime,
            endTime,
            thumbnail,
            domain,
            recruitedSkillsets,
            pictures,
            readDisclaimer,

            applicationDeadline, completionDeadline,

            eventDateRange, eventDateRangeStart, eventDateRangeEnd, eventDateStatus,
            location,
            bidding,
            referenceBid,

            attachment, attachmentType, attachmentFilename,
            candidateLimit,
            candidateSltLimit,
            pitch,
            rewardUpfront: rewardUpfront,
            reward: reward,
            assignSelf: assignSelf,
            status: status,
            createdBy: this.currentUser._id
        }
        if (community) {
            doc['community'] = community
        }

        if (communityParent) {
            doc['communityParent'] = communityParent
        }

        if (assignSelf) {
            // override the candidate select limit
            // TODO: visually note this in UI
            doc.candidateLimit = 1
            doc.candidateSltLimit = 1
        }

        const db_task = this.getDBModel('Task')

        // console.log('create task => ', doc);
        const task = await db_task.save(doc)

        this.sendCreateEmail(this.currentUser, task)

        // if assignSelf = true, we add self as the candidate
        if (assignSelf) {
            await this.addCandidate({taskId: task._id, userId: this.currentUser._id, assignSelf: true})
        }

        if (circle) {
            // Notify all users of the corresponding circle about the new task.
            this.sendNewCircleTaskNotification(circle, task)
        }

        return task
    }

    /**
     * Changing a task's reward/upfront after approval is only allowed by admins/council
     *
     * TODO: move status change triggers to a separate function
     *
     * TODO: no security to check if u own the task if you are leader
     *
     * @param param
     * @returns {Promise<boolean>}
     */
    public async update(param): Promise<boolean> {

        // not really assigning fields,
        const {
            taskId, rewardUpfront, reward
        } = param

        // we need to set this for the end of the fn so we have the updated task
        let sendTaskPendingRequiredApprovalEmail = false

        if (param.status === constant.TASK_STATUS.ASSIGNED) {
            throw 'Assigned Status is Deprecated'
        }

        // organizer cannot change task to these statuses
        if (this.currentUser.role === constant.USER_ROLE.LEADER) {

            if ([
                constant.TASK_STATUS.DISTRIBUTED,
                constant.TASK_STATUS.CANCELED,
                constant.TASK_STATUS.APPROVED

            ].includes(param.status)) {
                throw 'Access Denied - Status'
            }

        }

        // start logic
        const db_task = this.getDBModel('Task')
        const db_user = this.getDBModel('User')

        // get current
        const task = await db_task.findById(taskId)
        const taskOwner = await db_user.findById(task.createdBy)

        await db_task.getDBInstance().populate(task, {
            path: 'candidates',
            select: sanitize,
        })

        // flags for email actions later, maybe move this to an aspect or post call handler
        let sendTaskMarkedAsCompleteEmail = false

        // permission shortcuts
        if (this.currentUser.role === constant.USER_ROLE.MEMBER) {

            // there is an exception for members,
            // if they are the task assignee they can mark the task as complete only
            if (!_.includes([constant.TASK_STATUS.CREATED, constant.TASK_STATUS.PENDING], task.status)) {

                // only permit this to continue if there is only taskId and status fields on param, and status = SUBMITTED
                if (this.isTaskAssignee(task) && _.keys(param).length === 2 && !!param.taskId && param.status === constant.TASK_STATUS.SUBMITTED) {

                    // TODO: send an email notifying the owner/admin the task is marked complete
                    sendTaskMarkedAsCompleteEmail = true

                    // continue
                }
                else {
                    throw 'Access Denied - Owners cant edit tasks past PENDING'
                }

            } else if (this.currentUser._id.toString() !== task.createdBy.toString()) {
                throw 'Access Denied'
            }
        }

        // TODO: ensure reward cannot change if status APPROVED or after

        // explictly copy over fields, do not accept param as is
        const updateObj: any = _.omit(param, restrictedFields.update)

        // only allow approving these fields if user is admin

        // TODO: there are likely bugs here since owners are admins as well as organizers
        // but the same logic should execute for both and we are not doing that

        // TODO: we need a state diagram and a helper for this
        if (this.currentUser.role === constant.USER_ROLE.ADMIN) {

            if (param.status) {

                updateObj.status = param.status

                if (param.status === constant.TASK_STATUS.APPROVED) {

                    updateObj.approvedBy = this.currentUser._id
                    updateObj.approvedDate = new Date()

                    // if APPROVED we also consider if we are setting assignSelf now
                    let flagNotifyAssignPlusApprove = false

                    if (!task.assignSelf && param.assignSelf === true) {
                        await this.addCandidate({taskId: task._id, userId: task.createdBy, assignSelf: true})
                        flagNotifyAssignPlusApprove = true
                    }

                    // TODO: move this to agenda/queue
                    await this.sendTaskApproveEmail(this.currentUser, taskOwner, task, flagNotifyAssignPlusApprove)
                }
            }

        } else {
            const hasReward = task.reward.usd > 0 || task.rewardUpfront.usd > 0
            const willHaveReward = (rewardUpfront && rewardUpfront.usd > 0) ||
                (reward && reward.usd > 0)

            // Status should only change if reward changed from 0 to > 0
            if (!hasReward && willHaveReward) {
                updateObj.status = constant.TASK_STATUS.PENDING
                sendTaskPendingRequiredApprovalEmail = true
            }
        }

        // if you're the owner - applies for admins and organizers - TODO: revisit for security, should check if role is higher than MEMBER
        if (this.currentUser._id.toString() === task.createdBy.toString()) {

            // shortcut with error for these - only allow status change from APPROVED -> SUBMITTED
            // still need to handle some tasks that are assigned stage
            // TODO: update all ASSIGNED status tasks
            if (task.status !== constant.TASK_STATUS.APPROVED && task.status !== constant.TASK_STATUS.ASSIGNED &&
                param.status === constant.TASK_STATUS.SUBMITTED
            ) {
                throw 'Invalid Action'
            }

            // TODO: what is this, going backwards? - maybe just the email?
            // these status changes are only allowed if we are changing to APPROVED/SUBMITTED status
            if (task.status !== constant.TASK_STATUS.PENDING && task.status !== constant.TASK_STATUS.CREATED &&
                (
                    param.status === constant.TASK_STATUS.SUBMITTED ||
                    param.status === constant.TASK_STATUS.APPROVED
                )
            ) {
                updateObj.status = param.status

                if (param.status === constant.TASK_STATUS.SUBMITTED) {
                    await this.sendTaskSuccessEmail(taskOwner, task)
                }
            }
        }

        // TODO: check if user is approved candidate
        // TODO: accept as complete should not be allowed unless at least one candidate has submitted
        if (param.status === constant.TASK_STATUS.SUBMITTED) {
            updateObj.status = constant.TASK_STATUS.SUBMITTED
        }

        await db_task.update({_id: taskId}, updateObj)

        let updatedTask = await db_task.findById(taskId)

        // post update checks
        // TODO: if reward changed to 0, force status to CREATED

        if (sendTaskPendingRequiredApprovalEmail) {
            this.sendTaskPendingEmail(this.currentUser, updatedTask)
        }

        if (sendTaskMarkedAsCompleteEmail) {
            this.sendTaskMarkedAsCompleteEmail(taskOwner, this.currentUser, updatedTask)
        }

        return updatedTask
    }

    /**
     * We should only ever set deleted flag
     *
     * @param param
     * @returns {Promise<boolean>}
     */
    public async remove(param): Promise<boolean> {
        return true
    }

    /**
     * Only an admin/council role may approve the task
     *
     * @param param
     * @returns {Promise<boolean>}
     */
    public async approve(param): Promise<any> {
        const {id} = param

        const role = this.currentUser.role
        if (!_.includes([constant.USER_ROLE.ADMIN, constant.USER_ROLE.COUNCIL], role)) {
            throw 'Access Denied'
        }

        const db_task = this.getDBModel('Task')
        const rs = await db_task.update({_id: id}, {
            $set: {
                status: constant.TASK_STATUS.APPROVED
            }
        })
        console.log('approve task =>', rs)
        return rs
    }

    public async updateCandidate(param): Promise<boolean> {
        const {taskCandidateId, user, team, attachment, attachmentFilename, bid} = param
        const candidateSelector = {
            _id: param.taskCandidateId
        }
        const updateObj: any = {}

        if (user) {
            updateObj.user = user
        }

        if (team) {
            updateObj.team = team
        }

        if (attachment) {
            updateObj.attachment = attachment
        }

        if (attachmentFilename) {
            updateObj.attachmentFilename = attachmentFilename
        }

        if (bid || bid === 0) {
            updateObj.bid = bid
        }

        if (user || team) {
            updateObj.type = user
                ? constant.TASK_CANDIDATE_TYPE.USER
                : constant.TASK_CANDIDATE_TYPE.TEAM
        }

        const db_tc = this.getDBModel('Task_Candidate')
        if (!await db_tc.findOne(candidateSelector)) {
            throw 'Candidate not found'
        }

        await db_tc.update(candidateSelector, updateObj)

        const taskCandidate = await db_tc.getDBInstance().findOne(candidateSelector)
            .populate('user', sanitize)
            .populate('team', sanitize)

        if (taskCandidate.team) {
            const db_team = this.getDBModel('Team')
            await db_team.db.populate(taskCandidate.team, {
                path: 'owner',
                select: sanitize
            })
            await db_team.db.populate(taskCandidate.team, {
                path: 'members',
                select: sanitize
            })
        }

        return taskCandidate
    }

    /*
    * candidate could be user or team
    *
    * */
    public async addCandidate(param): Promise<boolean> {
        const {teamId, userId, taskId, applyMsg, assignSelf, attachment, attachmentFilename, bid} = param
        const doc: any = {
            task: taskId,
            applyMsg,
            attachment,
            attachmentFilename,
            bid
        }
        const db_user = this.getDBModel('User')

        if (teamId) {
            doc.team = teamId
            const db_team = this.getDBModel('Team')
            const team = await db_team.findOne({_id: teamId})
            if (!team) {
                throw 'invalid team id'
            }
            doc.type = constant.TASK_CANDIDATE_TYPE.TEAM
        }
        else if (userId) {
            doc.user = userId
            const user = await db_user.findOne({_id: userId})
            if (!user) {
                throw 'invalid user id'
            }
            doc.type = constant.TASK_CANDIDATE_TYPE.USER
        }
        else {
            throw 'no user id and team id'
        }

        const db_tc = this.getDBModel('Task_Candidate')
        if (await db_tc.findOne(doc)) {
            throw 'candidate already exists'
        }

        doc.status = constant.TASK_CANDIDATE_STATUS.PENDING

        // if we are assigning ourselves we automatically set to APPROVED
        if (assignSelf) {
            doc.status = constant.TASK_CANDIDATE_STATUS.APPROVED
        }

        const db_task = this.getDBModel('Task')
        const task = await db_task.findOne({_id: taskId})
        if (!task) {
            throw 'invalid task id'
        }

        console.log('add task candidate =>', doc)
        const taskCandidate = await db_tc.save(doc)

        // add the candidate to the task too
        if (task.candidates && task.candidates.length) {
            task.candidates.push(taskCandidate._id)
        } else {
            task.candidates = [taskCandidate._id]
        }

        await task.save()

        await db_tc.getDBInstance().populate(taskCandidate, {
            path: 'user',
            select: sanitize
        })

        await db_tc.getDBInstance().populate(taskCandidate, {
            path: 'team',
            select: sanitize
        })

        if (taskCandidate.team) {
            const db_ut = this.getDBModel('User_Team')
            await db_user.getDBInstance().populate(taskCandidate.team, {
                path: 'owner',
                select: sanitize
            })

            await db_ut.getDBInstance().populate(taskCandidate.team, {
                path: 'members',
                select: sanitize
            })
        }

        // send the email - first get the task owner
        if (!assignSelf) {
            const taskOwner = await db_user.findById(task.createdBy)

            await this.sendAddCandidateEmail(this.currentUser, taskOwner, task)
        }

        return taskCandidate
    }

    public async register(param): Promise<boolean> {
        const {userId, taskId} = param
        const doc: any = {
            task: taskId,
            category: constant.TASK_CANDIDATE_CATEGORY.RSVP,
            status: constant.TASK_CANDIDATE_STATUS.APPROVED
        }
        const db_user = this.getDBModel('User')

        if (userId) {
            doc.user = userId
            const user = await db_user.findOne({_id: userId})
            if (!user) {
                throw 'invalid user id'
            }
            doc.type = constant.TASK_CANDIDATE_TYPE.USER
        } else {
            throw 'no user id'
        }

        const db_tc = this.getDBModel('Task_Candidate')
        if (await db_tc.findOne(doc)) {
            throw 'candidate already exists'
        }

        const db_task = this.getDBModel('Task')
        const task = await db_task.findOne({_id: taskId})
        if (!task) {
            throw 'invalid task id'
        }

        console.log('register task candidate =>', doc)
        const taskCandidate = await db_tc.save(doc)

        // add the candidate to the task too
        if (task.candidates && task.candidates.length) {
            task.candidates.push(taskCandidate._id)
        } else {
            task.candidates = [taskCandidate._id]
        }

        await task.save()

        // populate the taskCandidate
        await db_tc.db.populate(taskCandidate, {
            path: 'user',
            select: sanitize
        })

        await db_tc.db.populate(taskCandidate, {
            path: 'team',
            select: sanitize
        })

        const taskOwner = await db_user.findById(task.createdBy)
        await this.sendAddCandidateEmail(this.currentUser, taskOwner, task)

        return taskCandidate
    }

    /**
     * You can either be the candidate or the task owner, or admin/council
     * @param param
     * @returns {Promise<boolean>}
     */
    public async removeCandidate(param): Promise<boolean> {
        const {taskId, taskCandidateId} = param

        const db_task = this.getDBModel('Task')
        const db_tc = this.getDBModel('Task_Candidate')

        let task = await db_task.getDBInstance().findOne({_id: taskId})
            .populate('createdBy', sanitize)
        let doc = await db_tc.findOne({_id: taskCandidateId})

        if (this.currentUser.role !== constant.USER_ROLE.ADMIN &&
            this.currentUser.role !== constant.USER_ROLE.COUNCIL &&
            (taskCandidateId && this.currentUser._id.toString() !== (_.get(doc, 'user._id', '')).toString()) &&
            (task.createdBy && task.createdBy._id.toString() !== this.currentUser._id.toString())) {
            throw 'Access Denied'
        }

        // TODO: check max applicants

        doc = {
            _id: taskCandidateId
        }

        await db_tc.remove(doc)

        task = await db_task.findOne({_id: taskId})
        if (!task) {
            throw 'invalid task id'
        }

        const result = await db_task.db.update({
            _id: task._id
        }, {
            $pull: {
                candidates: new ObjectId(taskCandidateId)
            }
        })

        console.log('remove task candidate =>', doc)

        return result

        /*
        // TODO: add this back in for permission checks
        const doc: any = {
            taskId,
            applyMsg
        };
        if(teamId){
            doc.teamId = teamId;
            doc.type = constant.TASK_CANDIDATE_TYPE.TEAM;`
        }
        else if(userId){
            doc.userId = userId;
            doc.type = constant.TASK_CANDIDATE_TYPE.USER;
        }
        else{
            throw 'no user id and team id';
        }

        const db_task = this.getDBModel('Task');
        const task = await db_task.findOne({_id: taskId});
        if(!task){
            throw 'invalid task id';
        }

        const role = this.currentUser.role;
        const cid = this.currentUser._id;

        // TODO add check for team
        if(!(
            _.includes([constant.USER_ROLE.ADMIN, constant.USER_ROLE.COUNCIL], role) ||
            cid === task.createdBy ||
            (userId && userId === cid)
        )){
            throw 'no permission to remove candidate';
        }

        const db_tc = this.getDBModel('Task_Candidate');
        */
    }

    public async deregister(param): Promise<boolean> {
        const {taskId, taskCandidateId} = param

        const db_task = this.getDBModel('Task')
        const db_tc = this.getDBModel('Task_Candidate')

        let task = await db_task.getDBInstance().findOne({_id: taskId})
            .populate('createdBy', sanitize)
        let doc = await db_tc.findOne({_id: taskCandidateId})

        if (this.currentUser.role !== constant.USER_ROLE.ADMIN &&
            this.currentUser.role !== constant.USER_ROLE.COUNCIL &&
            (taskCandidateId && this.currentUser._id.toString() !== doc.user._id.toString()) &&
            (task.createdBy && task.createdBy._id.toString() !== this.currentUser._id.toString())) {
            throw 'Access Denied'
        }

        doc = {
            _id: taskCandidateId
        }

        await db_tc.remove(doc)

        task = await db_task.findOne({_id: taskId})
        if (!task) {
            throw 'invalid task id'
        }

        const result = await db_task.db.update({
            _id: task._id
        }, {
            $pull: {
                candidates: new ObjectId(taskCandidateId)
            }
        })

        console.log('remove task candidate =>', doc)

        return result
    }

    /**
     * approve a candidate by admin or organizer
     *
     *
     */
    public async acceptCandidate(param): Promise<boolean> {
        const db_task = this.getDBModel('Task')
        const db_tc = this.getDBModel('Task_Candidate')

        let doc = await db_tc.findById(param.taskCandidateId)
        let task = await db_task.getDBInstance().findOne({_id: doc.task})
            .populate('createdBy', sanitize)

        if (this.currentUser.role !== constant.USER_ROLE.ADMIN &&
            (task.createdBy && task.createdBy._id.toString() !== this.currentUser._id.toString())) {
            throw 'Access Denied'
        }

        await db_tc.update({
            _id: param.taskCandidateId
        }, {
            status: constant.TASK_CANDIDATE_STATUS.APPROVED
        })

        doc = await db_tc.getDBInstance().findById(param.taskCandidateId)
            .populate('user')
        task = await db_task.getDBInstance().findOne({_id: doc.task})
            .populate({
                path: 'candidates',
                populate: {
                    path: 'user'
                }
            })

        let acceptedCnt = 0
        let users = []
        let usersWonBidding = []
        let usernameWonString = ''
        for (let candidate of task.candidates) {
            if (candidate.status === constant.TASK_CANDIDATE_STATUS.APPROVED) {
                acceptedCnt = +1
                usersWonBidding.push(candidate.user)
                usernameWonString += `${candidate.user.profile.firstName} ${candidate.user.profile.lastName}`
                if (task.candidates.length !== acceptedCnt) {
                    usernameWonString += ', '
                }
            }

            if (candidate.status !== constant.TASK_CANDIDATE_STATUS.APPROVED) {
                users.push(candidate.user)
            }
        }

        if (task.bidding || acceptedCnt >= task.candidateSltLimit) {
            await db_task.update({
                _id: task._id
            }, {
                status: constant.TASK_STATUS.APPROVED
            })

            this.sendWonBiddingEmail(usersWonBidding, task)
            this.sendLostBiddingEmail(users, task, doc, usernameWonString)
        }

        // TODO: remove unaccepted candidates and send them emails

        return await db_task.findById(task._id)
    }

    public async rejectCandidate(param): Promise<boolean> {
        const {taskCandidateId} = param
        const db_task = this.getDBModel('Task')
        const db_tc = this.getDBModel('Task_Candidate')

        let doc = await db_tc.findById(taskCandidateId)
        let task = await db_task.getDBInstance().findOne({_id: doc.task})
            .populate('createdBy', sanitize)

        if (this.currentUser.role !== constant.USER_ROLE.ADMIN &&
            (task.createdBy && task.createdBy._id.toString() !== this.currentUser._id.toString())) {
            throw 'Access Denied'
        }

        await db_tc.update({
            _id: taskCandidateId
        }, {
            status: constant.TASK_CANDIDATE_STATUS.REJECTED
        })

        return db_tc.getDBInstance().findOne({_id: taskCandidateId})
            .populate('team')
            .populate('user', sanitize)
    }

    public async withdrawCandidate(param): Promise<Document> {
        const {taskCandidateId} = param
        const db_task = this.getDBModel('Task')
        const db_tc = this.getDBModel('Task_Candidate')

        let doc = await db_tc.findById(taskCandidateId)
        await db_tc.db.populate(doc, ['team'])

        await db_tc.remove({
            _id: taskCandidateId
        })

        const task = await db_task.getDBInstance().findOne({_id: doc.task})
        const result = await db_task.db.update({
            _id: task._id
        }, {
            $pull: {
                candidates: new ObjectId(taskCandidateId)
            }
        })

        return result
    }

    public validate_name(name) {
        if (!validate.valid_string(name, 4)) {
            throw 'invalid task name'
        }
    }

    public validate_description(description) {
        if (!validate.valid_string(description, 1)) {
            throw 'invalid task description'
        }
    }

    public validate_type(type) {
        if (!type) {
            throw 'task type is empty'
        }
        if (!_.includes(constant.TASK_TYPE, type)) {
            throw 'task type is not valid'
        }
    }

    public validate_reward_ela(ela) {
        // TODO check current user has enough ela or not.
    }

    public validate_reward_votePower(votePower) {
        // TODO check current user has enough votePower or not.
    }

    /**
     * Returns all task candidates that match the user id
     *
     * @param userId
     */
    public async getCandidatesForUser(userId, status) {
        const db_task_candidate = this.getDBModel('Task_Candidate')

        let options: any = {
            user: userId
        }

        if (!_.isEmpty(status)) {
            options.status = status
        }

        return db_task_candidate.list(options)
    }

    /**
     * Returns all task candidates that match the team id
     *
     * @param teamId
     */
    public async getCandidatesForTeam(teamId, status) {
        const db_task_candidate = this.getDBModel('Task_Candidate')

        let options: any = {
            team: teamId
        }

        if (!_.isEmpty(status)) {
            options.status = status
        }

        return db_task_candidate.list(options)
    }

    /**
     * This is PENDING status if there is ELA > 0
     *
     * @param curUser
     * @param task
     * @returns {Promise<void>}
     */
    public async sendCreateEmail(curUser, task) {

        let subject = 'New Task Created: ' + task.name
        let body = `${this.currentUser.profile.firstName} ${this.currentUser.profile.lastName} has created the task ${task.name}`

        if (task.status === constant.TASK_STATUS.PENDING) {
            subject = 'ACTION REQUIRED: ' + subject
            body += ` and it requires approval`
        }

        body += `<br/>
            <br/>
            <a href="${process.env.SERVER_URL}/task-detail/${task._id}">Click here to view the ${task.type.toLowerCase()}</a>
            `


        const adminUsers = await this.getAdminUsers()

        for (let admin of adminUsers) {
            await mail.send({
                to: admin.email,
                toName: `${admin.profile.firstName} ${admin.profile.lastName}`,
                subject: subject,
                body: body
            })
        }
    }

    public async sendTaskPendingEmail(curUser, task) {

        let subject = 'Task ELA Reward Changed: ' + task.name
        let body = `${this.currentUser.profile.firstName} ${this.currentUser.profile.lastName} has changed the ELA reward for task ${task.name}`

        if (task.status === constant.TASK_STATUS.PENDING) {
            subject = 'ACTION REQUIRED: ' + subject
            body += ` and it requires approval
                    <br/>
                    <br/>
                    <a href="${process.env.SERVER_URL}/task-detail/${task._id}">Click here to view the ${task.type.toLowerCase()}</a>
                    `
        }

        const adminUsers = await this.getAdminUsers()

        for (let admin of adminUsers) {
            await mail.send({
                to: admin.email,
                toName: `${admin.profile.firstName} ${admin.profile.lastName}`,
                subject: subject,
                body: body
            })
        }
    }

    public async sendTaskMarkedAsCompleteEmail(taskOwner, curUser, task) {

        if (taskOwner._id.toString() === curUser._id.toString()) {
            return
        }

        let subject = 'Task ' + task.name + ' Marked as Complete'

        let body = `${this.currentUser.profile.firstName} ${this.currentUser.profile.lastName} has marked the task ${task.name} as complete.
            <br/>
            <br/>
            Please verify the task was completed properly and accept it: <a href="${process.env.SERVER_URL}/task-detail/${task._id}">Click here to view the ${task.type.toLowerCase()}</a>
        `
        await mail.send({
            to: taskOwner.email,
            toName: `${taskOwner.profile.firstName} ${taskOwner.profile.lastName}`,
            subject: subject,
            body: body
        })

    }

    public async sendWonBiddingEmail(users, task) {

        let candidateSubject = `Your application for task ${task.name} has been approved`
        let candidateBody = `Congratulations, you have won the bidding ${task.name}, you can get started.`

        for (let user of users) {
            await mail.send({
                to: user.email,
                toName: `${user.profile.firstName} ${user.profile.lastName}`,
                subject: candidateSubject,
                body: candidateBody
            })
        }
    }

    public async sendLostBiddingEmail(users, task, taskCandidate, usernameWonString) {

        let candidateSubject = `Your application for task ${task.name} has lost the bid`
        let candidateBody = `${usernameWonString} won the bid at ${taskCandidate.bid} ELA, but don't worry you can bid next time.`

        for (let user of users) {
            await mail.send({
                to: user.email,
                toName: `${user.profile.firstName} ${user.profile.lastName}`,
                subject: candidateSubject,
                body: candidateBody
            })
        }
    }

    // this email goes to all candidates
    public async sendTaskAssignedEmail(taskOwner, doc) {

    }

    // this email goes to all candidates
    public async sendTaskSuccessEmail(taskOwner, doc) {

    }

    /**
     * There are two emails - one to the applicant and the other to the task owner
     *
     * @param curUser
     * @param taskOwner
     * @param task
     * @returns {Promise<void>}
     */
    public async sendAddCandidateEmail(curUser, taskOwner, task) {

        let ownerSubject = `A candidate has applied for your task - ${task.name}`
        let ownerBody = `
            ${curUser.profile.firstName} ${curUser.profile.lastName} has applied for your task ${task.name}
            <br/>
            Please review their application
            <br/>
            <br/>
            <a href="${process.env.SERVER_URL}/profile/task-detail/${task._id}">Click here to view the ${task.type.toLowerCase()}</a>
            `
        let ownerTo = taskOwner.email
        let ownerToName = `${taskOwner.profile.firstName} ${taskOwner.profile.lastName}`

        await mail.send({
            to: ownerTo,
            toName: ownerToName,
            subject: ownerSubject,
            body: ownerBody
        })

        let candidateSubject = `Your application for task ${task.name} has been received`
        let candidateBody = `Thank you, the task owner ${taskOwner.profile.firstName} ${taskOwner.profile.lastName} will review your application and be in contact`
        let candidateTo = curUser.email
        let candidateToName = `${curUser.profile.firstName} ${curUser.profile.lastName}`

        await mail.send({
            to: candidateTo,
            toName: candidateToName,
            subject: candidateSubject,
            body: candidateBody
        })
    }

    public async sendNewCircleTaskNotification(id, task) {
        const db_team = this.getDBModel('Team')
        const db_user = this.getDBModel('User')
        const db_user_team = this.getDBModel('User_Team')

        const team = await db_team.findOne({
            _id: id,
            type: constant.TEAM_TYPE.CRCLE
        })

        if (team) {
            const userTeams = await db_user_team.find({_id: {$in: team.members}})
            const users = await db_user.find({_id: {$in: _.map(userTeams, 'user')}})
            const to = _.map(users, 'email')

            const formatUsername = (user) => {
                const firstName = user.profile && user.profile.firstName
                const lastName = user.profile && user.profile.lastName

                if (_.isEmpty(firstName) && _.isEmpty(lastName)) {
                    return user.username
                }

                return [firstName, lastName].join(' ')
            }

            const recVariables = _.zipObject(to, _.map(users, (target) => {
                return {
                    _id: target._id,
                    username: formatUsername(target)
                }
            }))

            const subject = `New CRcle Task has been created`
            const body = `
                <h2>Hello %recipient.username%,</h2>
                <br/>
                A new ${task.type} has been created under the ${team.name} CRcle you're a member of.
                <br/>
                <h3>
                ${task.name}
                </h3>
                <br/>
                <a href="${process.env.SERVER_URL}/profile/task-detail/${task._id}">Click here to view the ${task.type.toLowerCase()}</a>
                `

            await mail.send({
                to,
                subject: subject,
                body: body,
                recVariables
            })
        }
    }

    public async sendTaskApproveEmail(curUser, taskOwner, task, flagNotifyAssignPlusApprove) {

        let ownerSubject = `Your task proposal - ${task.name} has been approved`
        let ownerBody = `
            ${curUser.profile.firstName} ${curUser.profile.lastName} has approved your task proposal ${task.name}
            <br/>${flagNotifyAssignPlusApprove ? 'This task has also been assigned to you by the approver.<br/>' : ''}<br/>
            <a href="${process.env.SERVER_URL}/profile/task-detail/${task._id}">Click here to view the ${task.type.toLowerCase()}</a>
            `
        let ownerTo = taskOwner.email
        let ownerToName = `${taskOwner.profile.firstName} ${taskOwner.profile.lastName}`

        await mail.send({
            to: ownerTo,
            toName: ownerToName,
            subject: ownerSubject,
            body: ownerBody
        })
    }

    protected isTaskAssignee(task) {

        const taskAssigneeUserId = this.getTaskAssigneeUserId(task)
        let isTaskAssignee = false

        if (taskAssigneeUserId) {
            isTaskAssignee = taskAssigneeUserId.toString() === this.currentUser._id.toString()
        }

        return isTaskAssignee
    }

    // this assumes that task's candidate user is not populated
    protected getTaskAssigneeUserId(task) {
        const taskAssignee = _.filter(task.candidates, {status: constant.TASK_CANDIDATE_STATUS.APPROVED})

        if (taskAssignee.length) {
            return _.get(taskAssignee[0], 'user')
        }
    }

    protected async getAdminUsers() {
        const db_user = this.getDBModel('User')

        return db_user.find({
            role: constant.USER_ROLE.ADMIN,
            active: true
        })
    }


}
