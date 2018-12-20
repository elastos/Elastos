import * as express from 'express';
import * as Rollbar from 'rollbar';
import * as bodyParser from 'body-parser';
import * as helmet from 'helmet';
import * as morgan from 'morgan';
import * as timeout from 'connect-timeout';
import * as session from 'express-session';
import * as ConnectMongo from 'connect-mongo';
import * as cors from 'cors';
import * as fileUpload from 'express-fileupload';
import * as compression from 'compression';
import * as fs from 'fs';
import db from './db';

import router, {middleware} from './router';

import './config';

let rollbar = null;

if (process.env.NODE_ENV === 'production') {
    rollbar = new Rollbar(process.env.ROLLBAR_TOKEN);
}

(async ()=>{
    const DB = await db.create();

    const app = express();

    app.set('trust proxy', true)
    app.use(cors());
    app.use(compression());
    app.options('*', cors());

    // TODO: seems resetTimeout in Base isn't working, this is the temp workaround
    const TIMEOUT = '600s';
    app.use(timeout(TIMEOUT));


    morgan.format('ebp', '[Backend] :method :url :status :res[content-length] - :response-time ms');
    app.use(morgan('ebp'));
    app.use(morgan('common', {stream: fs.createWriteStream('./access.log', {flags: 'a'})}))


    app.use(helmet());
    const bodyParserOptions = {
        strict: false,
        limit: '2mb'
    };
    app.use(bodyParser.json(bodyParserOptions));
    app.use(bodyParser.urlencoded({extended: false}));

    const SessionStore = ConnectMongo(session);
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
    }));

    // init router
    app.use(middleware);
    app.use(fileUpload());
    app.use('/api', router);

    if (process.env.NODE_ENV === 'production') {
        app.use(rollbar.errorHandler())
    }

    const port = process.env.SERVER_PORT;
    app.listen(port, () => {
        console.log(`start server at port ${port}`);
    });

})();

