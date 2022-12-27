import Base from '../Base'
import SubmissionService from '../../service/SubmissionService'

export default class ArchiveSubmission extends Base {
    async action(){
        const submissionService = this.buildService(SubmissionService)
        const rs = await submissionService.archive(this.getParam())
        return this.result(1, rs)
    }
}
