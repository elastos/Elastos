import Base from './Base'
import {Document} from 'mongoose'
import * as _ from 'lodash'
import {constant} from '../constant'
import {geo} from '../utility/geo'
import * as uuid from 'uuid'
import {
    validate,
    utilCrypto,
    mail,
    permissions,
    getDidPublicKey,
    logger,
    getInformationByDid,
    getDidName,
} from '../utility'
import CommunityService from './CommunityService'
import * as jwt from 'jsonwebtoken'

const selectFields = `-logins -salt -password -elaBudget -elaOwed -votePower -resetToken`
const strictSelectFields = selectFields + ' -email -profile.walletAddress'

const restrictedFields = {
    update: ['_id', 'username', 'role', 'profile', 'salt']
}

let tm = undefined

export default class extends Base {
    protected async init() {
        // await this.cronJob()
    }

    /**
     * On registration we also add them to the country community,
     * if it doesn't exist yet we will create it as well
     *
     * @param param
     * @returns {Promise<"mongoose".Document>}
     */
    public async registerNewUser(param): Promise<Document> {
        const db_user = this.getDBModel('User')

        const username = param.username.toLowerCase()
        const email = param.email.toLowerCase()

        this.validate_username(username)
        this.validate_password(param.password)
        this.validate_email(email)

        // check username and email unique
        if (await db_user.findOne({username})) {
            throw 'This username is already taken'
        }
        if (await db_user.findOne({email: email})) {
            throw 'This email is already taken'
        }

        const salt = uuid.v4()

        const doc: any = {
            username,
            password: this.getPassword(param.password, salt),
            email,
            salt,
            profile: {
                firstName: param.firstName,
                lastName: param.lastName,
                country: param.country,
                timezone: param.timezone,
                state: param.state,
                city: param.city,
                beOrganizer: param.beOrganizer === 'yes',
                isDeveloper: param.isDeveloper === 'yes',
                source: param.source
            },
            role: constant.USER_ROLE.MEMBER,
            active: true
        }

        // simply validate did format
        if (param.did && _.isString(param.did) && param.did.length === 46) {
            const rs = param.did.split(':')
            if (rs.length === 3 && rs[0] === 'did' && rs[1] === 'elastos') {
                doc.did = {id: param.did}
            }
        }

        if (process.env.NODE_ENV === 'test') {
            if (param._id) {
                doc._id = param._id.$oid
            }
        }

        const newUser = await db_user.save(doc)

        await this.linkCountryCommunity(newUser)
        this.sendConfirmation(doc)

        return newUser
    }

    // record user login date
    public async recordLogin(param) {
        const db_user = this.getDBModel('User')
        await db_user.update(
            {_id: param.userId},
            {$push: {logins: new Date()}}
        )
    }

    public async getUserSalt(username): Promise<String> {
        const isEmail = validate.email(username)
        username = username.toLowerCase()

        const query = {[isEmail ? 'email' : 'username']: username}

        const db_user = this.getDBModel('User')
        const user = await db_user.db.findOne(query)

        if (!user) {
            throw 'invalid username or email'
        }
        return user.salt
    }

    /**
     * TODO: ensure we have a test to ensure param.admin is checked properly (currently true)
     * @param param
     * @returns {Promise<"mongoose".DocumentQuery<T extends "mongoose".Document, T extends "mongoose".Document>>}
     */
    public async show(param) {
        const {userId} = param
        const db_user = this.getDBModel('User')
        const userRole = _.get(this.currentUser, 'role')
        const isUserAdmin = permissions.isAdmin(userRole)
        const isSelf = _.get(this.currentUser, '_id') === userId
        let fields = isUserAdmin || isSelf ? selectFields : strictSelectFields

        if (param.admin && !isUserAdmin && !isSelf) {
            throw 'Access Denied'
        }

        const user = await db_user
            .getDBInstance()
            .findOne({
                _id: userId,
                $or: [{banned: {$exists: false}}, {banned: false}]
            })
            .select(fields)
            .populate('circles')

        if (!user) {
            throw `userId: ${userId} not found`
        }

        if (user.comments) {
            for (let comment of user.comments) {
                for (let thread of comment) {
                    await db_user.getDBInstance().populate(thread, {
                        path: 'createdBy',
                        select: fields
                    })
                }
            }

            for (let subscriber of user.subscribers) {
                await db_user.getDBInstance().populate(subscriber, {
                    path: 'user',
                    select: fields
                })
            }
        }

        return user
    }

    public async updateRole(param) {
        const {userId, role} = param
        const db_user = this.getDBModel('User')
        const userRole = _.get(this.currentUser, 'role')
        const isUserAdmin = permissions.isAdmin(userRole)

        if (!isUserAdmin) {
            throw 'Access Denied'
        }

        if (Object.keys(constant.USER_ROLE).indexOf(role) === -1) {
            throw 'invalid role'
        }
        return await db_user.update({_id: userId}, {role})
    }

    public async update(param) {
        const {userId} = param
        const updateObj: any = _.omit(param, restrictedFields.update)
        const db_user = this.getDBModel('User')
        let user = await db_user.findById(userId)
        const isSelf = _.get(this.currentUser, '_id', '').toString() === userId
        const userRole = _.get(this.currentUser, 'role')
        const isUserAdmin = permissions.isAdmin(userRole)
        const canUpdate = isUserAdmin || isSelf
        let countryChanged = false
        let fields = isUserAdmin || isSelf ? selectFields : strictSelectFields

        if (!canUpdate) {
            throw 'Access Denied'
        }

        if (!user) {
            throw `userId: ${userId} not found`
        }

        if (
            param.profile &&
            param.profile.country &&
            param.profile.country !== user.profile.country
        ) {
            countryChanged = true
        }

        if (param.profile) {
            updateObj.profile = Object.assign(user.profile, param.profile)

            if (param.profile.skillset) {
                updateObj.profile.skillset = param.profile.skillset
            }
        }

        if (param.timezone) {
            updateObj.timezone = param.timezone
        }

        if (param.email) {
            updateObj.email = param.email
        }

        if (param.password) {
            const salt = uuid.v4()
            updateObj.password = this.getPassword(param.password, salt)
            updateObj.salt = salt
        }

        if (param.removeAttachment) {
            updateObj.avatar = undefined
            updateObj.avatarFileType = ''
            updateObj.avatarFilename = ''
        }

        if (param.removeBanner) {
            updateObj.banner = undefined
            updateObj.bannerFileType = ''
            updateObj.bannerFilename = ''
        }

        if (param.popupUpdate) {
            updateObj.popupUpdate = param.popupUpdate
        }

        await db_user.update({_id: userId}, updateObj)

        user = db_user
            .getDBInstance()
            .findOne({_id: userId})
            .select(fields)
            .populate('circles')

        // if we change the country, we add the new country as a community if not already
        // keep the old one too - TODO: think through this logic, maybe we only keep the old one if the new one already exists
        if (countryChanged) {
            await this.linkCountryCommunity(user)
        }

        return user
    }

    public async findUser(query): Promise<Document> {
        const db_user = this.getDBModel('User')
        const isEmail = validate.email(query.username)
        return await db_user
            .getDBInstance()
            .findOne({
                [isEmail ? 'email' : 'username']: query.username.toLowerCase(),
                password: query.password,
                $or: [{banned: {$exists: false}}, {banned: false}]
            })
            .select(selectFields)
            .populate('circles')
    }

    public async findUserByDid(did: string): Promise<Document> {
        const db_user = this.getDBModel('User')
        const query = {'did.id': did}
        return await db_user.getDBInstance().findOne(query, selectFields)
    }

    public async findUsers(query): Promise<Document[]> {
        const db_user = this.getDBModel('User')

        return await db_user
            .getDBInstance()
            .find({
                _id: {
                    $in: query.userIds
                }
            })
            .select(strictSelectFields)
    }

    /*
     ************************************************************************************
     * Find All Users
     * - be very restrictive here, careful to not select sensitive fields
     * - TODO: may need sorting by full name for Empower 35? Or something else?
     ************************************************************************************
     */
    public async findAll(query): Promise<Object> {
        const db_user = this.getDBModel('User')

        const finalQuery: any = {
            active: true,
            archived: {$ne: true}
        }

        if (query.search) {
            finalQuery.$and = _.map(_.trim(query.search).split(' '), (part) => {
                return {
                    $or: [
                        {'profile.firstName': {$regex: part, $options: 'i'}},
                        {'profile.lastName': {$regex: part, $options: 'i'}},
                        {username: {$regex: part, $options: 'i'}}
                    ]
                }
            })
        }

        if (query.skillset) {
            const skillsets = query.skillset.split(',')
            finalQuery['profile.skillset'] = {$in: skillsets}
        }

        if (query.profession) {
            const professions = query.profession.split(',')
            finalQuery['profile.profession'] = {$in: professions}
        }

        if (query.empower) {
            finalQuery.empower = JSON.parse(query.empower)
        }

        const cursor = db_user.getDBInstance().find(finalQuery)
        const totalCursor = db_user.getDBInstance().find(finalQuery).count()

        if (query.results) {
            const results = parseInt(query.results, 10)
            const page = parseInt(query.page, 10)
            cursor.skip(results * (page - 1)).limit(results)
        }

        cursor.select(strictSelectFields).sort({username: 1})

        const users = await cursor
        const total = await totalCursor

        if (users.length) {
            const db_team = this.getDBModel('Team')

            for (let user of users) {
                await db_team.getDBInstance().populate(user, {
                    path: 'circles'
                })
            }
        }

        return {
            list: users,
            total
        }
    }

    public async getCouncilMembers(): Promise<Object> {
        const db_user = this.getDBModel('User')
        const query = {role: constant.USER_ROLE.COUNCIL}
        const councilMembers = await db_user
            .getDBInstance()
            .find(query)
            .select(constant.DB_SELECTED_FIELDS.USER.NAME_EMAIL)
        return {
            list: councilMembers
        }
    }

    public async changePassword(param): Promise<boolean> {
        const db_user = this.getDBModel('User')

        const {oldPassword, password} = param
        const username = param.username.toLowerCase()
        const userRole = _.get(this.currentUser, 'role')
        const isUserAdmin = permissions.isAdmin(userRole)
        const isSelf = _.get(this.currentUser, 'username') === username

        this.validate_password(oldPassword)
        this.validate_password(password)
        this.validate_username(username)

        if (!isUserAdmin && !isSelf) {
            throw 'Access Denied'
        }

        let user = await db_user.findOne({username}, {reject: false})
        if (!user) {
            throw 'user does not exist'
        }

        if (user.password !== this.getPassword(oldPassword, user.salt)) {
            throw 'old password is incorrect'
        }

        const res = await db_user.update(
            {username},
            {
                $set: {
                    password: this.getPassword(password, user.salt)
                }
            }
        )

        user = db_user.getDBInstance().findOne({username}).populate('circles')

        return user
    }

    /*
     ******************************************************************************************
     * Forgot/Reset Password
     *
     * The idea here is to ensure that the user gets no hint the email exists
     ******************************************************************************************
     */
    public async forgotPassword(param) {
        const {email} = param

        console.log(`forgotPassword called on email: ${email}`)

        const db_user = this.getDBModel('User')

        const userEmailMatch = await db_user.findOne({
            email: email,
            active: true
        })

        if (!userEmailMatch) {
            console.error('no user matched')
            return
        }

        // add resetToken
        const resetToken = await utilCrypto.randomHexStr(8)

        await userEmailMatch.update({
            resetToken
        })

        // send email
        await mail.send({
            to: userEmailMatch.email,
            toName: `${userEmailMatch.profile.firstName} ${userEmailMatch.profile.lastName}`,
            subject: 'Cyber Republic - Password Reset',
            body: `For your convenience your username is ${userEmailMatch.username}
                <br/>
                <br/>
                Please click this link to reset your password:
                <a href="${process.env.SERVER_URL}/reset-password?token=${resetToken}">${process.env.SERVER_URL}/reset-password?token=${resetToken}</a>`
        })
    }

    public async resetPassword(param) {
        const db_user = this.getDBModel('User')
        const {resetToken, password} = param

        this.validate_password(password)

        const userMatchedByToken = await db_user.db.findOne({
            resetToken: resetToken,
            active: true
        })

        if (!userMatchedByToken) {
            console.error(`resetToken ${resetToken} did not match user`)
            throw 'token invalid'
        }

        const result = await db_user.update(
            {_id: userMatchedByToken._id},
            {
                $set: {
                    password: this.getPassword(password, userMatchedByToken.salt)
                },
                $unset: {
                    resetToken: 1
                }
            }
        )

        if (!result.nModified) {
            console.error(`resetToken ${resetToken} password update failed`)
            throw 'password update failed'
        }

        return 1
    }

    /*
     * return ela budget sum amount.
     *
     * param : user's elaBudget
     * */
    public getSumElaBudget(ela) {
        let total = 0
        _.each(ela, (item) => {
            total += item.amount
        })

        return total
    }

    /*
     * return user password
     * password is built with sha512 to (password + salt)
     *
     * */
    public getPassword(password, salt) {
        return utilCrypto.sha512(password + salt)
    }

    public validate_username(username) {
        if (!validate.valid_string(username, 6)) {
            throw 'invalid username'
        }
    }

    public validate_password(password) {
        if (!validate.valid_string(password, 8)) {
            throw 'invalid password'
        }
    }

    public validate_email(email) {
        if (!validate.email(email)) {
            throw 'invalid email'
        }
    }

    /**
     * Send an Email
     *
     * @param param {Object}
     * @param param.fromUserId {String}
     * @param param.toUserId {String}
     * @param param.subject {String}
     * @param param.message {String}
     */
    public async sendEmail(param) {
        const {fromUserId, toUserId, subject, message} = param

        // ensure fromUser is logged in
        if (this.currentUser._id.toString() !== fromUserId) {
            throw 'User mismatch - from user must = sender'
        }

        const db_user = this.getDBModel('User')

        const fromUser = await db_user.findById(fromUserId)
        const toUser = await db_user.findById(toUserId)

        const formattedSubject = subject || 'New Cyber Republic private message'

        const body = `
            New message from <a href="${process.env.SERVER_URL}/member/${fromUserId}">${fromUser.username}</a>
            <br/>
            <br/>
            ${message}
        `

        if (!fromUser) {
            throw 'From user not found'
        }

        if (!toUser) {
            throw 'From user not found'
        }

        // we assume users must have entered an email

        await mail.send({
            to: toUser.email,
            toName: `${toUser.profile.firstName} ${toUser.profile.lastName}`,
            subject: formattedSubject,
            body,
            replyTo: {
                name: `${fromUser.profile.firstName} ${fromUser.profile.lastName}`,
                email: fromUser.email
            }
        })

        return true
    }

    public async sendRegistrationCode(param) {
        const {email, code} = param
        await mail.send({
            to: email,
            toName: email,
            subject: 'Your Cyber Republic registration code',
            body: `Your code: ${code}`
        })
        return true
    }

    public async sendConfirmation(param) {
        const {email} = param

        await mail.send({
            to: email,
            toName: email,
            subject: 'Welcome to Cyber Republic',
            body: `
                Your registration is complete, your login is automatically linked to the CR forums.<br/>
                <br/>
                <a href="https://forum.cyberrepublic.org">Click here to join us on the forums</a>
            `
        })

        return true
    }

    private async linkCountryCommunity(user) {
        const db_community = this.getDBModel('Community')
        const communityService = this.getService(CommunityService)

        if (!user.profile || _.isEmpty(user.profile.country)) {
            return
        }

        // 1st check if the country already exists
        let countryCommunity = await db_community.findOne({
            type: constant.COMMUNITY_TYPE.COUNTRY,
            geolocation: user.profile.country
        })

        if (!countryCommunity) {
            // create the country then attach
            countryCommunity = await communityService.create({
                name: geo.geolocationMap[user.profile.country],
                type: constant.COMMUNITY_TYPE.COUNTRY,
                geolocation: user.profile.country,
                parentCommunityId: undefined
            })
        }

        // now we should always have the community to attach it
        await communityService.addMember({
            userId: user._id,
            communityId: countryCommunity._id
        })
    }

    public async checkEmail(param) {
        const db_user = this.getDBModel('User')

        const email = param.email.toLowerCase()

        this.validate_email(email)

        if (await db_user.findOne({email: email})) {
            return {isExist: true}
        }

        return {isExist: false}
    }

    public async getElaUrl() {
        try {
            const userId = _.get(this.currentUser, '_id')
            const db_user = this.getDBModel('User')
            const user = await db_user.findById({_id: userId})
            if (_.isEmpty(user)) {
                return {success: false}
            }
            const jwtClaims = {
                iss: process.env.APP_DID,
                userId: this.currentUser._id,
                callbackurl: `${process.env.API_URL}/api/user/did-callback-ela`,
                claims: {},
                website: {
                    domain: process.env.SERVER_URL,
                    logo: `${process.env.SERVER_URL}/assets/images/cr_ela_wallet.svg`
                }
            }
            const jwtToken = jwt.sign(jwtClaims, process.env.APP_PRIVATE_KEY, {
                expiresIn: '7d',
                algorithm: 'ES256'
            })
            const url = `elastos://credaccess/${jwtToken}`
            return {success: true, url}
        } catch (err) {
            logger.error(err)
            return {success: false}
        }
    }

    public async didCallbackEla(param: any) {
        try {
            const jwtToken = param.jwt
            const claims: any = jwt.decode(jwtToken)
            if (!claims || !claims.req) {
                return {
                    code: 400,
                    success: false,
                    message: 'Problems parsing jwt token.'
                }
            }

            if (claims && !claims.req) {
                return {
                    code: 400,
                    success: false,
                    message: 'The payload of jwt token is not correct.'
                }
            }

            const payload: any = jwt.decode(
                claims.req.slice('elastos://credaccess/'.length)
            )
            if (!payload || (payload && !payload.userId)) {
                return {
                    code: 400,
                    success: false,
                    message: 'Problems parsing jwt token of CR website.'
                }
            }

            const db_user = this.getDBModel('User')
            const user = await db_user.findById({_id: payload.userId})
            if (!user) {
                return {
                    code: 400,
                    success: false,
                    message: 'User ID does not exist.'
                }
            }

            const rs: any = await getDidPublicKey(claims.iss)
            if (!rs) {
                const did = {
                    message: `Can not get your did's public key.`
                }
                await db_user.update({_id: payload.userId}, {$set: {did}})
                return {
                    code: 400,
                    success: false,
                    message: `Can not get your did's public key.`
                }
            }

            // verify response data from ela wallet
            return jwt.verify(
                jwtToken,
                rs.publicKey,
                async (err: any, decoded: any) => {
                    if (err) {
                        const did = {message: 'Verify signatrue failed.'}
                        await db_user.update({_id: payload.userId}, {$set: {did}})
                        return {
                            code: 401,
                            success: false,
                            message: 'Verify signatrue failed.'
                        }
                    } else {
                        try {
                            const doc = await this.findUserByDid(decoded.iss)
                            if (doc && !doc._id.equals(payload.userId)) {
                                const did = {
                                    message: 'This DID had been used by other user.'
                                }
                                await db_user.update({_id: payload.userId}, {$set: {did}})
                                return {
                                    code: 400,
                                    success: false,
                                    message: 'This DID had been used by other user.'
                                }
                            }
                            const did = { id: decoded.iss }
                            await db_user.update({_id: payload.userId}, {$set: {did}})
                            return {code: 200, success: true, message: 'Ok'}
                        } catch (err) {
                            logger.error(err)
                            return {
                                code: 500,
                                success: false,
                                message: 'Something went wrong'
                            }
                        }
                    }
                }
            )
        } catch (err) {
            logger.error(err)
            return {
                code: 500,
                success: false,
                message: 'Something went wrong'
            }
        }
    }

    public async getDid() {
        const userId = this.currentUser._id
        const db_user = this.getDBModel('User')
        const user = await db_user.findById({_id: userId})
        if (user) {
            const id = _.get(user, 'did.id')
            if (id) {
                return {success: true, did: user.did}
            }
            const message = _.get(user, 'did.message')
            if (message) {
                await db_user.update({_id: userId}, {$unset: {did: true}})
                return {success: false, message}
            }
        } else {
            return {success: false}
        }
    }

    public async loginElaUrl() {
        try {
            const nonce = uuid.v4()
            const jwtClaims = {
                iss: process.env.APP_DID,
                callbackurl: `${process.env.API_URL}/api/user/login-callback-ela`,
                nonce,
                claims: {},
                website: {
                    domain: process.env.SERVER_URL,
                    logo: `${process.env.SERVER_URL}/assets/images/cr_ela_wallet.svg`
                }
            }
            const jwtToken = jwt.sign(jwtClaims, process.env.APP_PRIVATE_KEY, {
                expiresIn: '7d',
                algorithm: 'ES256'
            })
            const url = `elastos://credaccess/${jwtToken}`

            const db_did = this.getDBModel('Did')
            await db_did.save({number: nonce})

            return {success: true, url}
        } catch (err) {
            logger.error(err)
            return {success: false}
        }
    }

    public async loginCallbackEla(param: any) {
        try {
            const jwtToken = param.jwt
            const claims: any = jwt.decode(jwtToken)
            if (!claims) {
                return {
                    code: 400,
                    success: false,
                    message: 'Problems parsing jwt token.'
                }
            }

            if (claims && !claims.req) {
                return {
                    code: 400,
                    success: false,
                    message: 'The payload of jwt token is not correct.'
                }
            }

            const payload: any = jwt.decode(
                claims.req.slice('elastos://credaccess/'.length)
            )
            if (!payload || (payload && !payload.nonce)) {
                return {
                    code: 400,
                    success: false,
                    message: 'Problems parsing jwt token of CR website.'
                }
            }

            const db_did = this.getDBModel('Did')
            const rs: any = await getDidPublicKey(claims.iss)
            if (!rs) {
                await db_did.update(
                    {nonce: payload.nonce},
                    {
                        $set: {
                            message: `Can not get your did's public key.`,
                            success: false
                        }
                    }
                )
                return {
                    code: 400,
                    success: false,
                    message: `Can not get your did's public key.`
                }
            }

            // verify response data from ela wallet
            return jwt.verify(
                jwtToken,
                rs.publicKey,
                async (err: any, decoded: any) => {
                    if (err) {
                        await db_did.update(
                            {nonce: payload.nonce},
                            {
                                $set: {
                                    message: `Verify signatrue failed.`,
                                    success: false
                                }
                            }
                        )
                        return {
                            code: 401,
                            success: false,
                            message: 'Verify signatrue failed.'
                        }
                    } else {
                        try {
                            const didDoc = await db_did.findOne({
                                number: payload.nonce,
                                did: decoded.iss
                            })
                            if (!_.isEmpty(didDoc)) {
                                return {code: 200, success: true, message: 'Ok'}
                            }

                            await db_did.update(
                                {number: payload.nonce},
                                {
                                    $set: {
                                        did: decoded.iss,
                                        success: true,
                                        message: 'Ok'
                                    }
                                }
                            )
                            return {code: 200, success: true, message: 'Ok'}
                        } catch (err) {
                            logger.error(err)
                            return {
                                code: 500,
                                success: false,
                                message: 'Something went wrong'
                            }
                        }
                    }
                }
            )
        } catch (err) {
            logger.error(err)
            return {
                code: 500,
                success: false,
                message: 'Something went wrong'
            }
        }
    }

    public async checkElaAuth(param: any) {
        try {
            if (!param.req) {
                return {success: false}
            }
            const jwtToken = param.req.slice('elastos://credaccess/'.length)
            if (!jwtToken) {
                return {success: false}
            }
            return jwt.verify(
                jwtToken,
                process.env.APP_PUBLIC_KEY,
                async (err: any, decoded: any) => {
                    if (err) {
                        return {success: false}
                    }
                    try {
                        const db_did = this.getDBModel('Did')
                        const doc = await db_did.findOne({number: decoded.nonce})
                        if (doc) {
                            if (doc.did) {
                                await db_did.getDBInstance().remove({number: decoded.nonce})
                                return {did: doc.did, success: true}
                            }
                            if (doc.success === false) {
                                await db_did.update(
                                    {number: decoded.nonce},
                                    {$unset: {success: true, message: true}}
                                )
                                return {message: doc.message, success: false}
                            }
                        } else {
                            return {success: false}
                        }
                    } catch (err) {
                        return {success: false}
                    }
                }
            )
        } catch (err) {
            return {success: false}
        }
    }

    public async eachJob() {
        // find user exist did
        const db_user = this.getDBModel('User')
        const result = await db_user.find({'did.id': {$exists: true}, role: {$nin: [constant.USER_ROLE.COUNCIL, constant.USER_ROLE.SECRETARY]}})

        const asyncForEach = async (array, callback) => {
            for (let index = 0; index < array.length; index++) {
                await callback(array[index], index, array);
            }
        }

        await asyncForEach(result, async (o: any) => {
            const didName = await getDidName(o.did.id)
            if (didName) {
                await db_user.getDBInstance().update({_id: o._id}, {$set: {'did.didName': didName}})
            }
        })
    }

    public async cronJob() {
        if (tm) {
            return false
        }
        tm = setInterval(async () => {
            console.log('---------------- start user cronJob -------------')
            this.eachJob()
        }, 1000 * 60 * 10)
    }
}
