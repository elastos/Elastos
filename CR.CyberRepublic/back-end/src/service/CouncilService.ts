import Base from './Base'
import {constant} from '../constant'
import {CVOTE_STATUS_TO_WALLET_STATUS} from './CVoteService'
import {ela, logger, getInformationByDid, getDidName} from '../utility'
import * as moment from 'moment'

const _ = require('lodash')

const DID_PREFIX = 'did:elastos:'

let tm = undefined

export default class extends Base {
    private model: any
    private secretariatModel: any
    private userMode: any
    private proposalMode: any

    protected init() {
        this.model = this.getDBModel('Council')
        this.secretariatModel = this.getDBModel('Secretariat')
        this.userMode = this.getDBModel('User')
        this.proposalMode = this.getDBModel('CVote')
    }

    public async term(): Promise<any> {
        const fields = [
            'index',
            'startDate',
            'endDate',
            'status'
        ]

        const result = await this.model.getDBInstance().find({}, fields)


        return _.map(result, (o: any) => {
            let dateObj = {}
            if (o.status === constant.TERM_COUNCIL_STATUS.HISTORY) {
                dateObj['endDate'] = o.endDate && moment(o.endDate).unix()
            }
            return ({
                id: o._id,
                ..._.omit(o._doc, ['_id', 'startDate', 'endDate']),
                startDate: moment(o.startDate).unix(),
                ...dateObj,
            })
        })
    }

    public async councilList(id: number): Promise<any> {
        const fields = [
            'status',
            'councilMembers.did',
            'councilMembers.user.did',
            'councilMembers.location',
            'councilMembers.status',
        ]

        const secretariatFields = [
            'did',
            'user.did',
            'location',
            'startDate',
            'endDate',
            'status',
        ]

        const result = await this.model.getDBInstance()
            .findOne({index: id}, fields)
            .populate('councilMembers.user', constant.DB_SELECTED_FIELDS.USER.NAME_EMAIL_DID)

        const secretariatResult = await this.secretariatModel.getDBInstance()
            .find({}, secretariatFields).sort({'startDate': -1})
            .populate('user', constant.DB_SELECTED_FIELDS.USER.NAME_EMAIL_DID)

        if (!result) {
            return {
                code: 400,
                message: 'Invalid request parameters',
                // tslint:disable-next-line:no-null-keyword
                data: null
            }
        }

        const filterFields = (o: any) => {
            return _.omit(o, ['_id', 'user', 'startDate', 'endDate',])
        }

        const councilList = result.status === constant.TERM_COUNCIL_STATUS.VOTING ? result.councilMembers :
            _.filter(result.councilMembers, (e: any) => (
                [constant.COUNCIL_STATUS.ELECTED, constant.COUNCIL_STATUS.IMPEACHED, constant.COUNCIL_STATUS.TERMINATED].includes(e.status))
            )
        const council = _.map(councilList, (o: any) => ({
            ...filterFields(o._doc),
            ...this.getUserInformation(o._doc, o.user)
        }))

        const secretariat = _.map(secretariatResult, (o: any) => ({
            ...filterFields(o._doc),
            ...this.getUserInformation(o._doc, o.user),
            startDate: moment(o.startDate).unix(),
            endDate: o.endDate && moment(o.endDate).unix(),
        }))

        return {
            council,
            secretariat
        }
    }

    public async councilInformation(param: any): Promise<any> {
        const {id, did} = param

        // query council
        const fields = [
            'height',
            'status',
            'councilMembers.user.did',
            'councilMembers.cid',
            'councilMembers.did',
            'councilMembers.address',
            'councilMembers.introduction',
            'councilMembers.impeachmentVotes',
            'councilMembers.depositAmount',
            'councilMembers.location',
            'councilMembers.status',
        ]
        const query = {
            councilMembers: {$elemMatch: {did}}
        }
        if (id) {
            query['index'] = id
        } else {
            query['status'] = constant.TERM_COUNCIL_STATUS.CURRENT
        }
        const councilList = await this.model.getDBInstance()
            .findOne(query, fields)
            .populate('councilMembers.user', constant.DB_SELECTED_FIELDS.USER.NAME_EMAIL_DID)

        // query secretariat
        const secretariatFields = [
            'user.did',
            'did',
            'address',
            'location',
            'birthday',
            'email',
            'introduction',
            'wechat',
            'weibo',
            'facebook',
            'microsoft',
            'startDate',
            'endDate',
            'status',
        ]
        const secretariat = await this.secretariatModel.getDBInstance()
            .findOne({did}, secretariatFields)
            .populate('user', constant.DB_SELECTED_FIELDS.USER.NAME_EMAIL_DID)

        if (!councilList && !secretariat) {
            return {
                type: 'Other'
            }
        }

        if (councilList) {
            const council = councilList && _.filter(councilList.councilMembers, (o: any) => o.did === did)[0]

            let term = []
            let impeachmentObj = {}
            if (councilList.status !== constant.TERM_COUNCIL_STATUS.VOTING) {
                // update impeachment
                const impeachmentThroughVotes = (await ela.circulatingSupply(councilList.height)) * 0.2
                impeachmentObj['impeachmentVotes'] = council.impeachmentVotes
                impeachmentObj['impeachmentThroughVotes'] = _.toNumber(impeachmentThroughVotes.toFixed(8))
                impeachmentObj['impeachmentRatio'] = _.toNumber((council.impeachmentVotes / impeachmentThroughVotes).toFixed(4))

                // update term
                if (council && council.user) {
                    const proposalFields = [
                        'createdBy',
                        'createdAt',
                        'vid',
                        'title',
                        'status',
                        'voteResult'
                    ]
                    const proposalList = await this.proposalMode.getDBInstance()
                        .find({$or: [{proposer: council.user._id}, {'voteResult.votedBy': council.user._id}]}, proposalFields).sort({createdAt: -1})
                        .populate('createdBy', constant.DB_SELECTED_FIELDS.USER.NAME_EMAIL_DID)

                    term = _.map(proposalList, (o: any) => {
                        const didName = _.get(o, 'createdBy.did.didName')
                        const chainStatus = [constant.CVOTE_CHAIN_STATUS.CHAINED, constant.CVOTE_CHAIN_STATUS.CHAINING]
                        // Todo: add chain status limit
                        // const voteResult = _.filter(o.voteResult, (o: any) => o.votedBy === council.user._id && (chainStatus.includes(o.status) || o.value === constant.CVOTE_RESULT.UNDECIDED))
                        const voteResult = _.filter(o.voteResult, (o: any) => o.votedBy.equals(council.user._id))
                        const currentVoteResult = _.get(voteResult[0], 'value')
                        return {
                            id: o.vid,
                            title: o.title,
                            didName,
                            status: CVOTE_STATUS_TO_WALLET_STATUS[o.status],
                            voteResult: currentVoteResult,
                            createdAt: moment(o.createdAt).unix()
                        }
                    })
                }
            }

            return {
                ..._.omit(council._doc, ['_id', 'user', 'impeachmentVotes']),
                ...this.getUserInformation(council._doc, council.user),
                ...impeachmentObj,
                term,
                type: 'CouncilMember'
            }
        }

        if (secretariat) {
            return {
                ..._.omit(secretariat._doc, ['_id', 'user', 'startDate', 'endDate']),
                ...this.getUserInformation(secretariat._doc, secretariat.user),
                startDate: moment(secretariat.startDate).unix(),
                endDate: secretariat.endDate && moment(secretariat.endDate).unix(),
                type: 'SecretaryGeneral'
            }
        }

    }

    public async eachSecretariatJob() {
        const secretariatPublicKey = '0349cb77a69aa35be0bcb044ffd41a616b8367136d3b339d515b1023cc0f302f87'
        const secretariatDID = 'igCSy8ht7yDwV5qqcRzf5SGioMX8H9RXcj'

        const currentSecretariat = await this.secretariatModel.getDBInstance().findOne({status: constant.SECRETARIAT_STATUS.CURRENT})
        const information: any = await getInformationByDid(DID_PREFIX + secretariatDID)
        const didName = await getDidName(DID_PREFIX + secretariatDID)
        const user = await this.userMode.getDBInstance().findOne({'did.id': DID_PREFIX + secretariatDID}, ['_id', 'did'])

        if (!currentSecretariat) {
            const doc: any = this.filterNullField({
                ...information,
                user: user && user._id,
                did: secretariatDID,
                didName,
                startDate: new Date(),
                status: constant.SECRETARIAT_STATUS.CURRENT
            })

            // add secretariat
            await this.secretariatModel.getDBInstance().create(doc)
        } else {

            // update secretariat
            await this.secretariatModel.getDBInstance().update({did: secretariatDID}, {
                ...information,
                user: user && user._id,
            })
        }

        if (user && user.did) {
            const did = this.filterNullField({
                'did.avatar': _.get(information, 'avatar'),
                'did.didName': didName,
                'did.compressedPublicKey': secretariatPublicKey,
            })
            // add public key into user's did
            await this.userMode.getDBInstance().update({_id: user._id}, {
                $set: did
            })
        }
    }

    public async cronJob() {
        if (tm) {
            return false
        }
        tm = setInterval(async () => {
            console.log('---------------- start council or secretariat cronJob -------------')
            await this.eachSecretariatJob()
            await this.eachCouncilJob()
        }, 1000 * 60 * 5)
    }

    /**
     * get user information
     * didName avatar
     * @param obj
     * @param user
     */
    private getUserInformation(obj: any, user: any) {
        const {didName, avatar}: any = obj || {}
        const {didName: userDidName, avatar: userAvatar}: any = user && user.did && _.pick(user.did, ['didName', 'avatar']) || {}
        return this.filterNullField({
            didName: userDidName || didName,
            avatar: userAvatar || avatar,
        })
    }

    private filterNullField(obj: object) {
        return _.pickBy(obj, _.identity)
    }

    public async eachCouncilJob() {
        const listcrs = await ela.currentCouncil()
        const listcds = await ela.currentCandidates()
        const height = await ela.height()

        const crrelatedStageStatus = await ela.getCrrelatedStage()

        const currentCrs = await this.model.getDBInstance().findOne({status: constant.TERM_COUNCIL_STATUS.CURRENT})
        const votingCds = await this.model.getDBInstance().findOne({status: constant.TERM_COUNCIL_STATUS.VOTING})
        const historyCrs = await this.model.getDBInstance().findOne({status: constant.TERM_COUNCIL_STATUS.HISTORY})

        let index: any
        if (currentCrs) {
            index = currentCrs.index
        } else if (!currentCrs && historyCrs) {
            index = historyCrs.index
        } else {
            index = 1
        }

        const fields = [
            'code',
            'cid',
            'did',
            'location',
            'penalty',
            'votes',
            'index',
        ]

        const dataToCouncil = (data: any) => ({
            ..._.pick(data, fields),
            address: data.url,
            impeachmentVotes: data.impeachmentvotes,
            depositAmount: data.depositamout,
            depositAddress: data.depositaddress,
            status: data.state,
        });

        const updateUserInformation = async (councilMembers: any) => {
            const didList = _.map(councilMembers, (o: any) => DID_PREFIX + o.did)
            const userList = await this.userMode.getDBInstance().find({'did.id': {$in: didList}}, ['_id', 'did.id'])
            const userByDID = _.keyBy(userList, (o: any) => o.did.id.replace(DID_PREFIX, ''))
            // add avatar nickname into user's did
            await Promise.all(_.map(userList, async (o: any) => {
                if (o && o.did && !o.did.id) {
                    return
                }
                const information: any = await getInformationByDid(o.did.id)
                const didName = await getDidName(o.did.id)
                const did = this.filterNullField({
                    'did.avatar': _.get(information, 'avatar'),
                    'did.didName': didName,
                })
                if (_.isEmpty(did)) {
                    return
                }
                await this.userMode.getDBInstance().update({_id: o._id}, {
                    $set: did
                })
            }))

            return _.map(councilMembers, (o: any) => ({
                ...o,
                user: userByDID[o.did]
            }))
        }

        const updateUserRole = async (councilMembers: any, role: any) => {
            const didList = _.map(councilMembers, (o: any) => DID_PREFIX + o.did)

            switch (role) {
                case constant.USER_ROLE.COUNCIL:
                    await this.userMode.update(
                        {'did.id': {$in: didList}},
                        {
                            $set: {
                                role: constant.USER_ROLE.COUNCIL
                            }
                        },
                        {
                            multi: true
                        }
                    )
                    break;
                case constant.USER_ROLE.MEMBER:
                    await this.userMode.update(
                        {'did.id': {$in: didList}},
                        {
                            $set: {
                                role: constant.USER_ROLE.MEMBER
                            }
                        },
                        {
                            multi: true
                        }
                    )
                    break;
            }
        }

        const updateInformation = async (list: any, data: any) => {
            const newCouncilMembers = _.map(list, (o: any) => dataToCouncil(o))
            const newCouncilsByDID = _.keyBy(newCouncilMembers, 'did')
            const oldCouncilsByDID = _.keyBy(data.councilMembers, 'did')

            // update IMPEACHED status
            if (data.status === constant.TERM_COUNCIL_STATUS.CURRENT) {
                const result = _.filter(oldCouncilsByDID, (v: any, k: any) =>
                    (newCouncilsByDID[k]
                        && v.status !== constant.COUNCIL_STATUS.IMPEACHED
                        && newCouncilsByDID[k].status === constant.COUNCIL_STATUS.IMPEACHED))
                await updateUserRole(result, constant.USER_ROLE.MEMBER)
            }

            const councils = _.map(oldCouncilsByDID, (v: any, k: any) => (_.merge(v._doc, newCouncilsByDID[k])))

            const councilMembers = await updateUserInformation(councils)
            await this.model.getDBInstance().update({_id: data._id}, {councilMembers})
        }

        if (listcrs.crmembersinfo && listcds.crcandidatesinfo) {
            // update currentCrs and VotingCds
            if (currentCrs && votingCds) {
                await updateInformation(listcrs.crmembersinfo, currentCrs)
                await updateInformation(listcds.crcandidatesinfo, votingCds)
            }
            // update currentCrs , add listcds -> database status VOTING
            else if (currentCrs && !votingCds) {
                // update
                await updateInformation(listcrs.crmembersinfo, currentCrs)

                // add
                const startTime = await ela.getBlockByHeight(crrelatedStageStatus.votingstartheight)
                const doc: any = {
                    index: index + 1,
                    height: height || 0,
                    startDate: new Date(startTime * 1000),
                    endDate: null,
                    status: constant.TERM_COUNCIL_STATUS.VOTING,
                    councilMembers: _.map(listcds.crcandidatesinfo, (o) => dataToCouncil(o))
                }
                await this.model.getDBInstance().create(doc)
            }
        }

        if (listcrs.crmembersinfo && !listcds.crcandidatesinfo) {
            if (currentCrs) {
                // votingCds status -> CURRENT, currentCrs status -> HISTORY
                if (votingCds) {
                    const time = await ela.getBlockByHeight(crrelatedStageStatus.ondutystartheight)
                    await this.model.getDBInstance().update(
                        {
                            _id: currentCrs._id
                        },
                        {
                            $set: {
                                endDate: new Date(time * 1000),
                                status: constant.TERM_COUNCIL_STATUS.HISTORY
                            }
                        }
                    )
                    await this.model.getDBInstance().update(
                        {
                            _id: votingCds._id
                        },
                        {
                            $set: {
                                startDate: new Date(time * 1000),
                                status: constant.TERM_COUNCIL_STATUS.CURRENT
                            }
                        }
                    )

                    // update member role, MEMBER => COUNCIL
                    await updateUserRole(listcrs.crmembersinfo, constant.USER_ROLE.COUNCIL)
                    // update member role, COUNCIL => MEMBER
                    await updateUserRole(currentCrs.councilMembers, constant.USER_ROLE.MEMBER)

                }
                // update CurrentCrs data
                if (!votingCds) {
                    await updateInformation(listcrs.crmembersinfo, currentCrs)
                }
            }
            // votingCds status -> CURRENT
            else if (!currentCrs && votingCds) {
                const startTime = await ela.getBlockByHeight(crrelatedStageStatus.ondutystartheight)
                await this.model.getDBInstance().update(
                    {
                        _id: votingCds._id
                    },
                    {
                        $set: {
                            startDate: new Date(startTime * 1000),
                            status: constant.TERM_COUNCIL_STATUS.CURRENT
                        }
                    }
                )
                // update member role, MEMBER => COUNCIL
                await updateUserRole(listcrs.crmembersinfo, constant.USER_ROLE.COUNCIL)

            }
            // if appear directly first current
            if (!currentCrs && !votingCds && !historyCrs) {
                // add
                const startTime = await ela.getBlockByHeight(crrelatedStageStatus.ondutystartheight)

                const doc: any = {
                    index: 1,
                    height: height || 0,
                    startDate: new Date(startTime * 1000),
                    endDate: null,
                    status: constant.TERM_COUNCIL_STATUS.CURRENT,
                    councilMembers: _.map(listcrs.crmembersinfo, (o) => dataToCouncil(o))
                }
                await this.model.getDBInstance().create(doc);
                // update user role, MEMBER => COUNCIL
                await updateUserRole(listcrs.crmembersinfo, constant.USER_ROLE.COUNCIL)

            }
        }

        if (!listcrs.crmembersinfo && listcds.crcandidatesinfo) {
            if (currentCrs) {
                // currentCrs status -> HISTORY
                // if temporary change, endTime is votingstartheight time
                let endTime = await ela.getBlockByHeight(crrelatedStageStatus.votingstartheight)
                if (!endTime) {
                    endTime = new Date().getTime() / 1000
                }
                await this.model.getDBInstance().update(
                    {
                        _id: currentCrs._id
                    },
                    {
                        $set: {
                            status: constant.TERM_COUNCIL_STATUS.HISTORY,
                            endDate: new Date(endTime * 1000)
                        }
                    }
                )
                // update member role, COUNCIL => MEMBER
                await updateUserRole(currentCrs.councilMembers, constant.USER_ROLE.MEMBER)

                // update votingCds data
                if (votingCds) {
                    await updateInformation(listcds.crcandidatesinfo, votingCds)
                }
                // add listcds -> database, status VOTING
                if (!votingCds) {
                    const startTime = await ela.getBlockByHeight(crrelatedStageStatus.ondutystartheight)
                    const doc: any = {
                        index: index + 1,
                        startDate: new Date(startTime * 1000),
                        endDate: null,
                        status: constant.TERM_COUNCIL_STATUS.VOTING,
                        height: height || 0,
                        councilMembers: _.map(listcds.crcandidatesinfo, (o) => dataToCouncil(o))
                    }
                    await this.model.getDBInstance().create(doc)
                }
            }
            if (!currentCrs) {
                // update votingCds data
                if (votingCds) {
                    await updateInformation(listcds.crcandidatesinfo, votingCds)
                }
                // add votingCds -> database , status VOTING
                if (!votingCds) {
                    if (historyCrs) {
                        const startTime = await ela.getBlockByHeight(crrelatedStageStatus.ondutystartheight)
                        const doc: any = {
                            index: index + 1,
                            startDate: new Date(startTime * 1000),
                            endDate: null,
                            status: constant.TERM_COUNCIL_STATUS.VOTING,
                            height: height || 0,
                            councilMembers: _.map(listcds.crcandidatesinfo, (o) => dataToCouncil(o))
                        }
                        await this.model.getDBInstance().create(doc);
                    } else {
                        const startTime = await ela.getBlockByHeight(crrelatedStageStatus.ondutystartheight)
                        const doc: any = {
                            index: index,
                            startDate: new Date(startTime * 1000),
                            endDate: null,
                            status: constant.TERM_COUNCIL_STATUS.VOTING,
                            height: height || 0,
                            councilMembers: _.map(listcds.crcandidatesinfo, (o) => dataToCouncil(o))
                        }
                        await this.model.getDBInstance().create(doc);
                    }

                }
            }
        }

        if (!listcrs.crmembersinfo && !listcds.crcandidatesinfo) {
            if (currentCrs) {
                await this.model.getDBInstance().update(
                    {
                        _id: currentCrs._id
                    },
                    {
                        $set: {
                            status: constant.TERM_COUNCIL_STATUS.HISTORY,
                            endDate: new Date()
                        }
                    }
                )
                // update member role, COUNCILE => MEMBER
                await updateUserRole(currentCrs.councilMembers, constant.USER_ROLE.MEMBER)
            }
        }
    }
}
