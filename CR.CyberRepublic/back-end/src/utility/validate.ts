import * as _ from 'lodash'
import * as validator from 'validator'

const F = {
    email(email){
        return validator.isEmail(email)
    },

    valid_string(str, min, max=32768){
        if(!str || !_.isString(str)) return false
        const len = str.length
        if(len < min) return false
        if(len > max) return false

        return true
    }
}

export default F
