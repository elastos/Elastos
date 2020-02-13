import _ from 'lodash'
import axios from 'axios'
import base64url from 'base64url'
import bs58 from 'bs58'
import utilCrypto from './crypto'
import mail from './mail'
import validate from './validate'
import sso from './sso'
import user from './user'
import * as permissions from './permissions'
import * as logger from './logger'

export { utilCrypto, sso, user, validate, permissions, mail, logger }

export const getEnv = () => process.env.NODE_ENV

export const getDidPublicKey = async (did: string) => {
  const headers = {
    'Content-Type': 'application/json',
    Authorization: process.env.DID_SIDECHAIN_AUTH
  }
  const data = {
    jsonrpc: '2.0',
    method: 'resolvedid',
    params: {
      did,
      all: false
    }
  }
  try {
    const res = await axios.post(process.env.DID_SIDECHAIN_URL, data, {
      headers
    })
    if (res && res.data && res.data.result) {
      const base64 = _.get(res.data.result, 'transaction[0].operation.payload')
      const payload = base64url.decode(base64)
      const base58 = _.get(JSON.parse(payload), 'publicKey[0].publicKeyBase58')
      return bs58.decode(base58).toString('hex')
    }
  } catch (err) {
    logger.error(err)
  }
}
