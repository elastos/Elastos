import Base from './Base'
import { Permission } from './schema/PermissionSchema'

export default class extends Base {
    protected getSchema(){
        return Permission
    }
    protected getName(){
        return 'permission'
    }
}
