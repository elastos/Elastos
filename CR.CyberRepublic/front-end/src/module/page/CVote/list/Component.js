import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import { Table, Tooltip, Row, Col, Button} from 'antd'
import I18N from '@/I18N'
import config from '@/config'
import { LANGUAGES } from '@/config/constant'

import './style.scss'
import moment from 'moment/moment'

export default class extends BaseComponent {

    constructor(p){
        super(p);

        this.state.list = null;
        this.state.loading = true;
    }

    ord_render() {
        const { language } = this.props
        const map = {
            '1' : I18N.get('council.voting.type.newMotion'),
            '2' : I18N.get('council.voting.type.motionAgainst'),
            '3' : I18N.get('council.voting.type.anythingElse')
        };

        const columns = [
            {
                title: I18N.get('council.voting.number'),
                dataIndex: 'vid',
                render: (vid, item, index) => {
                    return (<a className="tableLink" onClick={this.toDetail.bind(this, item._id)}>#{vid}</a>)
                }
            },
            {
                title : I18N.get('council.voting.title'),
                dataIndex : 'title',
                width: '30%',
                render: (title, item) => {
                    return (<a onClick={this.toDetail.bind(this, item._id)} className="tableLink">
                        { language === LANGUAGES.chinese ? ( item.title_zh ? item.title_zh : title) : title }</a>
                    )
                }
            },
            {
                title : I18N.get('council.voting.type'),
                dataIndex : 'type',
                render: (type, item) => {
                    return map[type];
                }
            },
            {
                title : I18N.get('council.voting.author'),
                dataIndex : 'proposedBy'
            },
            {
                title : `${I18N.get('council.voting.voteBy')} Kevin Zhang`,
                render: (id, item)=>{
                    return this.voteDataByUser('Kevin Zhang', item);
                }
            },
            {
                title : `${I18N.get('council.voting.voteBy')} Fay Li`,
                render: (id, item)=>{
                    return this.voteDataByUser('Fay Li', item);
                }
            },
            {
                title : `${I18N.get('council.voting.voteBy')} Yipeng Su`,
                render: (id, item)=>{
                    return this.voteDataByUser('Yipeng Su', item);
                }
            },
            {
                title : I18N.get('council.voting.status'),
                render: (id, item)=>{
                    return item.status || ''
                }
            },
            {
                title : I18N.get('council.voting.createdAt'),
                dataIndex : 'createdAt',
                render: (createdAt) => moment(createdAt).format('MMM D, YYYY'),
            }
        ]

        if (this.props.isCouncil) {

            columns.splice(1, 0, {
                dataIndex: 'published',
                render: (published, item, index) => {
                    return published ? <i className="fas fa-eye"></i> : <i className="far fa-eye-slash"></i>
                }
            })

        }


        return (
            <div className="p-cvote-list ebp-wrap">
                <div className="d_box">
                    <Row>
                        <Col span={8}>
                            <h3 style={{textAlign:'left'}}>
                                {I18N.get('council.voting.proposalList')}
                            </h3>
                        </Col>
                        <Col span={8} offset={8}>
                            {this.props.isCouncil &&
                            <Button style={{width:'100%'}} onClick={this.toCreate.bind(this)} size="large" type="ebp" htmlType="submit" className="d_btn">
                                Create New Proposal
                            </Button>}
                        </Col>
                    </Row>
                    <Table
                        columns={columns}
                        loading={this.state.loading}
                        dataSource={this.state.list}
                        rowKey={record => record._id}
                    />
                </div>
            </div>

        )
    }

    toDetail(id) {
        this.props.history.push(`/cvote/edit/${id}`);
    }
    toCreate(){
        this.props.history.push('/cvote/create');
    }

    async componentDidMount(){

        this.ord_loading(true);

        const list = await this.props.listData({}, this.props.isCouncil);

        this.setState({list});

        this.ord_loading();

    }

    ord_checkLogin(isLogin){

    }

    voteDataByUser(u, data){
        const map = data.vote_map;
        if (!data.vote_map) {
            // fix error in finding index of undefined
            return ''
        }
        if(!map[u]){
            return '';
        }
        const temp = map[u];
        if(temp === 'support') return 'Y';
        if(temp === 'reject') return 'N';
        return '';
    }
}
 