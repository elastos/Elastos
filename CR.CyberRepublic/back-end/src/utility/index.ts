import axios from 'axios'
import base64url from 'base64url'
import * as bs58 from 'bs58'
import * as moment from 'moment'
import utilCrypto from './crypto'
import mail from './mail'
import validate from './validate'
import sso from './sso'
import user from './user'
import timestamp from './timestamp'
import ela from './ela'
import * as permissions from './permissions'
import * as logger from './logger'
const _ = require('lodash')
const { PublicKey } = require('bitcore-lib-p256')
const jwkToPem = require('jwk-to-pem')
import * as jwt from 'jsonwebtoken'
const Big = require('big.js')

export {
  utilCrypto,
  sso,
  user,
  timestamp,
  ela,
  validate,
  permissions,
  mail,
  logger
}

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

export const getPemPublicKey = (publicKey: any) => {
  const key = PublicKey.fromString(publicKey)
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
    'Content-Type': 'application/json'
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
      if (!base64) {
        throw 'Can not get DID payload'
      }
      const payload: any = base64url.decode(base64)
      if (!payload) {
        throw 'Can not get decode DID payload'
      }
      const pubKeys = _.get(JSON.parse(payload), 'publicKey')
      if (!pubKeys) {
        throw 'Can not get DID public keys'
      }
      const matched = pubKeys.find((el) => el.id === '#primary')
      if (!matched) {
        throw 'Can not get DID primary key'
      }
      // compressed public key beginning with 02
      const publicKey = bs58.decode(matched.publicKeyBase58).toString('hex')
      if (!publicKey) {
        throw 'Can not decode DID primary key'
      }
      const pemPubKey = getPemPublicKey(publicKey)
      return {
        publicKey: pemPubKey,
        compressedPublicKey: publicKey
      }
    }
  } catch (err) {
    logger.error(err)
  }
}

export const getUtxosByAmount = async (amount: string) => {
  const headers = {
    'Content-Type': 'application/json'
  }
  const data = {
    jsonrpc: '2.0',
    method: 'getutxosbyamount',
    params: {
      address: 'CREXPENSESXXXXXXXXXXXXXXXXXX4UdT6b',
      amount,
      utxotype: 'unused'
    }
  }
  try {
    const res = await axios.post(process.env.ELA_NODE_URL, data, {
      headers
    })
    if (res && res.data) {
      const utxos = _.get(res.data, 'result')
      if (utxos === null) {
        return { success: false, utxos: null }
      }
      if (utxos) {
        let arr = []
        for (let i = 0; i < utxos.length; i++) {
          const item = utxos[i]
          if (item.confirmations < 2) {
            return { success: false, utxos: null }
          }
          const amount = Big(`${item.amount}e+8`).toString()
          arr.push({ txid: item.txid, vout: item.vout, amount })
        }
        return { success: true, utxos: arr }
      }
    }
  } catch (err) {
    logger.error(err)
  }
}

export const getProposalState = async (query: {
  drafthash?: string
  proposalhash?: string
}) => {
  const headers = {
    'Content-Type': 'application/json'
  }
  const data = {
    jsonrpc: '2.0',
    method: 'getcrproposalstate',
    params: query
  }
  try {
    const res = await axios.post(process.env.ELA_NODE_URL, data, {
      headers
    })
    if (res) {
      const status = _.get(res.data, 'result.proposalstate.status')
      const proposal = _.get(res.data, 'result.proposalstate.proposal')
      const proposalHash = _.get(res.data, 'result.proposalstate.proposalhash')
      if (status) {
        return { success: true, status, proposal, proposalHash }
      }
    }
  } catch (err) {
    logger.error(err)
  }
}

export const getProposalData = async (proposalHash: string) => {
  const headers = {
    'Content-Type': 'application/json'
  }
  const data = {
    jsonrpc: '2.0',
    method: 'getcrproposalstate',
    params: {
      proposalhash: proposalHash
    }
  }
  try {
    const res = await axios.post(process.env.ELA_NODE_URL, data, {
      headers
    })
    if (res) {
      const status = _.get(res.data, 'result.proposalstate.status')
      const data = _.get(res.data, 'result.proposalstate')
      if (status) {
        return { success: true, status, data }
      }
    }
  } catch (err) {
    logger.error(err)
  }
}

export const getInformationByDid = async (did: string) => {
  const data = {
    did: did
  }
  try {
    const res = await axios.post(
      `${process.env.UNIONSQUARE_URL}/api/dposnoderpc/check/jwtget`,
      data
    )
    const publicKeyObj: any = await getDidPublicKey(did)
    const jwtToken = res && res.data && res.data.data && res.data.data.jwt
    if (jwtToken && publicKeyObj) {
      return jwt.verify(
        jwtToken,
        publicKeyObj.publicKey,
        async (err: any, decoded: any) => {
          if (err) {
            logger.error(err)
          }
          return decoded && decoded.credentialSubject
        }
      )
    }
  } catch (err) {
    logger.error(err)
  }
}

export const getDidName = async (did: string) => {
  const headers = {
    'Content-Type': 'application/json'
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
      if (!base64) {
        throw 'Can not get DID payload'
      }
      const payload: any = base64url.decode(base64)
      const { verifiableCredential } = JSON.parse(payload)
      if (!_.isEmpty(verifiableCredential)) {
        const verifiableCredentialById = _.keyBy(verifiableCredential, 'id')
        return (
          verifiableCredentialById['#name'] &&
          _.get(verifiableCredentialById['#name'], 'credentialSubject.name')
        )
      }
    }
  } catch (err) {
    logger.error(err)
  }
}

export const getVoteResultByTxid = async (txid: string) => {
  const headers = {
    'Content-Type': 'application/json'
  }
  const data = {
    jsonrpc: '2.0',
    method: 'getrawtransaction',
    params: {
      txid: txid,
      verbose: true
    }
  }
  try {
    const res = await axios.post(process.env.ELA_NODE_URL, data, {
      headers
    })
    if (res && res.data && res.data.result) {
      const confirmations = _.get(res.data.result, 'confirmations')
      if (confirmations && confirmations > 0) {
        return true
      }
    }
    return false
  } catch (err) {
    logger.error(err)
  }
}

export const getCurrentHeight = async () => {
  const headers = {
    'Content-Type': 'application/json'
  }
  const data = {
    jsonrpc: '2.0',
    method: 'getcurrentheight'
  }
  try {
    const res = await axios.post(process.env.ELA_NODE_URL, data, {
      headers
    })
    if (res) {
      const height = _.get(res.data, 'result')
      if (height) {
        return { success: true, height }
      }
    }
  } catch (err) {
    logger.error(err)
  }
}
