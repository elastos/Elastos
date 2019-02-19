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

// style
import { Container, List, Item, ItemUndecided } from './style'

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

    if (this.props.canCreate) {
      columns.splice(1, 0, {
        dataIndex: 'published',
        render: (published, item, index) => (published ? <i className="fas fa-eye" /> : <i className="far fa-eye-slash" />),
      })
    }

    const statusIndicator = (
      <List>
        <Item yes />
        <span>{I18N.get('council.voting.type.support')}</span>
        <Item no />
        <span>{I18N.get('council.voting.type.reject')}</span>
        <Item abstained />
        <span>{I18N.get('council.voting.type.abstention')}</span>
        <ItemUndecided undecided />
        <span>{I18N.get('council.voting.type.undecided')}</span>
      </List>
    )

    const createBtn = this.props.canCreate && (
      <Col lg={8} md={12} sm={24} xs={24} style={{ textAlign: 'right' }}>
        <Button onClick={this.toCreate} className="cr-btn cr-btn-primary">
            Add a Proposal
        </Button>
      </Col>
    )
    return (
      <Container>
        <Row type="flex" align="middle" justify="end">
          {createBtn}
        </Row>
        <Row type="flex" align="middle" justify="space-between">
          <Col lg={8} md={8} sm={12} xs={24}>
            <h3 style={{ textAlign: 'left', paddingBottom: 0 }} className="komu-a cr-title-with-icon">
              {I18N.get('council.voting.proposalList')}
            </h3>
          </Col>
          <Col lg={8} md={8} sm={12} xs={24}>
            {statusIndicator}
          </Col>
        </Row>
        <Table
          columns={columns}
          loading={this.state.loading}
          dataSource={this.state.list}
          rowKey={record => record._id}
        />
      </Container>

    )
  }

  toDetail(id) {
    this.props.history.push(`/cvote/${id}`);
  }

  toCreate = () => {
    this.props.history.push('/cvote/create');
  }

  async componentDidMount() {
    this.ord_loading(true);
    try {
      const list = await this.props.listData({}, this.props.canCreate);
      this.setState({ list });
    } catch (error) {
      // do sth
    }

    this.ord_loading(false);
  }

  voteDataByUser = (data) => {
    const voteMap = data.vote_map;
    if (!data.vote_map) {
      // fix error in finding index of undefined
      return ''
    }
    const voteArr = _.map(voteMap, value => ((value === '-1' || _.isUndefined(value)) ? CVOTE_RESULT_TEXT.undefined : CVOTE_RESULT_TEXT[value.toLowerCase()]))
    const supportNum = _.countBy(voteArr).Yes || 0
    const percentage = supportNum * 100 / voteArr.length
    const proposalAgreed = percentage > 50
    const percentageStr = percentage.toString() && `${percentage.toFixed(1).toString()}%`
    return <VoteStats percentage={percentageStr} values={voteArr} yes={proposalAgreed} />
  }
}
