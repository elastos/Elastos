import Base from './Base'
import { PermissionRole } from './schema/PermissionSchema'

export default class extends Base {
    protected getSchema(){
        return PermissionRole
    }
    protected getName(){
        return 'permission_role'
    }
}
