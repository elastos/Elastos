import * as dotenv from 'dotenv'
import * as path from 'path'

const env = process.env.NODE_ENV || 'dev'

try{
    const configFilePath = path.resolve('', process.cwd() + `/env/${env}.env`)
    const config = dotenv.config({ path: configFilePath })
    console.log(config.parsed)
}catch(e){
    console.error(e)
}
