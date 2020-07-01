import Base from './Base'
import {Document} from 'mongoose'
import * as _ from 'lodash'
import {constant} from '../constant'
import {
    permissions,
    getDidPublicKey,
    getProposalState,
    getProposalData,
    getDidName,
    ela,
    getVoteResultByTxid
} from '../utility'
import * as moment from 'moment'
import * as jwt from 'jsonwebtoken'
import {
    mail,
    utilCrypto,
    user as userUtil,
    timestamp,
    logger
} from '../utility'
import {use} from 'chai'

const util = require('util')
const request = require('request')
const Big = require('big.js')
let tm = undefined

const BASE_FIELDS = [
    'title',
    'abstract',
    'goal',
    'motivation',
    'relevance',
    'budget',
    'budgetAmount',
    'elaAddress',
    'plan',
    'payment'
]

export const WALLET_STATUS_TO_CVOTE_STATUS = {
    ALL: [
        constant.CVOTE_STATUS.PROPOSED,
        constant.CVOTE_STATUS.NOTIFICATION,
        constant.CVOTE_STATUS.ACTIVE,
        constant.CVOTE_STATUS.FINAL,
        constant.CVOTE_STATUS.REJECT,
        constant.CVOTE_STATUS.DEFERRED,
        constant.CVOTE_STATUS.VETOED
    ],
    VOTING: [constant.CVOTE_STATUS.PROPOSED],
    NOTIFICATION: [constant.CVOTE_STATUS.NOTIFICATION],
    ACTIVE: [constant.CVOTE_STATUS.ACTIVE],
    FINAL: [constant.CVOTE_STATUS.FINAL],
    REJECTED: [constant.CVOTE_STATUS.REJECT, constant.CVOTE_STATUS.DEFERRED, constant.CVOTE_STATUS.VETOED]
}

export const CVOTE_STATUS_TO_WALLET_STATUS = {
    [constant.CVOTE_STATUS.PROPOSED]: 'VOTING',
    [constant.CVOTE_STATUS.NOTIFICATION]: 'NOTIFICATION',
    [constant.CVOTE_STATUS.ACTIVE]: 'ACTIVE',
    [constant.CVOTE_STATUS.FINAL]: 'FINAL',
    [constant.CVOTE_STATUS.REJECT]: 'REJECTED',
    [constant.CVOTE_STATUS.DEFERRED]: 'REJECTED',
    [constant.CVOTE_STATUS.VETOED]: 'VETOED'
}

const CHAIN_STATUS_TO_PROPOSAL_STATUS = {
    Registered: constant.CVOTE_STATUS.PROPOSED,
    CRAgreed: constant.CVOTE_STATUS.NOTIFICATION,
    CRCanceled: constant.CVOTE_STATUS.REJECT,
    VoterAgreed: constant.CVOTE_STATUS.ACTIVE,
    VoterCanceled: constant.CVOTE_STATUS.VETOED,
    Finished: constant.CVOTE_STATUS.FINAL,
    Aborted: {
        [constant.CVOTE_STATUS.PROPOSED]: constant.CVOTE_STATUS.REJECT,
        [constant.CVOTE_STATUS.NOTIFICATION]: constant.CVOTE_STATUS.VETOED
    }
}

const DID_PREFIX = 'did:elastos:'

export default class extends Base {
    // create a DRAFT propoal with minimal info
    public async createDraft(param: any): Promise<Document> {
        const db_suggestion = this.getDBModel('Suggestion')
        const db_cvote = this.getDBModel('CVote')
        const {title, proposedBy, proposer, suggestionId, payment} = param

        const vid = await this.getNewVid()
        const userRole = _.get(this.currentUser, 'role')
        if (!this.canCreateProposal()) {
            throw 'cvoteservice.create - no permission'
        }

        const doc: any = {
            title,
            vid,
            payment,
            status: constant.CVOTE_STATUS.DRAFT,
            published: false,
            contentType: constant.CONTENT_TYPE.MARKDOWN,
            proposedBy,
            proposer: proposer ? proposer : this.currentUser._id,
            createdBy: this.currentUser._id
        }
        const suggestion =
            suggestionId && (await db_suggestion.findById(suggestionId))
        if (!_.isEmpty(suggestion)) {
            doc.reference = suggestionId
        }

        Object.assign(doc, _.pick(suggestion, BASE_FIELDS))

        try {
            return await db_cvote.save(doc)
        } catch (error) {
            logger.error(error)
            return
        }
    }

    public async makeSuggIntoProposal(param: any) {
        const db_cvote = this.getDBModel('CVote')
        const db_suggestion = this.getDBModel('Suggestion')
        const db_user = this.getDBModel('User')
        const {suggestion, proposalHash, chainDid} = param
        const vid = await this.getNewVid()
        const [owner, creator] = await Promise.all([
            db_user.findById(suggestion.createdBy),
            db_user.findOne({'did.id': `did:elastos:${chainDid}`})
        ])
        const doc: any = {
            vid,
            type: suggestion.type,
            status: constant.CVOTE_STATUS.PROPOSED,
            published: true,
            contentType: constant.CONTENT_TYPE.MARKDOWN,
            proposedBy: userUtil.formatUsername(owner),
            proposer: suggestion.createdBy,
            createdBy: creator._id,
            reference: suggestion._id,
            proposalHash,
            draftHash: suggestion.draftHash,
            ownerPublicKey: suggestion.ownerPublicKey
        }

        Object.assign(doc, _.pick(suggestion, BASE_FIELDS))

        const councilMembers = await db_user.find({
            role: constant.USER_ROLE.COUNCIL
        })
        const voteResult = []
        doc.proposedAt = Date.now()
        _.each(councilMembers, (user) =>
            voteResult.push({
                votedBy: user._id,
                value: constant.CVOTE_RESULT.UNDECIDED
            })
        )
        doc.voteResult = voteResult
        doc.voteHistory = voteResult

        try {
            const res: any = await db_cvote.save(doc)
            await db_suggestion.update(
                {_id: suggestion._id},
                {
                    $addToSet: {reference: res._id},
                    $set: {tags: []},
                    proposalHash
                }
            )
            this.notifySubscribers(res)
            this.notifyCouncil(res)
            return { _id: res._id, vid: res.vid }
        } catch (error) {
            logger.error(error)
            return
        }
    }

    /**
     *
     * @param param
     * @returns {Promise<"mongoose".Document>}
     */
    public async updateDraft(param: any): Promise<Document> {
        const db_cvote = this.getDBModel('CVote')
        const {
            _id,
            title,
            type,
            abstract,
            goal,
            motivation,
            relevance,
            budget,
            plan,
            payment
        } = param

        if (!this.currentUser || !this.currentUser._id) {
            throw 'cvoteservice.update - invalid current user'
        }

        if (!this.canManageProposal()) {
            throw 'cvoteservice.update - not council'
        }

        const cur = await db_cvote.findOne({_id})
        if (!cur) {
            throw 'cvoteservice.update - invalid proposal id'
        }

        const doc: any = {
            contentType: constant.CONTENT_TYPE.MARKDOWN
        }

        if (title) doc.title = title
        if (type) doc.type = type
        if (abstract) doc.abstract = abstract
        if (goal) doc.goal = goal
        if (motivation) doc.motivation = motivation
        if (relevance) doc.relevance = relevance
        if (budget) {
            doc.budget = budget
        }
        if (plan) doc.plan = plan
        if (payment) doc.payment = payment

        try {
            await db_cvote.update({_id}, doc)
            const res = await this.getById(_id)
            return res
        } catch (error) {
            logger.error(error)
            return
        }
    }

    // delete draft proposal by proposal id
    public async deleteDraft(param: any): Promise<any> {
        try {
            const db_cvote = this.getDBModel('CVote')
            const {_id} = param
            const doc = await db_cvote.findOne({_id})
            if (!doc) {
                throw 'cvoteservice.deleteDraft - invalid proposal id'
            }
            if (doc.status !== constant.CVOTE_STATUS.DRAFT) {
                throw 'cvoteservice.deleteDraft - not draft proposal'
            }
            return await db_cvote.remove({_id})
        } catch (error) {
            logger.error(error)
        }
    }

    public async create(param): Promise<Document> {
        const db_cvote = this.getDBModel('CVote')
        const db_user = this.getDBModel('User')
        const db_suggestion = this.getDBModel('Suggestion')
        const {
            title,
            published,
            proposedBy,
            proposer,
            suggestionId,
            abstract,
            goal,
            motivation,
            relevance,
            budget,
            plan,
            payment
        } = param

        const vid = await this.getNewVid()
        const status = published
            ? constant.CVOTE_STATUS.PROPOSED
            : constant.CVOTE_STATUS.DRAFT

        const doc: any = {
            title,
            vid,
            status,
            published,
            contentType: constant.CONTENT_TYPE.MARKDOWN,
            proposedBy,
            abstract,
            goal,
            motivation,
            relevance,
            budget,
            plan,
            payment,
            proposer,
            createdBy: this.currentUser._id
        }

        const suggestion =
            suggestionId && (await db_suggestion.findById(suggestionId))
        if (!_.isEmpty(suggestion)) {
            doc.reference = suggestionId
        }

        const councilMembers = await db_user.find({
            role: constant.USER_ROLE.COUNCIL
        })
        const voteResult = []
        if (published) {
            doc.proposedAt = Date.now()
            _.each(councilMembers, (user) =>
                voteResult.push({
                    votedBy: user._id,
                    value: constant.CVOTE_RESULT.UNDECIDED
                })
            )
            doc.voteResult = voteResult
            doc.voteHistory = voteResult
        }

        try {
            const res = await db_cvote.save(doc)
            // add reference with suggestion
            if (!_.isEmpty(suggestion)) {
                await db_suggestion.update(
                    {_id: suggestionId},
                    {$addToSet: {reference: res._id}}
                )
                // notify creator and subscribers
                if (published) this.notifySubscribers(res)
            }

            // notify council member to vote
            if (published) this.notifyCouncil(res)

            return res
        } catch (error) {
            logger.error(error)
            return
        }
    }

    private async notifySubscribers(cvote: any) {
        const db_suggestion = this.getDBModel('Suggestion')
        const suggestionId = _.get(cvote, 'reference')
        if (!suggestionId) return
        const suggestion = await db_suggestion
            .getDBInstance()
            .findById(suggestionId)
            .populate('subscribers.user', constant.DB_SELECTED_FIELDS.USER.NAME_EMAIL)
            .populate('createdBy', constant.DB_SELECTED_FIELDS.USER.NAME_EMAIL)

        const councilMember = await this.getDBModel('User').findById(
            cvote.createdBy
        )

        // get users: creator and subscribers
        const toUsers = _.map(suggestion.subscribers, 'user') || []
        toUsers.push(suggestion.createdBy)
        const toMails = _.map(toUsers, 'email')

        // compose email object
        const subject = `The suggestion is referred in Proposal #${cvote.vid}`
        const body = `
      <p>Council member ${userUtil.formatUsername(
            councilMember
        )} has refer to your suggestion ${suggestion.title} in a proposal #${
            cvote.vid
        }.</p>
      <br />
      <p>Click this link to view more details:</p>
      <p><a href="${process.env.SERVER_URL}/proposals/${cvote._id}">${
            process.env.SERVER_URL
        }/proposals/${cvote._id}</a></p>
      <br /> <br />
      <p>Thanks</p>
      <p>Cyber Republic</p>
    `
        const recVariables = _.zipObject(
            toMails,
            _.map(toUsers, (user) => {
                return {
                    _id: user._id,
                    username: userUtil.formatUsername(user)
                }
            })
        )

        const mailObj = {
            to: toMails,
            // toName: ownerToName,
            subject,
            body,
            recVariables
        }

        // send email
        mail.send(mailObj)
    }

    private async notifyCouncil(cvote: any) {
        const db_user = this.getDBModel('User')
        const currentUserId = _.get(this.currentUser, '_id')
        const councilMembers = await db_user.find({
            role: constant.USER_ROLE.COUNCIL
        })
        const toUsers = _.filter(
            councilMembers,
            (user) => !user._id.equals(currentUserId)
        )
        const toMails = _.map(toUsers, 'email')

        const subject = `New Proposal: ${cvote.title}`
        const body = `
      <p>There is a new proposal added:</p>
      <br />
      <p>${cvote.title}</p>
      <br />
      <p>Click this link to view more details: <a href="${process.env.SERVER_URL}/proposals/${cvote._id}">${process.env.SERVER_URL}/proposals/${cvote._id}</a></p>
      <br /> <br />
      <p>Thanks</p>
      <p>Cyber Republic</p>
    `

        const recVariables = _.zipObject(
            toMails,
            _.map(toUsers, (user) => {
                return {
                    _id: user._id,
                    username: userUtil.formatUsername(user)
                }
            })
        )

        const mailObj = {
            to: toMails,
            // toName: ownerToName,
            subject,
            body,
            recVariables
        }

        mail.send(mailObj)
    }

    private async notifyCouncilToVote() {
        // find cvote before 1 day expiration without vote yet for each council member
        const db_cvote = this.getDBModel('CVote')
        const nearExpiredTime =
            Date.now() - (constant.CVOTE_EXPIRATION - constant.ONE_DAY)
        const unvotedCVotes = await db_cvote
            .getDBInstance()
            .find({
                proposedAt: {
                    $lt: nearExpiredTime,
                    $gt: Date.now() - constant.CVOTE_EXPIRATION
                },
                notified: {$ne: true},
                status: constant.CVOTE_STATUS.PROPOSED
            })
            .populate(
                'voteResult.votedBy',
                constant.DB_SELECTED_FIELDS.USER.NAME_EMAIL
            )

        _.each(unvotedCVotes, (cvote) => {
            _.each(cvote.voteResult, (result) => {
                if (result.value === constant.CVOTE_RESULT.UNDECIDED) {
                    // send email to council member to notify to vote
                    const {title, _id} = cvote
                    const subject = `Proposal Vote Reminder: ${title}`
                    const body = `
            <p>You only got 24 hours to vote this proposal:</p>
            <br />
            <p>${title}</p>
            <br />
            <p>Click this link to vote: <a href="${process.env.SERVER_URL}/proposals/${_id}">${process.env.SERVER_URL}/proposals/${_id}</a></p>
            <br /> <br />
            <p>Thanks</p>
            <p>Cyber Republic</p>
          `
                    const mailObj = {
                        to: result.votedBy.email,
                        toName: userUtil.formatUsername(result.votedBy),
                        subject,
                        body
                    }
                    mail.send(mailObj)

                    // update notified to true
                    db_cvote.update({_id: cvote._id}, {$set: {notified: true}})
                }
            })
        })
    }

    /**
     * List proposals, only an admin may request and view private records
     *
     * We expect the front-end to always call with {published: true}
     *
     * TODO: what's the rest way of encoding multiple values for a field?
     *
     * Instead of magic params, we should have just different endpoints I think,
     * this method should be as dumb as possible
     *
     * @param query
     * @returns {Promise<"mongoose".Document>}
     */
    public async list(param): Promise<Object> {
        const db_cvote = this.getDBModel('CVote')
        const currentUserId = _.get(this.currentUser, '_id')
        const userRole = _.get(this.currentUser, 'role')
        const query: any = {}
        if (!param.published) {
            if (!this.isLoggedIn() || !this.canManageProposal()) {
                throw 'cvoteservice.list - unpublished proposals only visible to council/secretary'
            } else if (
                param.voteResult === constant.CVOTE_RESULT.UNDECIDED &&
                permissions.isCouncil(userRole)
            ) {
                // get unvoted by current council
                query.voteResult = {
                    $elemMatch: {
                        value: constant.CVOTE_RESULT.UNDECIDED,
                        votedBy: currentUserId
                    }
                }
                query.published = true
                query.status = constant.CVOTE_STATUS.PROPOSED
            }
        } else {
            query.published = param.published
        }
        // createBy
        if (param.author && param.author.length) {
            let search = param.author
            const db_user = this.getDBModel('User')
            const pattern = search.split(' ').join('|')
            const users = await db_user
                .getDBInstance()
                .find({
                    $or: [
                        {username: {$regex: search, $options: 'i'}},
                        {'profile.firstName': {$regex: pattern, $options: 'i'}},
                        {'profile.lastName': {$regex: pattern, $options: 'i'}}
                    ]
                })
                .select('_id')
            const userIds = _.map(users, (el: { _id: string }) => el._id)
            query.createdBy = {$in: userIds}
        }
        // cvoteType
        if (
            param.type &&
            _.indexOf(_.values(constant.CVOTE_TYPE), param.type) >= 0
        ) {
            query.type = param.type
        }
        // startDate <  endDate
        if (
            param.startDate &&
            param.startDate.length &&
            param.endDate &&
            param.endDate.length
        ) {
            let endDate = new Date(param.endDate)
            endDate.setDate(endDate.getDate() + 1)
            query.createdAt = {
                $gte: new Date(param.startDate),
                $lte: endDate
            }
        }
        // Ends in times - 7day = startDate <  endDate
        if (
            param.endsInStartDate &&
            param.endsInStartDate.length &&
            param.endsInEndDate &&
            param.endsInEndDate.length
        ) {
            let endDate = new Date(
                new Date(param.endsInEndDate).getTime() - 7 * 24 * 3600 * 1000
            )
            endDate.setDate(endDate.getDate() + 1)
            query.createdAt = {
                $gte: new Date(
                    new Date(param.endsInStartDate).getTime() - 7 * 24 * 3600 * 1000
                ),
                $lte: endDate
            }
            query.status = {
                $in: [
                    constant.CVOTE_STATUS.PROPOSED,
                    constant.CVOTE_STATUS.ACTIVE,
                    constant.CVOTE_STATUS.REJECT,
                    constant.CVOTE_STATUS.NOTIFICATION,
                    constant.CVOTE_STATUS.FINAL,
                    constant.CVOTE_STATUS.DEFERRED,
                    constant.CVOTE_STATUS.INCOMPLETED
                ]
            }
        }
        // status
        if (param.status && constant.CVOTE_STATUS[param.status]) {
            query.status = param.status
        }
        // old data
        if (!param.old) {
            query.old = {$exists: false}
        }
        if (param.old) {
            query.old = true
        }
        // budget
        if (param.budgetLow || param.budgetHigh) {
            query.budgetAmount = {}
            if (param.budgetLow && param.budgetLow.length) {
                query.budgetAmount['$gte'] = parseInt(param.budgetLow)
            }
            if (param.budgetHigh && param.budgetHigh.length) {
                query.budgetAmount['$lte'] = parseInt(param.budgetHigh)
            }
        }
        // has tracking
        if (param.hasTracking) {
            const db_cvote_tracking = this.getDBModel('CVote_Tracking')
            const hasTracking = await db_cvote_tracking.find(
                {
                    status: constant.CVOTE_TRACKING_STATUS.REVIEWING
                },
                'proposalId'
            )
            let trackingProposals = []
            hasTracking.map(function (it) {
                trackingProposals.push(it.proposalId)
            })
            query._id = {
                $in: trackingProposals
            }
        }

        if (param.$or) query.$or = param.$or
        const fields = [
            'vid',
            'title',
            'type',
            'proposedBy',
            'status',
            'published',
            'proposedAt',
            'createdAt',
            'voteResult',
            'vote_map'
        ]

        // const list = await db_cvote.list(query, { vid: -1 }, 0, fields.join(' '))

        const cursor = db_cvote
            .getDBInstance()
            .find(query, fields.join(' '))
            .sort({vid: -1})

        if (param.results) {
            const results = parseInt(param.results, 10)
            const page = parseInt(param.page, 10)
            cursor.skip(results * (page - 1)).limit(results)
        }

        const rs = await Promise.all([
            cursor,
            db_cvote.getDBInstance().find(query).count()
        ])
        const list = rs[0]
        const total = rs[1]

        return {list, total}
    }

    /**
     *
     * @param param
     * @returns {Promise<"mongoose".Document>}
     */
    public async update(param): Promise<Document> {
        const db_user = this.getDBModel('User')
        const db_cvote = this.getDBModel('CVote')
        const {
            _id,
            published,
            notes,
            title,
            abstract,
            goal,
            motivation,
            relevance,
            budget,
            plan
        } = param

        if (!this.currentUser || !this.currentUser._id) {
            throw 'cvoteservice.update - invalid current user'
        }

        if (!this.canManageProposal()) {
            throw 'cvoteservice.update - not council'
        }

        const cur = await db_cvote.findOne({_id})
        if (!cur) {
            throw 'cvoteservice.update - invalid proposal id'
        }

        const doc: any = {
            contentType: constant.CONTENT_TYPE.MARKDOWN
        }
        const willChangeToPublish =
            published === true && cur.status === constant.CVOTE_STATUS.DRAFT

        if (title) doc.title = title
        if (abstract) doc.abstract = abstract
        if (goal) doc.goal = goal
        if (motivation) doc.motivation = motivation
        if (relevance) doc.relevance = relevance
        if (budget) {
            doc.budget = budget
        }
        if (plan) doc.plan = plan

        if (willChangeToPublish) {
            doc.status = constant.CVOTE_STATUS.PROPOSED
            doc.published = published
            doc.proposedAt = Date.now()
            const councilMembers = await db_user.find({
                role: constant.USER_ROLE.COUNCIL
            })
            const voteResult = []
            _.each(councilMembers, (user) =>
                voteResult.push({
                    votedBy: user._id,
                    value: constant.CVOTE_RESULT.UNDECIDED
                })
            )
            doc.voteResult = voteResult
            doc.voteHistory = voteResult
        }

        // always allow secretary to edit notes
        if (notes) doc.notes = notes
        try {
            await db_cvote.update({_id}, doc)
            const res = await this.getById(_id)
            if (willChangeToPublish) {
                this.notifyCouncil(res)
                this.notifySubscribers(res)
            }
            return res
        } catch (error) {
            logger.error(error)
            return
        }
    }

    public async finishById(id): Promise<any> {
        const db_cvote = this.getDBModel('CVote')
        const cur = await db_cvote.findOne({_id: id})
        if (!cur) {
            throw 'invalid proposal id'
        }
        if (!this.canManageProposal()) {
            throw 'cvoteservice.finishById - not council'
        }
        if (_.includes([constant.CVOTE_STATUS.FINAL], cur.status)) {
            throw 'proposal already completed.'
        }

        const rs = await db_cvote.update(
            {_id: id},
            {
                $set: {
                    status: constant.CVOTE_STATUS.FINAL
                }
            }
        )

        return rs
    }

    public async unfinishById(id): Promise<any> {
        const db_cvote = this.getDBModel('CVote')
        const cur = await db_cvote.findOne({_id: id})
        if (!cur) {
            throw 'invalid proposal id'
        }
        if (!this.canManageProposal()) {
            throw 'cvoteservice.unfinishById - not council'
        }
        if (
            _.includes(
                [constant.CVOTE_STATUS.FINAL, constant.CVOTE_STATUS.INCOMPLETED],
                cur.status
            )
        ) {
            throw 'proposal already completed.'
        }

        const rs = await db_cvote.update(
            {_id: id},
            {
                $set: {
                    status: constant.CVOTE_STATUS.INCOMPLETED
                }
            }
        )

        return rs
    }

    public async updateProposalBudget() {
        const { MILESTONE_STATUS, SUGGESTION_BUDGET_TYPE } = constant
        const db_cvote = this.getDBModel('CVote')
        const query = {
            old: { $exists: false },
            $or: [
                { status: 'ACTIVE' },
                {
                    status: 'FINAL',
                    budget: {
                        $elemMatch: {
                            type: SUGGESTION_BUDGET_TYPE.COMPLETION,
                            status: { $ne: MILESTONE_STATUS.WITHDRAWN }
                        }
                    }
                }
            ]
        }
        const proposals = await db_cvote.find(query)
        if (!proposals.length) {
            return
        }
        console.log('upb---proposal length---', proposals.length)
        const arr = []
        for (const proposal of proposals) {
            arr.push(this.updateMilestoneStatus(proposal))
        }
        await Promise.all(arr)
    }

    private async updateMilestoneStatus(proposal) {
        const {
            WITHDRAWN,
            WAITING_FOR_WITHDRAWAL,
            REJECTED,
            WAITING_FOR_APPROVAL
        } = constant.MILESTONE_STATUS
        const result = await getProposalData(proposal.proposalHash)
        if (!result) {
            return
        }
        let isStatusUpdated = false
        const status = _.get(result, 'status')
        if (status && status.toLowerCase() === 'finished') {
            proposal.status = constant.CVOTE_STATUS.FINAL
            isStatusUpdated = true
        }
        const budgets = _.get(result, 'data.proposal.budgets')
        let isBudgetUpdated = false
        if (budgets) {
            const budget = proposal.budget.map((item, index) => {
                const chainStatus = budgets[index].status.toLowerCase()
                if (chainStatus === 'withdrawn' && item.status === WAITING_FOR_WITHDRAWAL) {
                    isBudgetUpdated = true
                    return {...item, status: WITHDRAWN}
                }
                if (chainStatus === 'rejected' && item.status === WAITING_FOR_APPROVAL) {
                    isBudgetUpdated = true
                    this.notifyProposalOwner(
                        proposal.proposer,
                        this.rejectedMailTemplate(proposal.vid)
                    )
                    return {...item, status: REJECTED}
                }
                if (chainStatus === 'withdrawable' && item.status === WAITING_FOR_APPROVAL) {
                    isBudgetUpdated = true
                    this.notifyProposalOwner(
                        proposal.proposer,
                        this.approvalMailTemplate(proposal.vid)
                    )
                    return {...item, status: WAITING_FOR_WITHDRAWAL}
                }
                return item
            })
            if (isBudgetUpdated) {
                proposal.budget = budget
            }
        }
        if (isStatusUpdated || isBudgetUpdated) {
            console.log('ums---save proposal.vid---', proposal.vid)
            await proposal.save()
        }
    }

    public async getById(id): Promise<any> {
        const db_cvote = this.getDBModel('CVote')
        // access proposal by reference number
        const isNumber = /^\d*$/.test(id)
        let query: any
        if (isNumber) {
            query = {vid: parseInt(id)}
        } else {
            query = {_id: id}
        }
        const rs = await db_cvote
            .getDBInstance()
            .findOne(query, '-voteHistory')
            .populate(
                'voteResult.votedBy',
                constant.DB_SELECTED_FIELDS.USER.NAME_AVATAR
            )
            .populate('proposer', constant.DB_SELECTED_FIELDS.USER.NAME_EMAIL_DID)
            .populate('createdBy', constant.DB_SELECTED_FIELDS.USER.NAME_EMAIL_DID)
            .populate('reference', constant.DB_SELECTED_FIELDS.SUGGESTION.ID)
            .populate('referenceElip', 'vid')
        if (!rs) {
            return {success: true, empty: true}
        }

        if (rs.budgetAmount) {
            const doc = JSON.parse(JSON.stringify(rs))
            // deal with 7e-08
            doc.budgetAmount = Big(doc.budgetAmount).toFixed()
            return doc
        }
        return rs
    }

    public async getNewVid() {
        const db_cvote = this.getDBModel('CVote')
        // const n = await db_cvote.count({})
        // return n + 1
        // new version, vid string from 1
        const n = await db_cvote.count({old: {$exists: false}})
        return n + 1
    }

    public isExpired(data: any, extraTime = 0): Boolean {
        const ct = moment(data.proposedAt || data.createdAt).valueOf()
        if (Date.now() - ct - extraTime > constant.CVOTE_EXPIRATION) {
            return true
        }
        return false
    }

    public isCouncilExpired(data: any, extraTime = 0): Boolean {
        const ct = moment(data.proposedAt || data.createdAt).valueOf()
        if (Date.now() - ct - extraTime > constant.CVOTE_COUNCIL_EXPIRATION) {
            return true
        }
        return false
    }

    // proposal publicity
    public async isNotification(data): Promise<any> {
        const voteRejectAmount = data.votersrejectamount
        const registerHeight = data.registerheight
        const proportion = voteRejectAmount / registerHeight
        return proportion < 0.1
    }

    // proposal active/passed
    public isActive(data): Boolean {
        const supportNum =
            _.countBy(data.voteResult, 'value')[constant.CVOTE_RESULT.SUPPORT] || 0
        return supportNum > data.voteResult.length * 0.5
    }

    // proposal rejected
    public isRejected(data): Boolean {
        const rejectNum =
            _.countBy(data.voteResult, 'value')[constant.CVOTE_RESULT.REJECT] || 0
        return rejectNum > data.voteResult.length * 0.5
    }

    public async vote(param): Promise<Document> {
        const db_cvote = this.getDBModel('CVote')
        const {_id, value, reason} = param
        const cur = await db_cvote.findOne({_id})
        const votedBy = _.get(this.currentUser, '_id')
        if (!cur) {
            throw 'invalid proposal id'
        }
        const currentVoteResult = _.find(cur._doc.voteResult, ['votedBy', votedBy])
        const currentVoteHistory = cur._doc.voteHistory
        const currentVoteHistoryIndex = _.findLastIndex(currentVoteHistory, ['votedBy', votedBy])

        currentVoteHistory[currentVoteHistoryIndex] = {
            ..._.omit(currentVoteResult, ['_id'])
        }
        const reasonCreateDate = new Date()
        await db_cvote.update(
            {
                _id,
                'voteResult.votedBy': votedBy,
            },
            {
                $set: {
                    'voteResult.$.value': value,
                    'voteResult.$.reason': reason || '',
                    'voteResult.$.status': constant.CVOTE_CHAIN_STATUS.UNCHAIN,
                    'voteResult.$.reasonHash': utilCrypto.sha256D(reason + timestamp.second(reasonCreateDate)),
                    'voteResult.$.reasonCreatedAt': reasonCreateDate
                },
                $inc: {
                    __v: 1
                }
            }
        )
        await db_cvote.update(
            {
                _id,
                'voteHistory.votedBy': votedBy,
            },
            {
                $set: {
                    'voteHistory.$': {..._.omit(currentVoteResult, ['_id'])},
                },
                $inc: {
                    __v: 1
                }
            }
        )

        return await this.getById(_id)
    }

    public async updateNote(param): Promise<Document> {
        const db_cvote = this.getDBModel('CVote')
        const {_id, notes} = param

        const cur = await db_cvote.findOne({_id})
        if (!cur) {
            throw 'invalid proposal id'
        }
        if (!this.canManageProposal()) {
            throw 'cvoteservice.updateNote - not council'
        }
        if (this.currentUser.role !== constant.USER_ROLE.SECRETARY) {
            throw 'only secretary could update notes'
        }

        const rs = await db_cvote.update(
            {_id},
            {
                $set: {
                    notes: notes || ''
                }
            }
        )

        return await this.getById(_id)
    }

    public async cronJob() {
        if (tm) {
            return false
        }
        tm = setInterval(async () => {
            console.log('---------------- start cvote cronjob -------------')
            await this.pollProposal()

            // poll proposal status in chain
            // await this.pollProposalStatus()
            // council vote status in registered
            // await this.pollCouncilVoteStatus()
            // member vote status in agreed
            // await this.pollVotersRejectAmount()
        }, 1000 * 60 * 2)
    }

    private canManageProposal() {
        const userRole = _.get(this.currentUser, 'role')
        return permissions.isCouncil(userRole) || permissions.isSecretary(userRole)
    }

    private canCreateProposal() {
        const userRole = _.get(this.currentUser, 'role')
        return (
            !permissions.isCouncil(userRole) && !permissions.isSecretary(userRole)
        )
    }

    public async listcrcandidates(param) {
        const {pageNum, pageSize, state} = param

        let ret = null
        // url: 'http://54.223.244.60/api/dposnoderpc/check/listcrcandidates',
        const postPromise = util.promisify(request.post, {multiArgs: true})
        await postPromise({
            url:
                'https://unionsquare.elastos.org/api/dposnoderpc/check/listcrcandidates',
            form: {pageNum, pageSize, state},
            encoding: 'utf8'
        }).then((value) => (ret = value.body))

        return ret
    }

    // council vote onchain
    public async onchain(param) {
        try {
            const db_cvote = this.getDBModel('CVote')
            const userId = _.get(this.currentUser, '_id')
            const {id} = param

            const councilMemberDid = _.get(this.currentUser, 'did.id')
            if (!councilMemberDid) {
                return {success: false, message: 'this is not did'}
            }

            const role = _.get(this.currentUser, 'role')
            if (!permissions.isCouncil(role)) {
                return {success: false, message: 'member is no council'}
            }

            const cur = await db_cvote.findOne({_id: id})
            if (!cur) {
                return {success: false, message: 'not find proposal'}
            }

            const currentVoteResult: any = _.filter(cur.voteResult, (o: any) => userId.equals(o.votedBy))[0]

            const voteResultOnChain = {
                [constant.CVOTE_RESULT.SUPPORT]: 'approve',
                [constant.CVOTE_RESULT.REJECT]: 'reject',
                [constant.CVOTE_RESULT.ABSTENTION]: 'abstain',
            }

            const now = Math.floor(Date.now() / 1000)

            const jwtClaims = {
                iat: now,
                exp: now + (60 * 60 * 24),
                iss: process.env.APP_DID,
                command: 'reviewproposal',
                data: {
                    proposalHash: cur.proposalHash,
                    voteResult: voteResultOnChain[currentVoteResult.value],
                    opinionHash: currentVoteResult.reasonHash,
                    did: councilMemberDid
                }
            }

            const jwtToken = jwt.sign(JSON.stringify(jwtClaims), process.env.APP_PRIVATE_KEY, {
                algorithm: 'ES256'
            })
            const url = `elastos://crproposal/${jwtToken}`
            return {success: true, url}
        } catch (err) {
            logger.error(err)
            return {success: false}
        }
    }

    public async checkSignature(param: any) {
        const {id} = param
        const db_cvote = this.getDBModel('CVote')
        const userId = _.get(this.currentUser, '_id')
        const proposal = await db_cvote.getDBInstance().findOne({_id: id})
            .populate(
                'voteResult.votedBy',
                constant.DB_SELECTED_FIELDS.USER.NAME_AVATAR
            )
            .populate('proposer', constant.DB_SELECTED_FIELDS.USER.NAME_EMAIL_DID)
            .populate('createdBy', constant.DB_SELECTED_FIELDS.USER.NAME_EMAIL_DID)
            .populate('reference', constant.DB_SELECTED_FIELDS.SUGGESTION.ID)
            .populate('referenceElip', 'vid')
        if (proposal) {
            const voteResult = _.filter(proposal.voteResult, (o: any) => userId.equals(o.votedBy._id))[0]
            if (voteResult) {
                const signature = _.get(voteResult, 'signature.data')
                if (signature) {
                    return {success: true, data: proposal}
                }
                const message = _.get(voteResult, 'signature.message')
                if (message) {
                    return {success: false, message}
                }
            }
        } else {
            return {success: false}
        }
    }

    // update proposal information on the chain
    public async pollProposal() {
        const db_cvote = this.getDBModel('CVote')

        const list = await db_cvote.getDBInstance().find({
            proposalHash: {$exists: true},
            status: {
                $in: [
                    constant.CVOTE_STATUS.PROPOSED,
                    constant.CVOTE_STATUS.NOTIFICATION,
                ]
            }
        })

        const asyncForEach = async (array, callback) => {
            for (let index = 0; index < array.length; index++) {
                await callback(array[index], index, array);
            }
        }
        const rejectThroughAmount: any = (await ela.currentCirculatingSupply()) * 0.1
        await asyncForEach(list, async (o: any) => {
            const {proposalHash, status} = o._doc
            const rs: any = await getProposalData(proposalHash)
            if (!rs || rs.success === false) {
                return
            }
            switch (status) {
                case constant.CVOTE_STATUS.PROPOSED:
                    await this.updateProposalOnProposed({
                        rs,
                        _id: o._id,
                        status,
                    })
                    break;
                case constant.CVOTE_STATUS.NOTIFICATION:
                    await this.updateProposalOnNotification({
                        rs,
                        _id: o._id,
                        rejectThroughAmount
                    })
                    break;
            }
        })
    }

    public async updateProposalOnProposed(data: any) {
        const {rs, _id, status} = data
        const db_cvote = this.getDBModel('CVote')
        const {status: chainStatus} = rs
        const currentStatus = CHAIN_STATUS_TO_PROPOSAL_STATUS[chainStatus]

        if (status !== currentStatus) {
            await db_cvote.update({
                _id,
            }, {
                status: currentStatus,
            })
        }
    }

    public async updateProposalOnNotification(data: any) {
        const {WAITING_FOR_WITHDRAWAL, WAITING_FOR_REQUEST} = constant.MILESTONE_STATUS
        const db_cvote = this.getDBModel('CVote')
        const {rs, _id} = data
        let {rejectThroughAmount} = data
        const {
            data: {
                votersrejectamount: rejectAmount,
            },
            status: chainStatus
        } = rs

        const proposalStatus = CHAIN_STATUS_TO_PROPOSAL_STATUS[chainStatus]
        if (proposalStatus === constant.CVOTE_STATUS.ACTIVE) {
            const proposal = await db_cvote.findById(_id)
            const budget = proposal.budget.map((item: any) => {
                if (item.type === 'ADVANCE') {
                    return {...item, status: WAITING_FOR_WITHDRAWAL}
                } else {
                    return {...item, status: WAITING_FOR_REQUEST}
                }
            })
            await db_cvote.update({
                _id
            }, {
                $set: {
                    budget,
                    status: proposalStatus,
                    rejectAmount,
                    rejectThroughAmount
                }
            })
            return
        }
        switch (proposalStatus) {
            case constant.CVOTE_STATUS.VETOED:
                rejectThroughAmount = rejectAmount
                break;
            case constant.CVOTE_STATUS.NOTIFICATION:
                rejectThroughAmount = (await ela.currentCirculatingSupply()) * 0.1
                break;
        }

        await db_cvote.update({
            _id
        }, {
            status: proposalStatus,
            rejectAmount,
            rejectThroughAmount
        })
    }

    // member vote against
    public async memberVote(param): Promise<any> {
        try {
            const db_cvote = this.getDBModel('CVote')
            const {id} = param

            const cur = await db_cvote.findOne({_id: id})

            const now = Math.floor(Date.now() / 1000)
            const jwtClaims = {
                iat: now,
                exp: now + (60 * 60 * 24),
                iss: process.env.APP_DID,
                command: 'voteforproposal',
                data: {
                    proposalHash: cur.proposalHash
                }
            }

            const jwtToken = jwt.sign(jwtClaims, process.env.APP_PRIVATE_KEY, {
                algorithm: 'ES256'
            })
            const url = `elastos://crproposal/${jwtToken}`
            return {success: true, url}
        } catch (err) {
            logger.error(err)
            return {success: false}
        }
    }

    // member vote callback
    public async memberCallback(param): Promise<any> {
        return
    }

    /**
     * API to Wallet
     */
    public async allOrSearch(param): Promise<any> {
        const db_cvote = this.getDBModel('CVote')
        const query: any = {}

        if (
            !param.status ||
            !_.keys(WALLET_STATUS_TO_CVOTE_STATUS).includes(param.status)
        ) {
            return {
                code: 400,
                message: 'Invalid request parameters - status',
                // tslint:disable-next-line:no-null-keyword
                data: null
            }
        }

        // status
        query.status = WALLET_STATUS_TO_CVOTE_STATUS[param.status]

        // search
        if (param.search) {
            const search = _.trim(param.search)
            const db_user = this.getDBModel('User')
            const users = await db_user
                .getDBInstance()
                .find({
                    $or: [
                        {'did.didName': {$regex: _.trim(search), $options: 'i'}}
                    ]
                })
                .select('_id')
            const userIds = _.map(users, '_id')
            query.$or = [
                {title: {$regex: search, $options: 'i'}},
                {proposer: {$in: userIds}}
            ]
            if (_.isNumber(search)) {
                query.$or.push({vid: _.toNumber(search)})
            }
        }

        query.old = {$ne: true}

        const fields = [
            'vid',
            'title',
            'status',
            'createdAt',
            'proposer',
            'proposalHash'
        ]

        const cursor = db_cvote
            .getDBInstance()
            .find(query, fields.join(' '))
            .populate('proposer', constant.DB_SELECTED_FIELDS.USER.NAME_EMAIL_DID)
            .sort({vid: -1})

        if (
            param.page &&
            param.results &&
            parseInt(param.page) > 0 &&
            parseInt(param.results) > 0
        ) {
            const results = parseInt(param.results, 10)
            const page = parseInt(param.page, 10)

            cursor.skip(results * (page - 1)).limit(results)
        }

        const rs = await Promise.all([
            cursor,
            db_cvote.getDBInstance().find(query).count()
        ])

        // filter return data，add proposalHash to CVoteSchema
        const list = _.map(rs[0], function (o) {
            let temp = _.omit(o._doc, ['_id', 'proposer'])
            temp.proposedBy = _.get(o, 'proposer.did.didName')
            temp.status = CVOTE_STATUS_TO_WALLET_STATUS[temp.status]
            temp.createdAt = timestamp.second(temp.createdAt)
            return _.mapKeys(temp, function (value, key) {
                if (key == 'vid') {
                    return 'id'
                } else {
                    return key
                }
            })
        })

        const total = rs[1]
        return {list, total}
    }

    public async getProposalById(id): Promise<any> {
        const db_cvote = this.getDBModel('CVote')

        const fields = [
            'vid',
            'status',
            'abstract',
            'voteResult',
            'voteHistory',
            'createdAt',
            'proposalHash',
            'rejectAmount',
            'rejectThroughAmount',
        ]
        const proposal = await db_cvote
            .getDBInstance()
            .findOne({vid: id, old: {$ne: true}}, fields.join(' '))
            .populate('voteResult.votedBy', constant.DB_SELECTED_FIELDS.USER.NAME_EMAIL_DID)
            .populate('voteHistory.votedBy', constant.DB_SELECTED_FIELDS.USER.NAME_EMAIL_DID)

        if (!proposal) {
            return {
                code: 400,
                message: 'Invalid request parameters',
                // tslint:disable-next-line:no-null-keyword
                data: null
            }
        }

        const address = `${process.env.SERVER_URL}/proposals/${proposal.id}`

        const proposalId = proposal._id

        const voteResultFields = ['value', 'reason', 'votedBy', 'avatar']
        const voteHistoryList = _.groupBy(proposal._doc.voteHistory, 'votedBy._id')
        const voteResultWithNull = _.map(proposal._doc.voteResult, (o: any) => {
            let result
            if (o.status === constant.CVOTE_CHAIN_STATUS.CHAINED) {
                result = o._doc
            } else {
                const historyList = _.filter(voteHistoryList[o.votedBy._id], (e: any) => e.status === constant.CVOTE_CHAIN_STATUS.CHAINED)
                if (!_.isEmpty(historyList)) {
                    const history = _.sortBy(historyList, 'createdAt')
                    result = history[history.length - 1]._doc
                }
            }
            if (!_.isEmpty(result)) {
                return _.pick(
                    {
                        ...result,
                        votedBy: _.get(result, 'votedBy.did.didName'),
                        avatar: _.get(result, 'votedBy.did.avatar')
                    },
                    voteResultFields
                )
            }
        })
        const voteResult = _.filter(voteResultWithNull, (o: any) => !_.isEmpty(o))

        const tracking = await this.getTracking(proposalId)

        // const summary = await this.getSummary(proposalId)
        const summary = []

        const notificationResult = {}

        // duration
        const endTime = Math.round(proposal.createdAt.getTime() / 1000)
        const nowTime = Math.round(new Date().getTime() / 1000)

        if (proposal.status === constant.CVOTE_STATUS.PROPOSED) {
            notificationResult['duration'] = (endTime - nowTime + 604800) > 0 ? (endTime - nowTime + 604800) : 0
        }

        if (proposal.status === constant.CVOTE_STATUS.NOTIFICATION) {
            notificationResult['duration'] = (endTime - nowTime + 604800 * 2) > 0 ? (endTime - nowTime + 604800 * 2) : 0
        }

        if ([constant.CVOTE_STATUS.NOTIFICATION, constant.CVOTE_STATUS.VETOED].includes(proposal.status)
            && proposal.rejectAmount >= 0
            && proposal.rejectThroughAmount > 0) {
            notificationResult['rejectAmount'] = `${proposal.rejectAmount}`
            notificationResult['rejectThroughAmount'] = `${parseFloat(_.toNumber(proposal.rejectThroughAmount).toFixed(8))}`
            notificationResult['rejectRatio'] = _.toNumber((_.toNumber(proposal.rejectAmount) / _.toNumber(proposal.rejectThroughAmount)).toFixed(4))
        }

        return _.omit(
            {
                id: proposal.vid,
                status: CVOTE_STATUS_TO_WALLET_STATUS[proposal.status],
                abs: proposal.abstract,
                address,
                ..._.omit(proposal._doc, ['vid', 'abstract', 'rejectAmount', 'rejectThroughAmount', 'status', 'voteHistory']),
                ...notificationResult,
                createdAt: timestamp.second(proposal.createdAt),
                voteResult,
                tracking,
                summary
            },
            ['_id']
        )
    }

    public async getTracking(id) {
        const db_cvote = this.getDBModel('CVote')
        const db_user = this.getDBModel('User')
        const secretary = await db_user.getDBInstance().findOne({
            role: constant.USER_ROLE.SECRETARY,
            'did.id': 'did:elastos:igCSy8ht7yDwV5qqcRzf5SGioMX8H9RXcj'
        }, constant.DB_SELECTED_FIELDS.USER.NAME_EMAIL_DID)
        const propoal = await db_cvote.getDBInstance().findOne({_id: id}).populate('proposer', constant.DB_SELECTED_FIELDS.USER.NAME_EMAIL_DID)

        if (!propoal) {
            return
        }

        try {
            const didName = _.get(secretary, 'did.didName')
            const avatar = _.get(secretary, 'did.avatar')
            const ownerDidName = _.get(propoal, 'proposer.did.didName')
            const ownerAvatar = _.get(propoal, 'proposer.did.avatar') || _.get(propoal, 'proposer.profile.avatar')
            const {withdrawalHistory} = propoal
            const withdrawalList = _.filter(withdrawalHistory, (o: any) => o.milestoneKey !== '0')
            const withdrawalListByStage = _.groupBy(withdrawalList, 'milestoneKey')
            const keys = _.keys(withdrawalListByStage).sort().reverse()
            const result = _.map(keys, (k: any) => {
                const withdrawals = _.sortBy(withdrawalListByStage[`${k}`], 'createdAt')
                const withdrawal = withdrawals[withdrawals.length - 1]

                const comment = {}

                if (_.get(withdrawal, 'review.createdAt')) {
                    comment['content'] = _.get(withdrawal, 'review.reason')
                    comment['opinion'] = _.get(withdrawal, 'review.opinion')
                    comment['avatar'] = avatar
                    comment['createdBy'] = didName
                    comment['createdAt'] = moment(_.get(withdrawal, 'review.createdAt')).unix()
                }


                return {
                    stage: parseInt(k),
                    didName: ownerDidName,
                    avatar: ownerAvatar,
                    content: withdrawal.message,
                    createdAt: moment(withdrawal.createdAt).unix(),
                    comment
                }
            })

            return result
        } catch (err) {
            logger.error(err)
        }

    }

    public async getSummary(id) {
        const db_summary = this.getDBModel('CVote_Summary')
        const proposalId = id
        const querySummary: any = {
            proposalId
        }
        const fieldsSummary = [
            'comment',
            'content',
            'status',
            'createdAt',
            'updatedAt'
        ]
        const cursorSummary = db_summary
            .getDBInstance()
            .find(querySummary, fieldsSummary.join(' '))
            .populate('comment.createdBy', constant.DB_SELECTED_FIELDS.USER.NAME_EMAIL_DID)
            .sort({createdAt: -1})
        // const totalCursorSummary = db_summary.getDBInstance().find(querySummary).count()

        const summary = await cursorSummary
        // const totalSummary = await totalCursorSummary

        const list = _.map(summary, function (o) {
            const comment = o._doc.comment
            const contents = (JSON.parse(o.content))
            let content = ''
            _.each(contents.blocks, function (v: any, k: any) {
                content += v.text
                if (k !== (contents.blocks.length) - 1) {
                    content += '\n'
                }
            })
            const commentObj = {
                content: comment.content ? comment.content : null,
                createdBy: _.get(o, 'comment.createdBy.did.didName'),
                avatar: _.get(o, 'comment.createdBy.did.avatar')
            }
            const obj = {
                ...o._doc,
                comment: commentObj,
                content,
                createdAt: timestamp.second(o.createdAt),
                updatedAt: timestamp.second(o.updatedAt)
            }
            return _.pick(obj, fieldsSummary)
        })
        return list
    }

    public async getVotersRejectAmount(id) {
        try {
            const db_cvote = this.getDBModel('CVote')
            const cur = await db_cvote.find({_id: id})
            if (!cur) {
                throw 'this is not proposal'
            }
            const rs: any = await getProposalData(cur.proposalHash)
            if (!rs) {
                throw 'get one cr proposal crvotes by proposalhash is fail'
            }
            if (rs && rs.status === 'CRAgreed') {
                const {votersrejectamount, registerheight} = rs.data
                await db_cvote.update(
                    {_id: id},
                    {
                        $set: {
                            rejectAmount: votersrejectamount,
                            rejectThroughAmount: registerheight
                        }
                    }
                )
            }
            return {success: true, id}
        } catch (err) {
            logger.error(err)
            return
        }
    }

    public async temporaryChangeUpdateStatus() {
        const db_cvote = this.getDBModel('CVote')
        const proposaedList = await db_cvote.find({status: constant.CVOTE_STATUS.PROPOSED})
        const notificationList = await db_cvote.find({status: constant.CVOTE_STATUS.NOTIFICATION})
        const idsProposaed = []
        const idsNotification = []

        _.each(proposaedList, (item) => {
            idsProposaed.push(item._id)
            // this.proposalAborted(item.proposalHash)
        })
        _.each(notificationList, (item) => {
            idsNotification.push(item._id)
            // this.proposalAborted(item.proposalHash)
        })
        await db_cvote.update(
            {
                _id: {$in: idsProposaed}
            },
            {
                $set: {
                    status: constant.CVOTE_STATUS.REJECT
                }
            },
            {
                multi: true
            }
        )
        await db_cvote.update(
            {
                _id: {$in: idsNotification}
            },
            {
                $set: {
                    status: constant.CVOTE_STATUS.VETOED
                }
            },
            {
                multi: true
            }
        )
    }

    // update proposalHash status aborted
    // public async temporaryChangeUpdateStatus() {
    //     const db_cvote = this.getDBModel('CVote')
    //     const list = await db_cvote.find({
    //         $or: [
    //             {status: constant.CVOTE_STATUS.NOTIFICATION},
    //             {status: constant.CVOTE_STATUS.PROPOSED}
    //         ]
    //     })
    //     const idsAborted = []
    //     _.each(list, (item) => {
    //         idsAborted.push(item._id)
    //     })
    //    await db_cvote.update(
    //         {
    //             _id: {$in:idsAborted}
    //         },
    //         {
    //             $set:{
    //                 status: constant.CVOTE_STATUS.ABORTED
    //             }
    //         },
    //         {
    //             multi: true
    //         }
    //     )
    // }

    private rejectedMailTemplate(id: string) {
        const subject = `【Payment rejected】Your payment request is rejected by secretary`
        const body = `
        <p>One payment request in proposal #${id}  has been rejected.</p>
        <p>Click this link to view more details:</p>
        <p><a href="${process.env.SERVER_URL}/proposals/${id}">${process.env.SERVER_URL}/proposals/${id}</a></p>
        <br />
        <p>Cyber Republic Team</p>
        <p>Thanks</p>
        `
        return {subject, body}
    }

    private approvalMailTemplate(id: string) {
        const subject = `【Payment approved】Your payment request is approved by secretary`
        const body = `
        <p>One payment request in proposal ${id} has been approved, the ELA distribution will processed shortly, check your ELA wallet later.</p>
        <p>Click this link to view more details:</p>
        <p><a href="${process.env.SERVER_URL}/proposals/${id}">${process.env.SERVER_URL}/proposals/${id}</a></p>
        <br />
        <p>Cyber Republic Team</p>
        <p>Thanks</p>
        `
        return {subject, body}
    }

    private async notifyProposalOwner(
        proposer: any,
        content: {
            subject: string
            body: string
        }
    ) {
        const mailObj = {
            to: proposer.email,
            toName: userUtil.formatUsername(proposer),
            subject: content.subject,
            body: content.body
        }
        mail.send(mailObj)
    }

    public async updateVoteStatusByChain() {
        const db_ela = this.getDBModel('Ela_Transaction')
        const db_cvote = this.getDBModel('CVote')

        let elaVoteList = await db_ela.getDBInstance().find({type: constant.TRANSACTION_TYPE.COUNCIL_VOTE})
        if (_.isEmpty(elaVoteList)) {
            return
        }
        const elaVote = []
        const useIndex = []
        _.map(elaVoteList, (o: any) => {
            const data = {
                ...o._doc,
                ...JSON.parse(o.payload)
            }
            elaVote.push(data)
        })
        const query = []
        const byKeyElaList = _.keyBy(elaVote, 'proposalhash')
        _.forEach(byKeyElaList, (v: any, k: any) => {
            query.push(k)
        })
        const proposalList = await db_cvote.getDBInstance()
            .find({status: constant.CVOTE_STATUS.PROPOSED, proposalHash: {$in: query}})
            .populate('voteResult.votedBy', constant.DB_SELECTED_FIELDS.USER.NAME_EMAIL_DID)

        if (_.isEmpty(proposalList)) {
            return
        }
        const vote = []
        _.forEach(proposalList, (o: any) => {
            _.forEach(o.voteResult, (v: any) => {
                if (v.status === constant.CVOTE_CHAIN_STATUS.UNCHAIN) {
                    const oldReasonHash = v.reasonCreatedAt ?
                        utilCrypto.sha256D(v.reason + timestamp.second(v.reasonCreatedAt))
                        :
                        utilCrypto.sha256D(v.reason)
                    const reasonHash = v.reasonHash ? v.reasonHash : oldReasonHash
                    const data = {
                        proposalHash: o.proposalHash,
                        ...v._doc,
                        did: !_.isEmpty(v._doc.votedBy) ? v._doc.votedBy.did.id : null,
                        reasonHash
                    }
                    vote.push(data)
                }
            })
        })
        _.forEach(elaVote, async (o: any) => {
            const did: any = DID_PREFIX + o.did
            const voteList = _.find(vote, {'proposalHash': o.proposalhash, 'reasonHash': o.opinionhash, 'did': did})
            if (voteList) {
                const rs = await db_cvote.update({
                    'proposalHash': o.proposalhash,
                    'voteResult._id': voteList._id,
                }, {
                    $set: {
                        'voteResult.$.status': constant.CVOTE_CHAIN_STATUS.CHAINED
                    },
                    $inc: {
                        __v: 1
                    }
                })
                if (rs && rs.nModified == 1) {
                    await db_ela.remove({txid: o.txid})
                }

            }
        })
    }
}
