import Base from '../Base'
import { ela } from '../../utility'

export default class extends Base {
    // protected needLogin = true;
    async action(){
        const rs = await ela.height()
        return this.result(1, rs)
    }
}