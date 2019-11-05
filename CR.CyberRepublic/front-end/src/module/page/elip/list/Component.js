import React from 'react'
import _ from 'lodash'
import moment from 'moment/moment'
import BaseComponent from '@/model/BaseComponent'
import { Table, Row, Col, Button } from 'antd'
import { CSVLink } from 'react-csv'
import I18N from '@/I18N'
import { ELIP_FILTER } from '@/constant'
import { Container, StyledButton, StyledSearch, Filter, FilterLabel } from './style'
import { logger } from '@/util'
import userUtil from '@/util/user'

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
    const { isLogin, history } = this.props
    if (!isLogin) {
      sessionStorage.setItem('loginRedirect', '/elips/new')
      history.push('/login')
    } else {
      history.push('/elips/new')
    }
  }

  searchChangedHandler = (search) => {
    this.setState({ search: search.trim() }, this.debouncedRefetch)
  }

  setFilter = (filter) => {
    this.setState({ filter }, this.refetch)
  }

  renderAuthor = (createdBy) => {
    return userUtil.formatUsername(createdBy)
  }

  renderStatus = (status) => {
    return I18N.get(`elip.status.${status}`) || ''
  }

  renderCreatedAt = (createdAt) => {
    const lang = localStorage.getItem('lang') || 'en'
    return lang === 'en' ? moment(createdAt).format('MMM D, YYYY') : moment(createdAt).format('YYYY-MM-DD')
  }

  ord_render() {
    const { isSecretary, isLogin } = this.props
    const columns = [
      {
        title: I18N.get('elip.fields.number'),
        dataIndex: 'vid',
        render: (vid, item) => (
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
        dataIndex: 'createdBy',
        render: createdBy => this.renderAuthor(createdBy)
      },
      {
        title: I18N.get('elip.fields.status'),
        dataIndex: 'status',
        render: status => this.renderStatus(status)
      },
      {
        title: I18N.get('elip.fields.createdAt'),
        dataIndex: 'createdAt',
        render: createdAt => this.renderCreatedAt(createdAt)
      }
    ]

    const createBtn = (
      <Col lg={8} md={8} sm={12} xs={24} style={{ textAlign: 'right' }}>
        <StyledButton
          onClick={this.addElip}
          className="cr-btn cr-btn-primary"
        >
          {I18N.get('elip.button.add')}
        </StyledButton>
      </Col>
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
      <Col lg={8} md={12} sm={12} xs={24}>
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
          className={(this.state.filter === ELIP_FILTER.DRAFT && 'selected') || ''}
          onClick={() => this.setFilter(ELIP_FILTER.DRAFT)}
        >
          {I18N.get('elip.filter.DRAFT')}
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
      <Filter>
        <FilterLabel>{`${I18N.get('elip.show')}: `}</FilterLabel>
        {filterBtnGroup}
      </Filter>
    )

    const { list, loading } = this.state
    let dataCSV = []
    if (isSecretary) {
      const itemsCSV = _.map(list, v => [
        v.vid,
        v.title,
        this.renderAuthor(v.createdBy),
        this.renderStatus(v.status),
        _.replace(this.renderCreatedAt(v.createdAt), ',', ' ')
      ])
      dataCSV = _.concat(
        [
          [
            I18N.get('elip.fields.number'),
            I18N.get('elip.fields.title'),
            I18N.get('elip.fields.author'),
            I18N.get('elip.fields.status'),
            I18N.get('elip.fields.createdAt')
          ]
        ],
        itemsCSV
      )
    }
    return (
      <Container>
        <Row type="flex" align="middle" justify="space-between">
          {title}
          {createBtn}
        </Row>
        <Row
          type="flex"
          align="middle"
          justify="end"
          style={{ marginTop: 20, marginBottom: 20 }}
        >
          {searchInput}
          {isLogin && filterBtns}
        </Row>
        <Row type="flex" align="middle" justify="end">
          {isSecretary && (
            <CSVLink data={dataCSV} style={{ marginBottom: 16 }}>
              {I18N.get('elip.button.exportAsCSV')}
            </CSVLink>
          )}
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
