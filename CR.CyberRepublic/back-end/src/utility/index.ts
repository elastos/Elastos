import utilCrypto from './crypto';
import mail from './mail';
import validate from './validate';

export {
    utilCrypto,
    validate,
    mail
};

export const getEnv = () => process.env.NODE_ENV;
