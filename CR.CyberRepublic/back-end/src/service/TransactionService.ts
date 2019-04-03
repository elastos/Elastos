import Base from './Base'
import {Document} from 'mongoose'
import * as _ from 'lodash'
import {constant} from '../constant'
import {validate, utilCrypto} from '../utility'

export default class extends Base {

    /**
     * Add a transaction, this initializes to PENDING
     *
     * @param param
     * @returns {Promise<"mongoose".Document>}
     */
    public async create(param): Promise<Document> {
        return undefined
    }

    /**
     * Only the admin/leader can change certain fields like the status to SENT
     *
     * @param param
     * @returns {Promise<boolean>}
     */
    public async update(param): Promise<boolean> {
        return undefined
    }

    public async index(param): Promise<[Document]> {
        return undefined
    }
}
