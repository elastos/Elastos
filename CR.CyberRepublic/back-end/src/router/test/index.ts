import Base from '../Base'
import TestService from '../../service/TestService'

class helloworld extends Base {
    protected needLogin = true
    async action(){

        const testService = this.buildService(TestService)
        const list = await testService.getTestList()

        return this.result(1, list,'hello world, ebp')
    }
}

export default Base.setRouter([
    {
        path: '/hello',
        router: helloworld,
        method: 'get'
    },
])