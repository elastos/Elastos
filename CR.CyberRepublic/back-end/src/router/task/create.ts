import Base from '../Base'
import TaskService from '../../service/TaskService'

export default class extends Base{
    protected needLogin = true
    public async action(){
        const taskService = this.buildService(TaskService)
        const rs = await taskService.create(this.getParam())

        return this.result(1, rs)
    }
}