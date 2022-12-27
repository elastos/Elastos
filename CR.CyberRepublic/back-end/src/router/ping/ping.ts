import Base from '../Base'

export default class extends Base {

    async action(){
        return this.result(1, 'pong')
    }
}
