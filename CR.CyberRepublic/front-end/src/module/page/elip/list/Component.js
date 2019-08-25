import React from 'react'
import _ from 'lodash'
import moment from 'moment/moment'
import BaseComponent from '@/model/BaseComponent'
import { Table, Row, Col, Button } from 'antd'
import I18N from '@/I18N'
import { ELIP_FILTER } from '@/constant'
import { Container, StyledButton, StyledSearch, Filter } from './style'
import { logger } from '@/util'

export default class extends BaseComponent {
  constructor(props) {
    super(props)
    this.state = {
      list: [],
      search: '',
      filter: 'ALL',
      loading: true
    }
    this.debouncedRefetch = _.debounce(this.refetch.bind(this), 300)
  }

  async componentDidMount() {
    this.refetch()
  }

  getQuery = () => {
    const query = {}
    query.filter = this.state.filter
    const searchStr = this.state.search
    if (searchStr) {
      query.search = searchStr
    }

    return query
  }

  refetch = async () => {
    this.ord_loading(true)
    const { listData } = this.props
    const param = this.getQuery()
    try {
      const list = await listData(param)
      this.setState({ list })
    } catch (error) {
      logger.error(error)
    }
    this.ord_loading(false)
  }

  toDetailPage = (id) => {
    this.props.history.push(`/elips/${id}`)
  }

  addElip = () => {
    this.props.history.push('/elips/new')
  }

  searchChangedHandler = (search) => {
    this.setState({ search }, this.debouncedRefetch)
  }

  setFilter = (filter) => {
    this.setState({ filter }, this.refetch)
  }

  ord_render() {
    const { isSecretary, isLogin } = this.props
    const columns = [
      {
        title: I18N.get('elip.fields.number'),
        dataIndex: 'vid',
        render: (vid, item, index) => (
          <a
            className="tableLink"
            onClick={this.toDetailPage.bind(this, item._id)}
          >
            {`#${vid}`}
          </a>
        )
      },
      {
        title: I18N.get('elip.fields.title'),
        dataIndex: 'title',
        width: '35%',
        render: (title, item) => (
          <a
            onClick={this.toDetailPage.bind(this, item._id)}
            className="tableLink"
          >
            {title}
          </a>
        )
      },
      {
        title: I18N.get('elip.fields.author'),
        dataIndex: 'createdBy.username'
      },
      {
        title: I18N.get('elip.fields.status'),
        render: (status, item) => I18N.get(`elip.status.${item.status}`) || ''
      },
      {
        title: I18N.get('elip.fields.createdAt'),
        dataIndex: 'createdAt',
        render: createdAt => moment(createdAt).format('MMM D, YYYY')
      }
    ]

    const createBtn = isLogin && (
      <Row type="flex" align="middle" justify="end">
        <Col lg={8} md={12} sm={24} xs={24} style={{ textAlign: 'right' }}>
          <StyledButton
            onClick={this.addElip}
            className="cr-btn cr-btn-primary"
          >
            {I18N.get('elip.button.add')}
          </StyledButton>
        </Col>
      </Row>
    )

    const title = (
      <Col lg={8} md={8} sm={12} xs={24}>
        <h2
          style={{ textAlign: 'left', paddingBottom: 0 }}
          className="komu-a cr-title-with-icon"
        >
          {I18N.get('elip.header')}
        </h2>
      </Col>
    )

    const searchInput = (
      <Col lg={8} md={8} sm={12} xs={24}>
        <StyledSearch
          defaultValue={this.state.search}
          onSearch={this.searchChangedHandler}
          placeholder={I18N.get('developer.search.search.placeholder')}
        />
      </Col>
    )

    const filterBtnGroup = (
      <Button.Group className="filter-group">
        <StyledButton
          className={(this.state.filter === ELIP_FILTER.ALL && 'selected') || ''}
          onClick={() => this.setFilter(ELIP_FILTER.ALL)}
        >
          {I18N.get('elip.filter.ALL')}
        </StyledButton>
        <StyledButton
          className={(this.state.filter === ELIP_FILTER.APPROVED && 'selected') || ''}
          onClick={() => this.setFilter(ELIP_FILTER.APPROVED)}
        >
          {I18N.get('elip.filter.APPROVED')}
        </StyledButton>
        {isLogin && (
          <StyledButton
            className={(this.state.filter === ELIP_FILTER.SUBMITTED_BY_ME && 'selected') || ''}
            onClick={() => this.setFilter(ELIP_FILTER.SUBMITTED_BY_ME)}
          >
            {I18N.get('elip.filter.SUBMITTED_BY_ME')}
          </StyledButton>
        )}
        {isSecretary && (
          <StyledButton
            className={(this.state.filter === ELIP_FILTER.WAIT_FOR_REVIEW && 'selected') || ''}
            onClick={() => this.setFilter(ELIP_FILTER.WAIT_FOR_REVIEW)}
          >
            {I18N.get('elip.filter.WAIT_FOR_REVIEW')}
          </StyledButton>
        )}
      </Button.Group>
    )

    const filterBtns = (
      <Col lg={8} md={8} sm={12} xs={24}>
        <Filter>
          <span>{`${I18N.get('elip.show')}: `}</span>
          {filterBtnGroup}
        </Filter>
      </Col>
    )

    const { list, loading } = this.state
    return (
      <Container>
        {createBtn}
        <Row
          type="flex"
          align="middle"
          justify="space-between"
          style={{ marginTop: 20 }}
        >
          {title}
          {searchInput}
          {isLogin && filterBtns}
        </Row>
        <Table
          columns={columns}
          loading={loading}
          dataSource={list}
          rowKey={record => record.vid}
        />
      </Container>
    )
  }
}
