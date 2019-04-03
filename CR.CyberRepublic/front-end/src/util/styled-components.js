import _ from 'lodash'

export const getSassColor = (styleObj, varName) => _.get(styleObj, ['global', varName, 'value', 'hex'])
