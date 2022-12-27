import * as express from 'express'
import * as bodyParser from 'body-parser'
import * as helmet from 'helmet'
import * as morgan from 'morgan'
import * as timeout from 'connect-timeout'
import * as session from 'express-session'
import * as ConnectMongo from 'connect-mongo'
import * as cors from 'cors'
import * as fileUpload from 'express-fileupload'
import * as compression from 'compression'
import * as fs from 'fs'
import AccessControl from './utility/accessControl'
import db from './db'
import { logger } from './utility'

import router, {middleware} from './router'

import './config'
import './utility/agenda'

(async ()=>{
    const app = express()
    const DB = await db.create()
    const permissions = await DB.getModel('Permission').find()
    const prefix = '/api'

    app.set('trust proxy', true)
    app.use(cors())
    app.use(compression())
    app.options('*', cors())

    // TODO: seems resetTimeout in Base isn't working, this is the temp workaround
    const TIMEOUT = '600s'
    app.use(timeout(TIMEOUT))


    morgan.format('ebp', '[Backend] :method :url :status :res[content-length] - :response-time ms')
    app.use(morgan('ebp'))
    app.use(morgan('common', {stream: fs.createWriteStream('./access.log', {flags: 'a'})}))


    app.use(helmet())
    const bodyParserOptions = {
        strict: false,
        limit: '2mb'
    }
    app.use(bodyParser.json(bodyParserOptions))
    app.use(bodyParser.urlencoded({extended: false}))

    const SessionStore = ConnectMongo(session)
    app.use(session({
        name: 'ebp-token',
        secret: process.env.APP_SECRET || 'session_secret',
        store: new SessionStore({
            mongooseConnection: DB.connection
        }),
        saveUninitialized: false,
        resave: false,
        cookie: {
            secure: false,
            maxAge: 1000*60*60*24*30 // 30 days
        }
    }))

    // init router
    app.use(middleware)
    app.use(fileUpload())

    // setup access control for REST apis before the router middleware
    AccessControl(prefix, app, permissions)

    app.use(prefix, router)

    if (logger.rollbar()) {
        app.use(logger.rollbar().errorHandler())
    }

    const port = process.env.SERVER_PORT
    app.listen(port, () => {
        console.log(`start server at port ${port}`)
    })

})()
