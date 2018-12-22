import { createContainer } from '@/util'
import Component from './Component';
import SSOService from '@/service/SSOService';

export default createContainer(Component, () => {
    return {}
}, () => {
    const ssoService = new SSOService();

    return {
        async getLoginStr(params) {
            const data = await ssoService.getLoginStr(params);
            return data;
        }
    };
})
