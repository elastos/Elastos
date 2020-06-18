import Base from './Base'
import {constant} from '../constant'
import {ela, logger, getCurrentHeight} from '../utility'

const _ = require('lodash')

export default class extends Base {
    private model: any
    private configModel: any

    protected init() {
        this.model = this.getDBModel('Ela_Transaction')
        this.configModel = this.getDBModel('Config')
    }

    public async appendAllTransaction() {
        const startHeight = process.env.NODE_ENV === 'production' ? 10000 : 799
        const currentHeight = await ela.height()
        const currentConfig = await this.configModel.getDBInstance().findOne()
        const preHeight = (currentConfig && currentConfig.height) || startHeight

        if (preHeight >= currentHeight) {
            return
        }

        const allTransactions = []
        for (let height = preHeight + 1; height <= currentHeight; height++) {
            const transactions = await ela.getTransactionsByHeight(height)
            transactions.forEach((e: any) => {
                if (e && [constant.TRANSACTION_TYPE.COUNCIL_VOTE].includes(e.type)) {
                    allTransactions.push({
                        txid: e.txid,
                        payload: e.payload && JSON.stringify(e.payload),
                        type: e.type
                    })
                }
            });
        }
        if (allTransactions.length > 0) {
            const bulk = this.model.getDBInstance().collection.initializeUnorderedBulkOp()
            allTransactions.forEach((e) => bulk.insert(e))
            await bulk.execute()
        }

        if (!currentConfig) {
            await this.configModel.getDBInstance().create({
                height: currentHeight
            });
        } else {
            await this.configModel.getDBInstance().update({_id: currentConfig._id}, {
                height: currentHeight
            })
        }
    }

}
