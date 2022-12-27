import Base from '../Base'
import SubmissionService from '../../service/SubmissionService'

/**
 * Both the '/' and '/:submissionId' routes map to this class
 */
export default class CampaignSubmission extends Base {
    async action(){
        const submissionService = this.buildService(SubmissionService)
        const rs = await submissionService.show(this.getParam())
        return this.result(1, rs)
    }
}
