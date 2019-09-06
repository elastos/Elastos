import Base from '../Base'
import { logger } from '../../utility'

export default class Logout extends Base {
  async action() {
    const destroySession = (): Promise<boolean> => {
      return new Promise((resolve, reject) => {
        this.session.destroy((err: any) => {
          if (err) {
            logger.error(err)
            reject(false)
          }
          resolve(true)
        })
      })
    }
    const rs = await destroySession()
    return this.result(1, rs)
  }
}
