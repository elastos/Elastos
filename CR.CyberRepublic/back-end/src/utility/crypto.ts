import * as hmacSha512 from 'crypto-js/hmac-sha512'
import * as aes from 'crypto-js/aes'
import * as utf8 from 'crypto-js/enc-utf8'
import * as crypto from 'crypto'

const secret = process.env.APP_SECRET || 'app'
/**
 * The Production APP_SECRET if leaked with the salt will compromise all accounts
 */
export default {
  sha512(str: string) {
    return hmacSha512(str, secret).toString()
  },
  sha256(str: string, secret?: string) {
    const localSecret = secret || process.env.SSO_SECRET || 'SSO_SECRET'
    const hmac = crypto.createHmac('sha256', localSecret)
    hmac.update(str)
    return hmac.digest('hex')
  },
  encrypt(str: string) {
    return aes
      .encrypt(aes.encrypt(str, secret).toString(), secret + '_x')
      .toString()
  },
  decrypt(str: string) {
    return aes
      .decrypt(aes.decrypt(str, secret + '_x').toString(utf8), secret)
      .toString(utf8)
  },
  async randomHexStr(numBytes) {
    const buffer = await crypto.randomBytes(numBytes)
    return buffer.toString('hex')
  },
  sha256D(str: string) {
    const hash = crypto.createHash('sha256')
    const rs = hash.update(str).digest()
    const normalHash = crypto.createHash('sha256').update(rs).digest('hex')
    const reverseHash = normalHash
      .match(/[a-fA-F0-9]{2}/g)
      .reverse()
      .join('')
    return reverseHash
  }
}
