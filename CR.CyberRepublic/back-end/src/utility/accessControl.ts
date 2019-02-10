import { Request, Response, NextFunction, Router } from 'express'
import db from '../db'
import * as _ from 'lodash'

/**
 * Install protecting routes into the app express application.
 * @param   {Object} app    The expressjs application to protect with
 * @param   {Object} permissions Define the path/method/roles configuration
 * @param {String} The prefix string of each path to protect
 */
const protectRoles = (prefix: String, app: any, permissions: Array<any>) => {
  _.each(permissions, permission => {
    const { httpMethod, url } = permission
    app[httpMethod](prefix + url, async (req: Request, res: Response, next: NextFunction) => {
      const DB = await db.create()
      const user = _.get(req, 'session.user')
      if (!user) return next()
      const userRole = _.get(user, 'role')
      try {
        const permissionRole = await DB.getModel('Permission_Role').findOne({
          url,
          httpMethod,
          role: userRole,
        })
        // false if there is no permision defined for this role
        req['isAccessAllowed'] = _.get(permissionRole, 'isAllowed', false)
        // DEV ONLY: SUPER_ADMIN has access to everything
        if (userRole === 'SUPER_ADMIN') {
          req['isAccessAllowed'] = true
        }
      } catch (err) {
        console.log('err happened: ', err)
      }
      next()
    })
  })
}

/**
 * The middleware to check if the request contains at least one roles enabled to
 * fullfill the requesting resource
 * @param   {Object}   req  the requsest object
 * @param   {Object} res  the response object
 * @param   {Function} next next middlware
 * @returns {Function} the next middleware or 401
 */
function checkRoleAuthorization(req: Request, res: Response, next: NextFunction) {
  const isAccessAllowed = _.get(req, 'isAccessAllowed')
  // when there is no isAccessAllowed field in req object,
  // we just allow the req to continue
  if (_.has(req, 'isAccessAllowed') && !isAccessAllowed) {
    return next('401 Unhautorized')
  }
  next()
}

export default async (prefix: String, app: any, permissions: Array<any>) => {
  // to protect the app with the path/method/roles defined in permissions
  // we need to create apps middlwares/routes for each path/method
  // (done by protectRoles) and then to add a middlware to check
  // if at least one requesting role satisfies the path/method roles
  // (done by checkRoleAuthorization)

  if (!prefix || typeof prefix != 'string') prefix = ''

  protectRoles(prefix, app, permissions)
  app.use(prefix + '*', checkRoleAuthorization)
}
