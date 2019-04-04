import utilCrypto from './crypto'
import mail from './mail'
import validate from './validate'
import sso from './sso'
import user from './user'
import * as permissions from './permissions'

export {
    utilCrypto,
    sso,
    user,
    validate,
    permissions,
    mail
}

export const getEnv = () => process.env.NODE_ENV
