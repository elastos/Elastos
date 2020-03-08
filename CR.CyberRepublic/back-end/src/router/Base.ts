import {Response, Request, Router} from 'express'
import * as Session from 'express-session'
import Service from '../service/Base'
import * as _ from 'lodash'
import DB from '../db'
import { logger } from '../utility'

interface RESULT {
    code: number
    data?: any
    error?: Error
    message?: string
}

interface ROUTEING {
    path: string
    router: any
    method: string
    timeout?: number
}

export default abstract class {
    static setRouter(list: ROUTEING[]): Router{
        const router = Router()
        _.each(list, (item)=>{

            // this runs on init

            // this sets the callback for router.[method]
            router[item.method](item.path, (req, res)=>{

                // this block is the callback
                // for the file upload - we remove any timeout
                /*
                if (item.router.name === 'UploadFile' && item.path === '/file') {
                    req.clearTimeout()
                }
                */
                const c = new item.router(req, res)
                return c.main()
            })
        })
        return router
    }


    protected req: Request
    protected res: Response
    protected session: Session
    protected db

    protected needLogin = false

    constructor(req, res) {
        this.req = req
        this.res = res
        this.session = req.session
        this.init()
    }

    protected init(){}

    public async main(): Promise<any> {
        try{
            if(!await this.validate()){
                return this.result(-1, { code: 401, message: 'Please login'})
            }

            this.db = await DB.create()
            const result = await this.action()
            if(result){
                if (result.data && _.isNumber(result.data.code)) {
                    this.res.status(result.data.code).json(result.data)
                } else {
                    this.res.set('Content-Type', 'application/json')
                    this.res.json(result)
                }
            }

        }catch(e){
            logger.error(e)
            this.res.json(this.result(-1, e))
        }
    }

    private async validate(){
        // check need login or not
        if(this.needLogin){
            if(!this.session.user){
                this.res.sendStatus(401)
                return false
            }
        }

        return true
    }

    // need to override
    abstract async action()


    protected result(code, dataOrError, msg?){
        const opts: RESULT = {
            code,
            data: dataOrError,
            error : dataOrError,
            message : msg
        }
        if(opts.code > 0){
            return {
                code : opts.code,
                data : opts.data,
                message : opts.message || 'ok'
            }
        }
        else{
            const err = opts.error
            return {
                code : err['code'] ? -err['code'] : opts.code,
                type : err.name || '',
                error : err.message || err.toString()
            }
        }

    }

    /*
    * get service
    * */
    protected buildService<T extends Service>(service: { new(...args): T }): T{
        return new service(this.db, this.session)
    }

    protected getParam(key?: string): any{
        const param = _.extend({}, this.req.query, this.req.body, this.req.params)
        return key ? _.get(param, key, '') : param
    }

}
