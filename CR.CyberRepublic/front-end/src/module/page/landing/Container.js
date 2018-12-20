import {createContainer} from '@/util'
import Component from './Component'
import LanguageService from '@/service/LanguageService';
import _ from 'lodash'

export default createContainer(Component, (state) => {

    return {
        is_login: state.user.is_login,
        language: state.language.language
    }
}, () => {
    const languageService = new LanguageService();
    return {
        changeLanguage(lang) {
            languageService.changeLanguage(lang);
        }
    }
})
