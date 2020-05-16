import Base from './Base'
import {permissions, ela} from '../utility';

const _ = require('lodash')


export default class extends Base {
    public async updateProposal(param: any): Promise<any> {
        const userRole = _.get(this.currentUser, 'role')

        if (!permissions.isAdmin(userRole)){
            return {
                code: 403,
            }
        }

        const {id, status, proposedAt, voteResult, voteHistory, rejectAmount, rejectHeight } = param

        const db_cvote = this.getDBModel('CVote')

        const rs = await db_cvote.getDBInstance().update({_id: id}, {
            status,
            proposedAt,
            voteResult,
            voteHistory,
            rejectAmount: rejectAmount || 0,
            rejectHeight: rejectHeight || 0,
        })

        return rs
    }

    public async proposal(id: string): Promise<any> {
        const db_cvote = this.getDBModel('CVote')

        const fields = [
            'status',
            'proposedAt',
            'voteResult',
            'voteHistory',
            'rejectAmount',
            'rejectHeight',
        ]

        const rs = await db_cvote.getDBInstance().findOne({_id: id}, fields)

        return rs
    }

    public async listCouncil(): Promise<any> {
        const rs = await ela.currentCouncil()

        return rs
    }

    public async listCandidates(): Promise<any> {
        const rs = await ela.currentCandidates()

        return rs
    }
}
