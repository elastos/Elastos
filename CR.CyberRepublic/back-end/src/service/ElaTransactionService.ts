import Base from './Base'
import {constant} from '../constant'
import {ela, logger, getInformationByDid, getDidName, getCurrentHeight} from '../utility'
import * as moment from 'moment'

const _ = require('lodash')

export default class extends Base {
    private model: any
    private configModel: any

    protected init() {
        this.model = this.getDBModel('ElaTransaction')
        this.configModel = this.getDBModel('Config')
    }

    public async getAllTransaction() {
        const startHeight = process.env.FIRST_COUNCIL_HEIGHT
        const currentHeight = await getCurrentHeight()
        const currentConfig = await this.configModel.getDBInstance().findLast()
        const preHeight = (currentConfig && currentConfig.height ) || startHeight

        if (preHeight >= currentHeight) {
            return
        }

        const allTransactions = []
        for (let height = preHeight; height === currentHeight; height++){
            const transactions = await ela.getTransactionsByHeight(height);
            transactions.forEach((e) => {
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
            const bulk = this.model.getDBInstance().initializeUnorderedBulkOp();
            allTransactions.forEach((e) => bulk.insert(e))
            await bulk.execute()
        }

        if (!currentConfig) {
            await this.configModel.getDBInstance.add({
                height: currentHeight
            });
        } else {
            await this.configModel.getDBInstance.update({_id: currentConfig._id}, {
                height: currentHeight
            })
        }
    }

}
