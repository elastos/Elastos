import Base from './Base'
import {constant} from '../constant'
import {ela, logger, getInformationByDID} from '../utility'
import * as moment from 'moment'

const _ = require('lodash')

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
            endDate: moment(o.endDate).unix(),
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

        const result = await this.model.getDBInstance().findOne({index: id}, fields)
            .populate('user', constant.DB_SELECTED_FIELDS.USER.NAME_EMAIL_DID)

        const secretariatResult = await this.secretariatModel
            .getDBInstance().find({}, secretariatFields).sort({'startDate': -1})
            .populate('user', constant.DB_SELECTED_FIELDS.USER.NAME_EMAIL_DID)

        if (!result) {
            return {
                code: 400,
                message: 'Invalid request parameters - status',
                // tslint:disable-next-line:no-null-keyword
                data: null
            }
        }

        const information = (user: any) => {
            if (user && user.did) {
                return _.pick(user.did, ['didName', 'avatar'])
            }
            return {}
        }

        const council = _.map(result.councilMembers, (o: any) => ({
            ..._.omit(o, ['user']),
            ...information(o.user)
        }))

        const secretariat = _.map(secretariatResult, (o: any) => ({
            ..._.omit(o._doc, ['_id', 'user']),
            ...information(o.user)
        }))

        return {
            council,
            secretariat
        }
    }

    public async councilInformation(param: any): Promise<any> {
        const {id, did} = param
        const query = {}

        if (id) {
            query['index'] = id
        } else {
            query['status'] = constant.TERM_COUNCIL_STATUS.CURRENT
        }

        const fields = [
            'height',
            'status',
            'councilMembers.did',
            'councilMembers.didName',
            'councilMembers.avatar',
            'councilMembers.address',
            'councilMembers.introduction',
            'councilMembers.impeachmentVotes',
            'councilMembers.depositAmount',
            'councilMembers.location',
            'councilMembers.status',
        ]

        const result = await this.model.getDBInstance()
            .findOne(query, fields)

        const council = result && _.filter(result.councilMembers, (o: any) => o.did === did)

        if (!result || !council) {
            return {
                code: 400,
                message: 'Invalid request parameters',
                // tslint:disable-next-line:no-null-keyword
                data: null
            }
        }

        // TODO： return term information
        let term
        if (result.status !== constant.TERM_COUNCIL_STATUS.VOTING) {

        }

        return {
            ...council[0]._doc,
            impeachmentHeight: result.height * 0.2
        }
    }

    public async eachSecretariatJob() {
        const secretariatPublicKey = '0349cb77a69aa35be0bcb044ffd41a616b8367136d3b339d515b1023cc0f302f87'
        const secretariatDID = 'igCSy8ht7yDwV5qqcRzf5SGioMX8H9RXcj'

        const currentSecretariat = await this.secretariatModel.getDBInstance().findOne({status: constant.SECRETARIAT_STATUS.CURRENT})
        const information: any = await getInformationByDID(secretariatDID)
        const user = await this.userMode.getDBInstance().findOne({'did.id': secretariatDID}, ['_id', 'did'])

        if (!currentSecretariat) {
            const doc: any = {
                ...information,
                user: user._id,
                did: secretariatDID,
                startDate: new Date(),
                status: constant.SECRETARIAT_STATUS.CURRENT
            }

            // add public key into user's did
            await this.userMode.getDBInstance().update({_id: user._id}, {
                $set: {
                    'did.compressedPublicKey': secretariatPublicKey
                }
            })

            // add secretariat
            await this.secretariatModel.getDBInstance().create(doc)
        } else {

            // update secretariat
            if (information) {
                await this.secretariatModel.getDBInstance().update({did: secretariatDID}, {
                    ...information
                })
            }

            // if public key not on the user's did
            if (user && user.did && !user.did.compressedPublicKey) {
                const rs = await this.userMode.getDBInstance().update({_id: user._id}, {
                    $set: {'did.compressedPublicKey': secretariatPublicKey}
                })
            }
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
            didName: data.nickname,
            address: data.url,
            impeachmentVotes: data.impeachmentvotes,
            depositAmount: data.depositamout,
            depositHash: data.deposithash,
            status: data.state,
        });

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

            const didList = _.map(doc.councilMembers, 'did')
            const userList = await this.userMode.getDBInstance().find({'did.id': {$in: didList}}, ['_id', 'did.id'])
            const userByDID = _.keyBy(userList, 'did.id')

            doc.councilMembers = _.map(doc.councilMembers, (o: any) => ({
                ...o,
                user: userByDID[o.did]
            }))

            // TODO: need to optimizing (multiple update)
            // add avatar nickname into user's did
            await Promise.all(_.map(userList, async (o: any) => {
                const information: any = await getInformationByDID(o.did.id)
                const result = _.pick(information, ['avatar', 'didName'])
                if (result) {
                    await this.userMode.getDBInstance().update({_id: o._id}, {
                        $set: {
                            'did.avatar': result.avatar,
                            'did.didName': result.didName
                        }
                    })
                }
            }))

            await this.model.getDBInstance().create(doc);

        } else {
            console.log('exist')

            // update data


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
        }, 1000 * 30)
    }
}
