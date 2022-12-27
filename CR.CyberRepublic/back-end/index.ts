console.log(`Starting "${process.env.NODE_ENV}"`)
if (process.env.NODE_ENV === 'staging' || process.env.NODE_ENV === 'production') {
    require('newrelic')
}
require('./src/app')
