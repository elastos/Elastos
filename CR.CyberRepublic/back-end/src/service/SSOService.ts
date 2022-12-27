import Base from './Base'
import { sso } from '../utility'

export default class extends Base {
    public login(param: any): string {
        const { sso: payload, sig } = param // fetch from incoming request
        if (!sso.validate(payload, sig) || !this.currentUser) {
            throw 'login info invalid'
        }
        const nonce = sso.getNonce(payload)
        const {
            _id,
            email,
            username,
            profile: { lastName, firstName, bio }
        } = this.currentUser
        const userparams = {
            // Required, will throw exception otherwise
            nonce,
            external_id: _id.toString(),
            email,
            // Optional
            username,
            name: `${firstName} ${lastName}`,
            bio,
        }
        const loginString = sso.buildLoginString(userparams)
        return loginString
    }
}