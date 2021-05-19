import Base from '../Base'
import SubmissionService from '../../service/SubmissionService'

export default class extends Base{
    // protected needLogin = true;
    public async action(){
        const submissionService = this.buildService(SubmissionService)
        const rs = await submissionService.create(this.getParam())

        return this.result(1, rs)
    }
}
