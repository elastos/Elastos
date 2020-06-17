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
            if (o.status !== constant.TERM_COUNCIL_STATUS.VOTING) {
                dateObj['startDate'] = o.startDate && moment(o.startDate).unix()
            }
            if (o.status === constant.TERM_COUNCIL_STATUS.HISTORY) {
                dateObj['endDate'] = o.endDate && moment(o.endDate).unix()
            }
            return ({
                id: o._id,
                ..._.omit(o._doc, ['_id', 'startDate', 'endDate']),
                ...dateObj,
            })
        })
    }

    public async councilList(id: number): Promise<any> {
        const fields = [
            'status',
            'councilMembers.didName',
            'councilMembers.avatar',
            'councilMembers.did',
            'councilMembers.user.did',
            'councilMembers.location',
            'councilMembers.status',
        ]

        const secretariatFields = [
            'did',
            'didName',
            'avatar',
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

        if (!id && !did) {
            return {
                tyep: 'Other'
            }
        }

        // query council
        const fields = [
            'height',
            'circulatingSupply',
            'status',
            'councilMembers.didName',
            'councilMembers.avatar',
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
            councilMembers: {
                $elemMatch: {
                    did,
                    status: {$in: [constant.COUNCIL_STATUS.ELECTED, constant.COUNCIL_STATUS.IMPEACHED]}
                }
            }
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
            const unelectedCouncil = await ela.depositCoin(did);
            if (unelectedCouncil) {
                return {
                    type: 'UnelectedCouncilMember',
                    depositAmount: _.get(unelectedCouncil, 'available')
                }
            }
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
                const circulatingSupply = councilList.circulatingSupply ? councilList.circulatingSupply : (await ela.circulatingSupply(councilList.height))
                const impeachmentThroughVotes = circulatingSupply * 0.2
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
                        'voteResult',
                        'voteHistory'
                    ]
                    const proposalList = await this.proposalMode.getDBInstance()
                        .find({$or: [{proposer: council.user._id}, {'voteResult.votedBy': council.user._id}, {'voteHistory.votedBy': council.user._id}]}, proposalFields).sort({createdAt: -1})
                        .populate('createdBy', constant.DB_SELECTED_FIELDS.USER.NAME_EMAIL_DID)

                    term = _.map(proposalList, (o: any) => {
                        const didName = _.get(o, 'createdBy.did.didName')
                        let voteResult = _.filter(o.voteResult, (o: any) => council.user._id.equals(o.votedBy) && o.status == constant.CVOTE_CHAIN_STATUS.CHAINED)
                        if (_.isEmpty(voteResult)) {
                            voteResult = _.filter(o.voteHistory, (o: any) => council.user._id.equals(o.votedBy) && o.status == constant.CVOTE_CHAIN_STATUS.CHAINED)
                        }
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
                    term = _.filter(term, (o: any) => {
                        return o.voteResult && o.voteResult !== constant.CVOTE_RESULT.UNDECIDED
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
        // const secretariatPublicKey = '0349cb77a69aa35be0bcb044ffd41a616b8367136d3b339d515b1023cc0f302f87'
        const secretaryGeneral = await ela.getSecretaryGeneral()
        const { secretarygeneral: secretariatPublicKey } = secretaryGeneral || { secretarygeneral: null }
        const secretariatDID = process.env.SECRETARIAT_DID

        const currentSecretariat = await this.secretariatModel.getDBInstance().findOne({status: constant.SECRETARIAT_STATUS.CURRENT})
        const { did: currentSecretariatDID } = currentSecretariat || { did : secretariatDID }
        const information: any = await getInformationByDid(DID_PREFIX + currentSecretariatDID)
        const didName = await getDidName(DID_PREFIX + currentSecretariatDID)
        const user = await this.userMode.getDBInstance().findOne({'did.id': DID_PREFIX + currentSecretariatDID}, ['_id', 'did'])

        if (!currentSecretariat) {
            const doc: any = this.filterNullField({
                ...information,
                user: user && user._id,
                did: currentSecretariatDID,
                didName,
                startDate: new Date(),
                status: constant.SECRETARIAT_STATUS.CURRENT
            })

            // add secretariat
            await this.secretariatModel.getDBInstance().create(doc)
        } else {

            // update secretariat
            await this.secretariatModel.getDBInstance().update({$or: [{did: currentSecretariatDID}, {did: DID_PREFIX + currentSecretariatDID}]}, {
                ...information,
                did: currentSecretariatDID,
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
            await this.eachCouncilJobPlus()
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

    public async temporaryChangeUpdateStatus() {
        const db_cvote = this.getDBModel('CVote')
        const proposaedList = await db_cvote.find({status: constant.CVOTE_STATUS.PROPOSED, old: {$ne: true}})
        const notificationList = await db_cvote.find({status: constant.CVOTE_STATUS.NOTIFICATION, old: {$ne: true}})
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

    public async eachCouncilJobPlus() {
        const listCrs = await ela.currentCouncil()
        const height = await ela.height()
        const circulatingSupply = await ela.currentCirculatingSupply()

        const crRelatedStageStatus = await ela.getCrrelatedStage()

        const {onduty: isOnduty, invoting: isInVoting} = crRelatedStageStatus

        const currentCrs = await this.model.getDBInstance().findOne({status: constant.TERM_COUNCIL_STATUS.CURRENT})
        const votingCds = await this.model.getDBInstance().findOne({status: constant.TERM_COUNCIL_STATUS.VOTING})
        const historyCrs = await this.model.getDBInstance().findOne({status: constant.TERM_COUNCIL_STATUS.HISTORY})

        let index: any
        if (currentCrs) {
            index = currentCrs.index
        } else if (!currentCrs && historyCrs) {
            index = historyCrs.index
        } else {
            index = 0
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
            const councilsMember = await Promise.all(_.map(councilMembers, async (o: any) => {
                const information: any = await getInformationByDid(o.did)
                const didName = await getDidName(DID_PREFIX + o.did)
                const did = this.filterNullField({
                    'did.avatar': _.get(information, 'avatar'),
                    'did.didName': didName,
                })
                if (!_.isEmpty(did) && userByDID[o.did]) {
                    await this.userMode.getDBInstance().update({_id: userByDID[o.did]._id}, {
                        $set: did
                    })
                }
                const data = {
                    ...o,
                    ..._.omit(information,['did']),
                }
                return {
                    ...data,
                    didName,
                    user: userByDID[o.did]
                }
            }))
            return councilsMember
        }

        const updateUserRoleToNewDid = async () => {
            const electedCouncils = _.filter(listCrs.crmembersinfo, (o: any) => o.state === constant.COUNCIL_STATUS.ELECTED)
            const impeachedCouncils = _.filter(listCrs.crmembersinfo, (o: any) => o.state !== constant.COUNCIL_STATUS.ELECTED)

            const electedDidList = _.map(electedCouncils, (o: any) => DID_PREFIX + o.did)
            const impeachedDidList = _.map(impeachedCouncils, (o: any) => DID_PREFIX + o.did)

            await this.userMode.getDBInstance().update(
                {'did.id': {$in: electedDidList}},
                {
                    $set: {role: constant.USER_ROLE.COUNCIL}
                },
                {
                    multi: true
                }
            )
            await this.userMode.getDBInstance().update(
                {'did.id': {$in: impeachedDidList}},
                {
                    $set: {role: constant.USER_ROLE.MEMBER}
                },
                {
                    multi: true
                }
            )
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
                        {multi: true}
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
                        {multi: true}
                    )
                    break;
            }
        }

        const updateInformation = async (list: any, data: any, status: any) => {
            const newCouncilMembers = _.map(list, (o: any) => dataToCouncil(o))
            const newCouncilsByDID = _.keyBy(newCouncilMembers, 'did')
            const oldCouncilsByDID = _.keyBy(data && data.councilMembers, 'did')

            let councils
            let doc = {
                index,
                height,
                circulatingSupply,
                startDate: null,
                endDate: null,
                status: constant.TERM_COUNCIL_STATUS.VOTING,
                councilMembers: [],
                ..._.omit(data && data._doc, ['_id'])
            }

            const startTime = await ela.getTimestampByHeight(crRelatedStageStatus.ondutystartheight)
            const endTime = await ela.getTimestampByHeight(crRelatedStageStatus.votingstartheight)
            if (status) {
                const councilMembers = await updateUserInformation(newCouncilMembers)
                doc['status'] = status
                doc['startDate'] = new Date(startTime * 1000)
                doc['councilMembers'] = councilMembers
            }

            if (_.isEmpty(data)) {
                doc['index'] = index + 1
                await this.model.getDBInstance().create(doc)
                return
            }

            if (status && data.status === constant.TERM_COUNCIL_STATUS.VOTING) {
                doc['status'] = status
                doc['startDate'] = new Date(startTime * 1000)
            }
            if (status && data.status === constant.TERM_COUNCIL_STATUS.CURRENT) {
                doc['status'] = status
                doc['startDate'] = data.startDate
                doc['endDate'] = crRelatedStageStatus.ondutystartheight !== 0 ? new Date(startTime * 1000) : new Date(endTime * 1000)
            }

            if (!_.isEmpty(oldCouncilsByDID)) {
                // update IMPEACHED status
                if (data.status === constant.TERM_COUNCIL_STATUS.CURRENT) {
                    const result = _.filter(oldCouncilsByDID, (v: any, k: any) =>
                        (newCouncilsByDID[k]
                            // && v.status !== constant.COUNCIL_STATUS.IMPEACHED
                            && newCouncilsByDID[k].status === constant.COUNCIL_STATUS.IMPEACHED))
                    await updateUserRole(result, constant.USER_ROLE.MEMBER)
                }
                councils = _.map(oldCouncilsByDID, (v: any, k: any) => (_.merge(v._doc, newCouncilsByDID[k])))
            } else {
                councils = newCouncilMembers
            }
            const councilMembers = await updateUserInformation(councils)
            doc['councilMembers'] = councilMembers
            doc['height'] = height
            doc['circulatingSupply'] = circulatingSupply
            await this.model.getDBInstance().update({_id: data._id}, {...doc})
        }

        if (isOnduty) {
            if (isInVoting) {
                await updateInformation(listCrs.crmembersinfo, currentCrs, null)
                await updateInformation(null, votingCds, null)
            } else {
                if (currentCrs && votingCds) {
                    await updateInformation(listCrs.crmembersinfo, votingCds, constant.TERM_COUNCIL_STATUS.CURRENT)
                    await updateUserRole(listCrs.crmembersinfo, constant.USER_ROLE.COUNCIL)
                    await updateInformation(null, currentCrs, constant.TERM_COUNCIL_STATUS.HISTORY)
                    await updateUserRole(currentCrs.councilMembers, constant.USER_ROLE.MEMBER)
                }
                if (!currentCrs && votingCds) {
                    await updateInformation(listCrs.crmembersinfo, votingCds, constant.TERM_COUNCIL_STATUS.CURRENT)
                    await updateUserRole(listCrs.crmembersinfo, constant.USER_ROLE.COUNCIL)
                }
                if (currentCrs && !votingCds) {
                    await updateInformation(listCrs.crmembersinfo, currentCrs, null)
                    await updateUserRoleToNewDid()
                }
                if (!currentCrs && !votingCds && !historyCrs) {
                    await updateInformation(listCrs.crmembersinfo, null, constant.TERM_COUNCIL_STATUS.CURRENT)
                    await updateUserRole(listCrs.crmembersinfo, constant.USER_ROLE.COUNCIL)
                }
            }
        }

        if (!isOnduty) {
            if (currentCrs) {
                await updateInformation(null, currentCrs, constant.TERM_COUNCIL_STATUS.HISTORY)
                await updateUserRole(currentCrs.councilMembers, constant.USER_ROLE.MEMBER)
                await this.temporaryChangeUpdateStatus()
            }
            if (!votingCds) {
                await updateInformation(null, null, null)
            }
        }
    }

    public async getCouncilSecretariat() {
        const councils = await this.model.getDBInstance().findOne({status: constant.TERM_COUNCIL_STATUS.CURRENT})
        const secretariat = await this.secretariatModel.getDBInstance().findOne()
        return {
            councils:councils._doc,
            secretariat:secretariat._doc
        }
    }
}
