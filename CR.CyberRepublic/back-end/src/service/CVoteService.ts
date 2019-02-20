import Base from './Base';
import { Document } from 'mongoose';
import * as _ from 'lodash';
import { constant } from '../constant';
import { permissions } from '../utility';
import * as moment from 'moment';

let tm = null;

const restrictedFields = {
    update: [
        '_id',
        'createdBy',
        'createdAt'
    ]
}

export default class extends Base {

    public async create(param): Promise<Document>{
        const db_cvote = this.getDBModel('CVote');
        const db_user = this.getDBModel('User');

        const {
            title, type, content, published, proposedBy, motionId, isConflict, notes,
        } = param;

        const councilMembers = await db_user.find({role: constant.USER_ROLE.COUNCIL});
        const voteResult = []
        const vote_map = {}
        const reason_map = {}
        _.each(councilMembers, user => {
          const fullName = `${user.profile.firstName} ${user.profile.lastName}`
          voteResult.push({ votedBy: user._id })
          vote_map[fullName] = -1
          reason_map[fullName] = ''
        })

        const doc: any = {
            title,
            type,
            published,
            content,
            proposedBy,
            motionId,
            isConflict,
            notes,
            voteResult,
            createdBy : this.currentUser._id
        };

        const vid = await this.getNewVid();
        doc.vid = vid;
        doc.status = published ? constant.CVOTE_STATUS.PROPOSED : constant.CVOTE_STATUS.DRAFT;

        const cvote = await db_cvote.save(doc);

        this.sendEmailNotification(cvote, 'create');

        return cvote;
    }

    /**
     * List proposals, only an admin may request and view private records
     *
     * We expect the front-end to always call with {published: true}
     *
     * TODO: what's the rest way of encoding multiple values for a field?
     *
     * Instead of magic params, we should have just different endpoints I think,
     * this method should be as dumb as possible
     *
     * @param query
     * @returns {Promise<"mongoose".Document>}
     */
    public async list(param): Promise<Document>{

        const db_cvote = this.getDBModel('CVote');
        const db_user = this.getDBModel('User');
        const query: any = {};

        // if we are not querying only published records, we need to be an admin
        // TODO: write a test for this
        if (!param.published) {
            if (!this.isLoggedIn() || !this.canManageProposal()) {
                throw 'cvoteservice.list - unpublished proposals only visible to council/secretary';
            } else {
                // only owner can list his own proposal
                query.$or = [
                    { createdBy: _.get(this.currentUser, '_id'), published: false },
                    { published: true },
                ]
            }
        } else {
            query.published = param.published
        }

        // we should map over allowed filters manually
        // console.log(query)

        const list = await db_cvote.list(query, {
            createdAt: -1
        }, 100);

        for(const item of list){
            if(item.createdBy){
                const u = await db_user.findOne({_id : item.createdBy});
                item.createdBy = u.username;
            }

        }

        return list;
    }

    /**
     *
     * @param param
     * @returns {Promise<"mongoose".Document>}
     */
    public async update(param): Promise<Document>{
        const db_cvote = this.getDBModel('CVote');
        const { _id, published, notes } = param

        if(!this.currentUser || !this.currentUser._id){
            throw 'cvoteservice.update - invalid current user';
        }

        if (!this.canManageProposal()) {
            throw 'cvoteservice.update - not council'
        }

        const cur = await db_cvote.findOne({ _id });
        if(!cur) {
            throw 'cvoteservice.update - invalid proposal id';
        }

        const doc: any = {}

        if (published && cur.status === constant.CVOTE_STATUS.DRAFT) doc.status = constant.CVOTE_STATUS.PROPOSED

        // always allow secretary to edit notes
        if (notes) doc.notes = notes

        await db_cvote.update({ _id }, doc);

        this.sendEmailNotification({ _id }, 'update');

        return await this.getById(_id);
    }

    public async finishById(id): Promise<any>{
        const db_cvote = this.getDBModel('CVote');
        const cur = await db_cvote.findOne({_id : id});
        if(!cur){
            throw 'invalid proposal id';
        }
        if (!this.canManageProposal()) {
            throw 'cvoteservice.finishById - not council'
        }
        if(_.includes([constant.CVOTE_STATUS.FINAL], cur.status)){
            throw 'proposal already completed.';
        }

        const rs = await db_cvote.update({_id : id}, {
            $set : {
                status : constant.CVOTE_STATUS.FINAL
            }
        })

        return rs;
    }

    public async getById(id): Promise<any>{
      const db_cvote = this.getDBModel('CVote')
      const rs = await db_cvote.getDBInstance().findOne({_id : id})
        .populate('voteResult.votedBy', constant.DB_SELECTED_FIELDS.USER.NAME_AVATAR)
      return rs;
    }

    private param_metadata(meta: string){
        const rs = {};
        if(meta){
            const list = meta.split(',');

            _.each(list, (str)=>{
                const tmp = str.split('|');
                if(tmp.length === 2){
                    rs[tmp[0]] = tmp[1];
                }
            });
        }
        return rs;
    }

    public async getNewVid(){
        const db_cvote = this.getDBModel('CVote');
        const n = await db_cvote.count({});
        return n+1;
    }

    public getNewStatus(vote_map, data){
        let rs = constant.CVOTE_STATUS.PROPOSED;
        let ns = 0;
        let nf = 0;
        _.each(vote_map, (v)=>{
            if(v === 'support'){
                ns++;
            }
            else if(v === 'reject'){
                nf++;
            }
        });

        if(data && this.isExpired(data)){
            return constant.CVOTE_STATUS.DEFERRED;
        }

        // TODO: later there will be more than 3 council members
        if(nf > 1){
            rs = constant.CVOTE_STATUS.REJECT;
        }
        if(ns > 1){
            rs = constant.CVOTE_STATUS.ACTIVE;
        }


        return rs;
    }

    public isExpired(data): Boolean{
        const ct = moment(data.createdAt).valueOf();
        if(Date.now() - ct > constant.CVOTE_EXPIRATION){
            return true;
        }
        return false;
    }

    public async vote(param): Promise<Document>{
        const db_cvote = this.getDBModel('CVote');
        const { _id, value, reason } = param
        const cur = await db_cvote.findOne({ _id });
        if(!cur) {
            throw 'invalid proposal id';
        }

        const rs = await db_cvote.updateOne(
          {
            _id,
            'voteResult.votedBy': _.get(this.currentUser, '_id')
          }, {
            $set : {
              'voteResult.$.value': value,
              'voteResult.$.reason': reason,
            }
          }
        )

        return rs;
    }

    public async updateNote(param): Promise<Document>{
        const db_cvote = this.getDBModel('CVote');

        const cur = await db_cvote.findOne({_id : param._id});
        if(!cur){
            throw 'invalid proposal id';
        }
        if (!this.canManageProposal()) {
            throw 'cvoteservice.updateNote - not council'
        }
        if(this.currentUser.role !== constant.USER_ROLE.SECRETARY){
            throw 'only secretary could update notes';
        }

        const rs = await db_cvote.update({_id : param._id}, {
            $set : {
                notes : param.notes || ''
            }
        })

        return rs;
    }

    public async sendEmailNotification(data, updateOrCreate){
        // const list = [
        //     'kevinzhang@elastos.org',
        //     'suyipeng@elastos.org',
        //     'fay@elastos.org',
        //     'zhufeng@elastos.org'
        // ];

        // _.each(['liyangwood@aliyun.com'], async (address)=>{
        //     await mail.send({
        //         to : address,
        //         toName : 'Jacky.li',
        //         subject : 'CVote - Notification',
        //         body : `
        //             Proposal ${updateOrCreate}
        //             <br />
        //             please alick this link to get details.
        //             <a href="${process.env.SERVER_URL}/cvote/edit/${data._id}">${process.env.SERVER_URL}/cvote/edit/${data._id}</a>
        //         `
        //     });
        // });
    }

    private async eachJob() {
        const db_cvote = this.getDBModel('CVote');
        const list = await db_cvote.find({
            status: constant.CVOTE_STATUS.PROPOSED
        });
        const ids = [];

        _.each(list, (item)=>{
            if(this.isExpired(item)){
                ids.push(item._id);
            }
        });

        await db_cvote.update({_id : {
            $in : ids
        }}, {
            status : constant.CVOTE_STATUS.DEFERRED
        });
    }

    public cronjob(){
        if(tm){
            return false;
        }
        tm = setInterval(()=>{
            console.log('---------------- start cvote cronjob -------------');
            this.eachJob();
        }, 1000*60);
    }

    private canManageProposal() {
        const userRole = _.get(this.currentUser, 'role')
        return permissions.isCouncil(userRole) || permissions.isSecretary(userRole)
    }
}
