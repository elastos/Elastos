import Base from './Base'
import {constant} from '../constant'
import {ela, logger} from '../utility'
import * as moment from 'moment'

const _ = require('lodash')

let tm = undefined

// TODO：history of council
export default class extends Base {
    private model: any

    protected init() {
        this.model = this.getDBModel('Council')
    }

    public async term(param: any): Promise<any> {
        return {
            test: 'test'
        }
    }

    public async termList(param: any): Promise<any> {
        return {
            test: 'test'
        }
    }

    public async termInformation(param: any): Promise<any> {
        return {
            test: 'test'
        }
    }

    public async eachJob() {
        const currentCouncil = await ela.currentCouncil()
        const candidates = await ela.currentCandidates()

        const lastCouncil = await this.model.getDBInstance().findOne().sort({ index: -1 })

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

            await this.model.save(doc);

            return
        }

        const { index, endDate } = lastCouncil

        if (moment(endDate).startOf('day').isBefore(moment().startOf('day'))) {
            // add council
            const doc: any = {
                index: index + 1,
                startDate: new Date(),
                endDate: moment().add(1, 'years').toDate(),
                status: constant.TERM_COUNCIL_STATUS.CURRENT,
                councilMembers: _.map(currentCouncil.crmembersinfo, (o) => dataToCouncil(o))
            }

            await this.model.getDBInstance().save(doc)
        } else {
            // change council
            const normalChange = moment(endDate)
                .startOf('day')
                .subtract(1, 'months')
                .isAfter(moment().startOf('day'))

            if (normalChange || currentCouncil.crmembersinfo) {
                const doc: any = {
                    index: index + 1,
                    startDate: new Date(),
                    endDate: moment().add(1, 'years').add(1, 'months').toDate(),
                    status: constant.TERM_COUNCIL_STATUS.VOTING,
                    councilMembers: _.map(candidates.crcandidatesinfo, (o) => dataToCouncil(o))
                }

                await this.model.getDBInstance().save(doc);
            }

        }

        await this.model.getDBInstance().update({_id: lastCouncil._id}, {
            $set: {
                ...lastCouncil,
                status: constant.TERM_COUNCIL_STATUS.HISTORY,
            }
        })

    }

    public cronJob() {
        if (tm) {
            return false
        }
        tm = setInterval(() => {
            console.log('---------------- start council or secretariat cronJob -------------')
            this.eachJob()
        }, 1000 * 60 * 30)
    }
}
