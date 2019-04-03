import Base from './Base'
import {Document} from 'mongoose'
import * as _ from 'lodash'
import {constant} from '../constant'
import {validate} from '../utility'

export default class extends Base {

    /**
     * Create a Community, you can nest them under other communities,
     * each community can have a leader
     *
     * @param param
     * @returns {Promise<"mongoose".Document>}
     */
    public async create(param): Promise<Document> {
        const db_community = this.getDBModel('Community')

        // validate
        this.validate_name(param.name)
        const {name, parentCommunityId, geolocation, type, leaderIds} = param

        const doc = {
            name,
            parentCommunityId,
            geolocation,
            type,
            leaderIds: this.param_leaderIds(leaderIds),
            createdBy: this.currentUser ? this.currentUser._id : undefined
        }

        return await db_community.save(doc)
    }

    /**
     * Only the admin/leader can change certain fields like the status to SENT
     *
     * @param param
     * @returns {Promise<boolean>}
     */
    public async update(param): Promise<boolean> {
        const db_community = this.getDBModel('Community')

        // validate
        this.validate_name(param.name)
        const {_id, name, parentCommunityId, geolocation, type, leaderIds} = param

        const doc = {
            $set : {
                name,
                parentCommunityId,
                geolocation,
                type,
                leaderIds : this.param_leaderIds(leaderIds)
            }
        }

        return await db_community.update({_id : _id}, doc)
    }

    /**
     * Get list Community
     *
     * @param param
     * @returns {Promise<"mongoose".Document>}
     */
    public async index(param): Promise<[Document]> {
        const db_community = this.getDBModel('Community')
        const db_user_community = this.getDBModel('User_Community')

        const query: any = param.query || {}

        if (param.communityHasUser) {

            if (param.communityHasUser === 'undefined') {
                param.communityHasUser = undefined
            }

            const userCommunities = await db_user_community.find({
                userId: param.communityHasUser
            })

            query._id = {$in: _.map(userCommunities, 'communityId')}
        }

        return await db_community.getDBInstance().find(query).sort({name: 1})
    }

    /**
     * Get Community
     *
     * @param param
     * @returns {Promise<"mongoose".Document>}
     */
    public async get(communityId): Promise<Document> {
        const db_community = this.getDBModel('Community')
        return await db_community.findById(communityId)
    }

    /**
     * Get list member
     *
     * @param param
     * @returns {Promise<"mongoose".Document>}
     */
    public async listMember(param): Promise<Document[]>{
        const {communityId} = param
        const db_user_community = this.getDBModel('User_Community')

        return await db_user_community.find({communityId})
    }

    /**
     * Get list community
     *
     * @param param
     * @returns {Promise<"mongoose".Document>}
     */
    public async listCommunity(param): Promise<Document[]>{
        const {userId} = param
        const db_user_community = this.getDBModel('User_Community')

        return await db_user_community.find({userId})
    }

    public async findCommunities(query): Promise<Document[]>{
        const db_community = this.getDBModel('Community')
        return await db_community.find({
            '_id' : {
                $in : query.communityIds
            }
        })
    }

    /**
     * Add member to Community
     *
     * @param param
     * @param.userId
     * @param.communityId
     *
     * @returns {Promise<boolean>}
     */
    public async addMember(param): Promise<boolean> {
        const {userId, communityId} = param

        const db_user_community = this.getDBModel('User_Community')
        const tmp = await db_user_community.findOne({
            userId: userId,
            communityId: communityId
        })

        if (tmp) {
            return
        }

        await db_user_community.save({
            userId,
            communityId
        })

        return true
    }

    /**
     * Remove member out Community
     *
     * @param param
     * @returns {Promise<boolean>}
     */
    public async removeMember(param): Promise<boolean> {
        const {userId, communityId} = param

        const db_user_community = this.getDBModel('User_Community')
        const tmp = await db_user_community.findOne({
            userId,
            communityId
        })
        if (!tmp) {
            throw 'user is not exist'
        }

        await db_user_community.findOneAndDelete({
            userId: userId,
            communityId: communityId
        })

        return true
    }

    /**
     * Remove Community
     *
     * @param param
     * @returns {Promise<boolean>}
     */
     public async removeCommunity(communityId): Promise<boolean> {
        const db_community = this.getDBModel('Community')
        await db_community.findByIdAndDelete(communityId)

        return true
    }

    public validate_name(name){
        if(!validate.valid_string(name, 2)){
            throw 'invalid community name'
        }
    }

    public param_leaderIds(leaderIds: string){
        let rs = []
        if(leaderIds){
            rs = leaderIds.split(',')
        }
        return rs
    }
}
