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

        return _.map(result, (o: any) => ({
            id: o._id,
            ..._.omit(o._doc, ['_id']),
            startDate: moment(o.startDate).unix(),
            endDate: o.status === constant.TERM_COUNCIL_STATUS.HISTORY && o.endDate && moment(o.endDate).unix(),
        }))
    }

    public async councilList(id: number): Promise<any> {
        const fields = [
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

        const council = _.map(result.councilMembers, (o: any) => ({
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
                        const voteResult = _.filter(o.voteResult, (o: any) => o.votedBy === council.user._id && (chainStatus.includes(o.status) || o.value === constant.CVOTE_RESULT.UNDECIDED))
                        // const voteResult = _.filter(o.voteResult, (o: any) => o.votedBy.equals(council.user._id))
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

    public async eachJob() {
        const currentCouncil = await ela.currentCouncil()
        const candidates = await ela.currentCandidates()
        const height = await ela.height();

        const lastCouncil = await this.model.getDBInstance().findOne().sort({index: -1})

        const fields = [
            'code',
            'cid',
            'did',
            'location',
            'penalty',
            'index'
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

            // TODO: need to optimizing (multiple update)
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

        // not exist council
        if (!lastCouncil) {
            const doc: any = {
                index: 1,
                startDate: new Date(),
                height: height || 0,
            }

            // add voting or current council list
            if (candidates.crcandidatesinfo) {
                doc.endDate = moment().add(1, 'years').add(1, 'months').toDate()
                doc.status = constant.TERM_COUNCIL_STATUS.VOTING
                doc.councilMembers = _.map(candidates.crcandidatesinfo, async (o) => {
                    const obj = dataToCouncil(o)
                    const depositObj = await ela.depositCoin(o.did)
                    if (!depositObj) {
                        return obj
                    }
                    return {
                        ...o,
                        depositAmount: depositObj && depositObj.available || '0'
                    }
                })
            } else if (currentCouncil.crmembersinfo) {
                doc.endDate = moment().add(1, 'years').toDate()
                doc.status = constant.TERM_COUNCIL_STATUS.CURRENT
                doc.councilMembers = _.map(currentCouncil.crmembersinfo, (o) => dataToCouncil(o))
            }

            doc.councilMembers = await updateUserInformation(doc.councilMembers)

            await this.model.getDBInstance().create(doc);

        } else {
            let newCouncilMembers
            // update data
            if (lastCouncil.status === constant.TERM_COUNCIL_STATUS.VOTING) {
                newCouncilMembers = _.map(candidates.crcandidatesinfo, async (o: any) => {
                    const obj = dataToCouncil(o)
                    const depositObj = await ela.depositCoin(o.did)
                    if (!depositObj) {
                        return obj
                    }
                    return {
                        ...o,
                        depositAmount: depositObj && depositObj.available || '0'
                    }
                })
            }

            if (lastCouncil.status === constant.TERM_COUNCIL_STATUS.CURRENT) {
                newCouncilMembers = _.map(currentCouncil.crmembersinfo, (o: any) => dataToCouncil(o))
            }

            const newCouncilsByDID = _.keyBy(newCouncilMembers, 'did')
            const oldCouncilsByDID = _.keyBy(lastCouncil.councilMembers, 'did')

            const councils = _.map(oldCouncilsByDID, (v: any, k: any) => (_.merge(v._doc, newCouncilsByDID[k])))
            const councilMembers = await updateUserInformation(councils)

            await this.model.getDBInstance().update({_id: lastCouncil._id}, {councilMembers})

            // TODO: 换届
            // const {index, endDate} = lastCouncil
            //
            // if (moment(endDate).startOf('day').isBefore(moment().startOf('day'))) {
            //     // add council
            //     await this.model.getDBInstance().update({_id: lastCouncil._id}, {
            //         $set: {
            //             ...lastCouncil,
            //             status: constant.TERM_COUNCIL_STATUS.CURRENT,
            //         }
            //     })
            // } else {
            //     // change council
            //     const normalChange = moment(endDate)
            //         .startOf('day')
            //         .subtract(1, 'months')
            //         .isAfter(moment().startOf('day'))
            //
            //     if (normalChange || currentCouncil.crmembersinfo) {
            //         const doc: any = {
            //             index: index + 1,
            //             startDate: new Date(),
            //             endDate: moment().add(1, 'years').add(1, 'months').toDate(),
            //             status: constant.TERM_COUNCIL_STATUS.VOTING,
            //             height: height || 0,
            //             councilMembers: _.map(candidates.crcandidatesinfo, (o) => dataToCouncil(o))
            //         }
            //
            //         await this.model.getDBInstance().save(doc);
            //         await this.model.getDBInstance().update({_id: lastCouncil._id}, {
            //             $set: {
            //                 ...lastCouncil,
            //                 status: constant.TERM_COUNCIL_STATUS.HISTORY,
            //             }
            //         })
            //     }
            //
            // }
        }

    }

    public async cronJob() {
        if (tm) {
            return false
        }
        tm = setInterval(async () => {
            console.log('---------------- start council or secretariat cronJob -------------')
            await this.eachJob()
            await this.eachSecretariatJob()
        }, 1000 * 60 * 10)
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
}
