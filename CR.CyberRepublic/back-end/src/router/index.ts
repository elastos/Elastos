import { Request, Response, NextFunction, Router } from 'express'
import { getEnv } from '../utility'
import db from '../db'
import { utilCrypto } from '../utility'
import * as moment from 'moment'

import community from './community'
import cvote from './cvote'
import cvoteTracking from './cvote_tracking'
import cvoteSummary from './cvote_summary'
import google from './google'
import ping from './ping'
import permission from './permission'
import permissionRole from './permission_role'
import release from './release'
import sso from './sso'
import submission from './submission'
import suggestion from './suggestion'
import test from './test'
import team from './team'
import task from './task'
import taskCandidate from './task_candidate'
import teamCandidate from './team_candidate'
import upload from './upload'
import user from './user'
import elip from './elip'
import elipReview from './elip_review'
import council from './council'
import milestone from './milestone'

/**
 * Every request intercepts the token and sets the session user from the userId again
 *
 * @param {e.Request} req
 * @param {e.Response} res
 * @param {e.NextFunction} next
 * @returns {boolean}
 */
export const middleware = async (
  req: Request,
  res: Response,
  next: NextFunction
) => {
  // check token
  const token = req.headers['api-token']
  const DB = await db.create()

  if (token) {
    const json = JSON.parse(utilCrypto.decrypt(token.toString()))
    if (json.userId && json.expired && json.expired - moment().unix() > 0) {
      try {
        const user = await DB.getModel('User').findOne({ _id: json.userId })
        // TODO: find better way to not send the salt back to the front-end

        if (user) {
          delete user._doc.salt
          req['session'].user = user
          req['session'].userId = user.id
        }
      } catch (err) {
        console.log('err happened: ', err)
      }
    }
  } else if (req['session'].userId) {
    // check session
    const session = req['session']
    try {
      const user = await DB.getModel('User').findOne({ _id: session.userId })

      if (user) {
        req['session'].user = user
      }
    } catch (err) {
      console.log('err happened: ', err)
    }
  }
  next()
}

const router = Router()

if (getEnv() === 'dev') {
  router.use('/test', test)
}

router.use('/ping', ping)

router.use('/community', community)
router.use('/cvote', cvote)
router.use('/cvoteTracking', cvoteTracking)
router.use('/cvoteSummary', cvoteSummary)
router.use('/google', google)
router.use('/permission', permission)
router.use('/permissionRole', permissionRole)
router.use('/release', release)
router.use('/team', team)
router.use('/task', task)
router.use('/taskCandidate', taskCandidate)
router.use('/teamCandidate', teamCandidate)
router.use('/submission', submission)
router.use('/suggestion', suggestion)
router.use('/sso', sso)
router.use('/user', user)
router.use('/upload', upload)
router.use('/elip', elip)
router.use('/elipReview', elipReview)
router.use('/council', council)
router.use('/proposals', milestone)

router.use((req, res) => {
  return res.sendStatus(403)
})

export default router
