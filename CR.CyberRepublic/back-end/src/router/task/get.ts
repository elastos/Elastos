import Base from '../Base'
import TaskService from '../../service/TaskService'

/**
 * Both the '/' and '/:taskId' routes map to this class
 */
export default class GetTask extends Base {

    async action(){

        const taskService = this.buildService(TaskService)
        const rs = await taskService.show(this.getParam())
        return this.result(1, rs)
    }
}
