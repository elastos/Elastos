import React from 'react'
import _ from 'lodash'
import moment from 'moment/moment'
import BaseComponent from '@/model/BaseComponent'
import {
  Table, Row, Col, Button,
} from 'antd'
import I18N from '@/I18N'
import { LANGUAGES } from '@/config/constant'
import VoteStats from '../stats/Component'
import { CVOTE_RESULT_TEXT } from '@/constant'

import './style.scss'

export default class extends BaseComponent {
  constructor(p) {
    super(p);

    this.state.list = null;
    this.state.loading = true;
  }

  ord_render() {
    const { language } = this.props
    const map = {
      1: I18N.get('council.voting.type.newMotion'),
      2: I18N.get('council.voting.type.motionAgainst'),
      3: I18N.get('council.voting.type.anythingElse'),
    };

    const columns = [
      {
        title: I18N.get('council.voting.number'),
        dataIndex: 'vid',
        render: (vid, item, index) => (
          <a className="tableLink" onClick={this.toDetail.bind(this, item._id)}>
#
            {vid}
          </a>
        ),
      },
      {
        title: I18N.get('council.voting.title'),
        dataIndex: 'title',
        width: '30%',
        render: (title, item) => (
          <a onClick={this.toDetail.bind(this, item._id)} className="tableLink">
            { language === LANGUAGES.chinese ? (item.title_zh ? item.title_zh : title) : title }
          </a>
        ),
      },
      {
        title: I18N.get('council.voting.type'),
        dataIndex: 'type',
        render: (type, item) => map[type],
      },
      {
        title: I18N.get('council.voting.author'),
        dataIndex: 'proposedBy',
      },
      {
        title: I18N.get('council.voting.voteByCouncil'),
        render: (id, item) => this.voteDataByUser(item),
      },
      {
        title: I18N.get('council.voting.status'),
        render: (id, item) => item.status || '',
      },
      {
        title: I18N.get('council.voting.createdAt'),
        dataIndex: 'createdAt',
        render: createdAt => moment(createdAt).format('MMM D, YYYY'),
      },
    ]

    if (this.props.isCouncil) {
      columns.splice(1, 0, {
        dataIndex: 'published',
        render: (published, item, index) => (published ? <i className="fas fa-eye" /> : <i className="far fa-eye-slash" />),
      })
    }

    const createBtn = this.props.isCouncil && (
      <Button style={{ width: '100%' }} onClick={this.toCreate} size="large" type="ebp" htmlType="submit" className="d_btn">
          Create New Proposal
      </Button>
    )
    return (
      <div className="p-cvote-list ebp-wrap">
        <div className="d_box">
          <Row>
            <Col span={8}>
              <h3 style={{ textAlign: 'left' }} className="komu-a cr-title-with-icon">
                {I18N.get('council.voting.proposalList')}
              </h3>
            </Col>
            <Col span={8} offset={8}>
              {createBtn}
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
    this.props.history.push(`/cvote/${id}`);
  }

  toCreate() {
    this.props.history.push('/cvote/create');
  }

  async componentDidMount() {
    this.ord_loading(true);

    const list = await this.props.listData({}, this.props.isCouncil);

    this.setState({ list });

    this.ord_loading();
  }

  voteDataByUser = (data) => {
    const voteMap = data.vote_map;
    if (!data.vote_map) {
      // fix error in finding index of undefined
      return ''
    }
    const voteArr = _.map(voteMap, value => CVOTE_RESULT_TEXT[value.toLowerCase()])
    const supportNum = _.countBy(voteArr).Yes || 0
    const proposalAgreed = supportNum > 50
    const percentage = (supportNum / voteArr.length).toString() && `${(supportNum / voteArr.length).toFixed(1).toString()}%`
    return <VoteStats percentage={percentage} values={voteArr} yes={proposalAgreed} />
  }
}
