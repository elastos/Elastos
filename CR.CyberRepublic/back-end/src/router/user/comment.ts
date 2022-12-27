import Base from '../Base'
import CommentService from '../../service/CommentService'

export default class extends Base{
    protected needLogin = true

    public async action(){
        const commentService = this.buildService(CommentService)
        const rs = await commentService.create('User', this.getParam())
        return this.result(1, rs)
    }
}
