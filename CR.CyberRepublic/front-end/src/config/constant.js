import * as _ from 'lodash'

export const RECAPTCHA_KEY = '6LeH0DIUAAAAAMfp3kJQdiW0y-4VsIM-y53GRBBD';
export const MIN_LENGTH_PASSWORD = 8;

// Responsive layout
export const MAX_WIDTH_MOBILE = 768;
export const MIN_WIDTH_PC = 769;

export const EMPOWER_MAX_BUSINESS = 4;
export const EMPOWER_MAX_MARKETING = 4;
export const EMPOWER_MAX_LEGAL = 2;
export const EMPOWER_MAX_DESIGNER = 2;
export const EMPOWER_MAX_VIDEOGRAPHER = 2;
export const EMPOWER_MAX_WRITER_CONTENT = 3;
export const EMPOWER_MAX_WRITER_TECHNICAL = 3;
export const EMPOWER_MAX_DAPP_ANALYST = 5;
export const EMPOWER_MAX_REGIONAL_EVANGELIST = 10;
export const MAX_LENGTH_COMMENT = 1024;
export const TOOLBAR_OPTIONS = [
    ['bold', 'italic', 'underline', 'strike'],
    [{'list': 'ordered'}, {'list': 'bullet'}]
];
export const LINKIFY_OPTION = {
    defaultProtocol: 'https',
    target: {
        url: '_self' // Does not work for some reason
    }
};
export const LANGUAGES = {
  english: 'en',
  chinese: 'zh'
}

