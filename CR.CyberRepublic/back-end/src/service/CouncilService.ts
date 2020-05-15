import Base from './Base'
import {constant} from '../constant'
import {ela, logger, getInformationByDID} from '../utility'
import * as moment from 'moment'

const _ = require('lodash')

let tm = undefined

export default class extends Base {
    private model: any
    private userMode: any
    private proposalMode: any
    protected init() {
        this.model = this.getDBModel('Council')
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

    public async councilList(id: String): Promise<any> {
        const fields = [
            'councilMembers.did',
            'councilMembers.didName',
            'councilMembers.avatar',
            'councilMembers.location',
            'councilMembers.status',
        ]

        const result = await this.model.getDBInstance().findOne({_id: id}, fields)

        if (!result) {
            return {
                code: 400,
                message: 'Invalid request parameters - status',
                // tslint:disable-next-line:no-null-keyword
                data: null
            }
        }

        return result.councilMembers
    }

    public async councilInformation(param: any): Promise<any> {
        const {id, did} = param

        const fields = [
            'height',
            'status',
            'councilMembers.did',
            'councilMembers.didName',
            'councilMembers.avatar',
            'councilMembers.address',
            'councilMembers.introduction',
            'councilMembers.impeachmentVotes',
            'councilMembers.location',
            'councilMembers.status',
        ]

        const result = await this.model.getDBInstance().findOne({_id: id}, fields)
        const council = result && _.filter(result.councilMembers, (o: any) => o.did === did)
        const user = await this.userMode.getDBInstance().findOne({'did.id': did}, ['_id'])
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
                doc.councilMembers = _.map(candidates.crcandidatesinfo, (o) => dataToCouncil(o))
            } else if (currentCouncil.crmembersinfo) {
                doc.endDate = moment().add(1, 'years').toDate()
                doc.status = constant.TERM_COUNCIL_STATUS.CURRENT
                doc.councilMembers = _.map(currentCouncil.crmembersinfo, (o) => dataToCouncil(o))
            }

            await this.model.getDBInstance().create(doc);

        } else {
            // TODO: exist bug

            console.log('exist')

            // 更新数据

            // 是否换届

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

    public cronJob() {
        if (tm) {
            return false
        }
        tm = setInterval(async () => {
            console.log('---------------- start council or secretariat cronJob -------------')
            await this.eachJob()
        }, 1000 * 60)
    }
}
