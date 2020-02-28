import _ from 'lodash'
import axios from 'axios'
import base64url from 'base64url'
import bs58 from 'bs58'
import moment from 'moment'
import utilCrypto from './crypto'
import mail from './mail'
import validate from './validate'
import sso from './sso'
import user from './user'
import * as permissions from './permissions'
import * as logger from './logger'
import * as fs from 'fs'
import * as path from 'path'

export { utilCrypto, sso, user, validate, permissions, mail, logger }

export const getEnv = () => process.env.NODE_ENV

export const loadKey = (filename: string) => {
  return fs.readFileSync(path.join(__dirname, filename));
}

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
    const res = await axios.post(
      process.env.DID_SIDECHAIN_URL,
      data,
      { headers }
    )
    if (res && res.data && res.data.result) {
      const base64 = _.get(res.data.result, 'transaction[0].operation.payload')
      const payload: any = base64url.decode(base64)
      const pubKeys = _.get(JSON.parse(payload), 'publicKey')
      const matched = pubKeys.find(el => el.id === '#primary')
      // compressed public key beginning with 02
      const publicKey = bs58.decode(matched.publicKeyBase58).toString('hex')
      return {
        expirationDate: moment(payload.expires),
        publicKey
      }
    }
  } catch (err) {
    logger.error(err)
  }
}
