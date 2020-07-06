import axios from 'axios'
import * as logger from './logger'

const DEFAULT_HEADERS = {
    'Content-Type': 'application/json',
}

const DEFAULT_BLOCK_HEIGHT_PRO = 7 * 720
const DEFAULT_BLOCK_HEIGHT_DEV = 40

const ela = {
    async height() {
        const data = {
            method: 'getcurrentheight'
        }
        try {
            const res = await axios.post(process.env.ELA_NODE_URL, data, {
                headers: DEFAULT_HEADERS
            })
            if (res && res.data && res.data.result) {
                return res.data.result
            }
        } catch (err) {
            logger.error(err)
        }
    },
    async balance(address: string) {
        const data = {
            'method': 'getreceivedbyaddress',
            'params': {
                address
            }
        }
        try {
            const res = await axios.post(process.env.ELA_NODE_URL, data, {
                headers: DEFAULT_HEADERS
            })
            if (res && res.data && res.data.result) {
                return res.data.result
            }
        } catch (err) {
            logger.error(err)
        }
    },
    async getSecretaryGeneral() {
        const data = {
            'method': 'getsecretarygeneral'
        }
        try {
            const res = await axios.post(process.env.ELA_NODE_URL, data, {
                headers: DEFAULT_HEADERS
            })
            if (res && res.data && res.data.result) {
                return res.data.result
            }
        } catch (err) {
            logger.error(err)
        }
    },
    async circulatingSupply(height) {
        return 33000000
            + height * 5.02283105
            - await this.balance('CRASSETSXXXXXXXXXXXXXXXXXXXX2qDX5J')
            - await this.balance('CREXPENSESXXXXXXXXXXXXXXXXXX4UdT6b')
            - await this.balance('ELANULLXXXXXXXXXXXXXXXXXXXXXYvs3rr')
    },
    async currentCirculatingSupply() {
        return this.circulatingSupply(await this.height())
    },
    async currentCouncil() {
        const data = {
            'method': 'listcurrentcrs',
            'params': {
                'state': 'all'
            }
        }
        try {
            const res = await axios.post(process.env.ELA_NODE_URL, data, {
                headers: DEFAULT_HEADERS
            })
            if (res && res.data && res.data.result) {
                return res.data.result
            }
        } catch (err) {
            logger.error(err)
        }
    },
    async currentCandidates() {
        const data = {
            'method': 'listcrcandidates',
            'params': {
                'state': 'active'
            }
        }
        try {
            const res = await axios.post(process.env.ELA_NODE_URL, data, {
                headers: DEFAULT_HEADERS
            })
            if (res && res.data && res.data.result) {
                return res.data.result
            }
        } catch (err) {
            logger.error(err)
        }
    },
    async getCrrelatedStage() {
        const data = {
            'method': 'getcrrelatedstage',
            'params': {}
        }
        try {
            const res = await axios.post(process.env.ELA_NODE_URL, data, {
                headers: DEFAULT_HEADERS
            })
            if (res && res.data && res.data.result) {
                return res.data.result
            }
        } catch (err) {
            logger.error(err)
        }
    },
    async getBlockByHeight(height) {
        const data = {
            'method': 'getblockbyheight',
            'params': {
                'height': height
            }
        }
        try {
            const res = await axios.post(process.env.ELA_NODE_URL, data, {
                headers: DEFAULT_HEADERS
            })
            if (res && res.data && res.data.result) {
                return res.data.result
            }
        } catch (err) {
            logger.error(err)
        }
    },
    async getTimestampByHeight(height) {
        const result = await this.getBlockByHeight(height)
        return result && result.time
    },
    async getTransactionsByHeight(height) {
        const result = await this.getBlockByHeight(height)
        return result && result.tx
    },
    async depositCoin(did: string) {
        const data = {
            'method': 'getcrdepositcoin',
            'params': {
                'id': did
            }
        }
        try {
            const res = await axios.post(process.env.ELA_NODE_URL, data, {
                headers: DEFAULT_HEADERS
            })
            if (res && res.data && res.data.result) {
                return res.data.result
            }
        } catch (err) {
            logger.error(err)
        }
    },
    async calHeightTime(height) {
        const oneMinute = 1000 * 60
        const mProposed = (DEFAULT_BLOCK_HEIGHT_PRO * 2 - 30) * oneMinute
        const mNotification = (DEFAULT_BLOCK_HEIGHT_PRO * 2 * 2 - 30) * oneMinute
        const startTime = await this.getTimestampByHeight(height)
        const proposedEnds = mProposed + startTime * 1000
        const notificationEnds = mNotification + startTime * 1000
        return { proposedEnds, notificationEnds}
    }
}
export default ela
