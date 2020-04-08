import axios from 'axios'
import base64url from 'base64url'
import * as bs58 from 'bs58'
import * as moment from 'moment'
import utilCrypto from './crypto'
import mail from './mail'
import validate from './validate'
import sso from './sso'
import user from './user'
import * as permissions from './permissions'
import * as logger from './logger'
const _ = require('lodash')
const { PublicKey } = require('bitcore-lib-p256')
const jwkToPem = require('jwk-to-pem')

export { utilCrypto, sso, user, validate, permissions, mail, logger }

export const getEnv = () => process.env.NODE_ENV

export const uncompressPubKey = (key: any) => {
  if (!key.compressed) {
    throw new Error('Public key is not compressed.')
  }
  const x = key.point.getX()
  const y = key.point.getY()
  const xbuf = x.toBuffer({ size: 32 })
  const ybuf = y.toBuffer({ size: 32 })
  return Buffer.concat([Buffer.from([0x04]), xbuf, ybuf])
}

export const getPemPubKey = (key: any) => {
  if (!key.compressed) {
    throw new Error('Public key is not compressed.')
  }
  const x = key.point.getX()
  const y = key.point.getY()
  const jwk = {
    kty: 'EC',
    crv: 'P-256',
    x: x.toBuffer({ size: 32 }).toString('base64'),
    y: y.toBuffer({ size: 32 }).toString('base64')
  }
  return jwkToPem(jwk)
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
    const res = await axios.post(process.env.DID_SIDECHAIN_URL, data, {
      headers
    })
    if (res && res.data && res.data.result) {
      const base64 = _.get(res.data.result, 'transaction[0].operation.payload')
      const payload: any = base64url.decode(base64)
      const pubKeys = _.get(JSON.parse(payload), 'publicKey')
      const matched = pubKeys.find(el => el.id === '#primary')
      // compressed public key beginning with 02
      const publicKey = bs58.decode(matched.publicKeyBase58).toString('hex')
      const pemPubKey = getPemPubKey(PublicKey.fromString(publicKey))
      return {
        expirationDate: moment(payload.expires),
        publicKey: pemPubKey
      }
    }
  } catch (err) {
    logger.error(err)
  }
}
