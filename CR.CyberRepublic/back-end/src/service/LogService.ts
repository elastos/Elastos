import Base from './Base'
import {Document} from 'mongoose'
import {constant} from '../constant'

export default class extends Base {
    private model
    protected init(){
        this.model = this.getDBModel('Log')
    }
    public async applyToAddTeam(
        teamId: string,
        memberId: string,
        reason: string
    ): Promise<Document>{
        return await this.model.save({
            teamId,
            memberId,
            reason,
            type : constant.LOG_TYPE.APPLY_TEAM
        })
    }
}