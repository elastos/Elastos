import * as mongoose from 'mongoose'
import * as _ from 'lodash'
import {Document} from 'mongoose'

/**
 * This is an abstraction layer around the db model
 *
 * Use getDBInstance if you need access to the native Mongoose model
 *
 * NOTE: using the functions here is ill advised, we should move towards an agreement
 * to not overwrite and simplify already provided functions
 *
 * You must use getDBInstance if you need to use .populate and foreign keys
 *
 * However added functionality like findByIdAndDelete are fine
 */
export default abstract class {
    private db
    private schema
    private reject_fields: object

    constructor(DB){
        this.schema = this.buildSchema()
        this.db = DB.model(this.getName(), this.schema)

        this.reject_fields = _.extend({
            __v : false
        }, this.rejectFields())
    }

    protected buildSchema(){
        const schema = new mongoose.Schema(_.extend({
            createdAt: {
                type: Date,
                default: Date.now
            },
            updatedAt: {
                type: Date,
                default: Date.now
            }
        }, this.getSchema()), _.extend({
            timestamps: true
        }, this.getSchemaOption()))

        return schema
    }

    protected abstract getSchema(): mongoose.SchemaDefinition
    protected getSchemaOption(): mongoose.SchemaOptions{
        return {}
    }
    protected abstract getName(): string
    protected rejectFields(): object{
        return {}
    }

    public async save(doc: object): Promise<Document>{
        return await this.db.create(doc)
    }

    // BUG: seems we can't chain sort on the result
    public async find(query, opts?): Promise<Document[]>{
        const option = this.buildFindOptions(opts)
        const reject_fields = option.reject ? this.reject_fields : {}
        let res = await this.db.find(query, reject_fields)
        return res
    }

    public async findById(id, opts?): Promise<Document>{
        return await this.findOne({_id: id}, opts)
    }

    public async findOne(query, opts?): Promise<Document>{
        const option = this.buildFindOptions(opts)
        const reject_fields = option.reject ? this.reject_fields : {}
        return await this.db.findOne(query, reject_fields)
    }

    public async findByIdAndDelete(id): Promise<Document>{
        return await this.db.findByIdAndDelete(id)
    }

    public async findOneAndDelete(query): Promise<Document>{
        return await this.db.findOneAndDelete(query)
    }

    public async update(query, doc, opts?: updateOptions): Promise<Document>{
        return await this.db.update(query, doc, this.buildUpdateOptions(opts))
    }

    // TODO why can not run, will lead a memory leak error.
    public async findOneAndUpdate(query, doc, opts?): Promise<Document>{
        return await this.db.findOneAndUpdate(query, doc, opts)
    }

    public async count(query): Promise<number>{
        return await this.db.count(query)
    }
    public async list(query, sort?, limit?, select?: string): Promise<[Document]>{
        return await this.db.find(query).select(select || '').sort(sort || {}).limit(_.toNumber(limit) || 1000)
    }

    public getAggregate(){
        return this.db.aggregate()
    }
    public getDBInstance(){
        return this.db
    }

    public async remove(query){
        return await this.db.remove(query)
    }

    private buildUpdateOptions(opts?: updateOptions): updateOptions{
        return _.extend({
            multi : false
        }, opts||{})
    }
    private buildFindOptions(opts?: findOptions): findOptions{
        return _.extend({
            reject: true
        }, opts||{})
    }
}

interface updateOptions {
    multi?: boolean
}
interface findOptions {
    reject?: boolean
}
