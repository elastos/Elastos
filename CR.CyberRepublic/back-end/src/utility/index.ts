import utilCrypto from './crypto';
import mail from './mail';
import validate from './validate';
import sso from './sso';

export {
    utilCrypto,
    sso,
    validate,
    mail
};

export const getEnv = () => process.env.NODE_ENV;
