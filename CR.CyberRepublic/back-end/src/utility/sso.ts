import crypto from './crypto'
import * as querystring from 'querystring'

const sso = {
    validate (payload, sig) {
        const str = querystring.unescape(payload)
        return crypto.sha256(str) === sig ? true : false
    },
    getNonce (payload) {
        const q = querystring.parse(
            new Buffer(querystring.unescape(payload), 'base64').toString()
        )
        if ('nonce' in q) {
            return q['nonce']
        } else {
            throw new Error('Missing Nonce in payload!')
        }
    },
    buildLoginString (params) {
        if (!('external_id' in params)) {
            throw new Error('Missing required parameter: external_id')
        }
        if (!('nonce' in params)) {
            throw new Error('Missing required parameter: nonce')
        }
        if (!('email' in params)) {
            throw new Error('Missing required parameter: email')
        }

        const payload = new Buffer(querystring.stringify(params), 'utf8').toString('base64')

        return querystring.stringify({
            'sso': payload,
            'sig': crypto.sha256(payload)
        })
    }
}

export default sso