import React from 'react'
import _ from 'lodash'
import moment from 'moment/moment'
import {
  Table,
  Row,
  Col,
  Button,
  Select,
  Input,
  DatePicker,
  Checkbox,
  Icon
} from 'antd'
import rangePickerLocale from 'antd/es/date-picker/locale/zh_CN'
import { CSVLink } from 'react-csv'
import BaseComponent from '@/model/BaseComponent'
import Meta from '@/module/common/Meta'
import I18N from '@/I18N'
import { logger } from '@/util'
import { CVOTE_RESULT, CVOTE_STATUS, CVOTE_CHAIN_STATUS } from '@/constant'
import VoteStats from '../stats/Component'
import userUtil from '@/util/user'
import { ReactComponent as UpIcon } from '@/assets/images/icon-up.svg'
import { ReactComponent as DownIcon } from '@/assets/images/icon-down.svg'

// style
import {
  Container,
  List,
  Item,
  ItemUndecided,
  StyledButton,
  StyledSearch,
  FilterLabel,
  FilterPanel,
  FilterContent,
  FilterItem,
  FilterItemLabel,
  FilterClearBtn,
  CheckboxText,
  SplitLabel,
  ViewOldDataBtn
} from './style'

const { RangePicker } = DatePicker

const FILTERS = {
  ALL: 'all',
  UNVOTED: CVOTE_RESULT.UNDECIDED
}

const BUDGET_REQUESTED_OPTIONS = {
  1: { value: '0 - 100 (ELA)', budgetLow: 0, budgetHigh: 100 },
  2: { value: '100 - 1000 (ELA)', budgetLow: 100, budgetHigh: 1000 },
  3: { value: '> 1000 (ELA)', budgetLow: 1000 }
}

export default class extends BaseComponent {
  constructor(p) {
    super(p)

    const { isVisitableFilter } = this.props
    const {
      voteResult,
      search,
      status,
      budgetRequested,
      hasTrackingMsg,
      isUnvotedByYou,
      creationDate,
      author,
      type,
      endsDate
    } = this.props.filters
    this.state = {
      list: [],
      alllist: [],
      total: 1,
      loading: true,
      page: 1,
      isVisitableFilter,
      voteResult,
      search,
      status,
      budgetRequested,
      hasTrackingMsg,
      isUnvotedByYou,
      creationDate,
      author,
      type,
      endsDate,
      showOldData: false
    }

    this.debouncedRefetch = _.debounce(this.refetch.bind(this), 300)
  }

  async componentDidMount() {
    this.refetch()
  }

  handleFilter = () => {
    const { isVisitableFilter } = this.state
    this.setState({ isVisitableFilter: !isVisitableFilter })
  }

  handleSearchChange = (e) => {
    this.setState({ search: e.target.value })
  }

  handleStatusChange = (status) => {
    this.setState({ status })
  }

  handleBudgetRequestedChange = (budgetRequested) => {
    this.setState({ budgetRequested })
  }

  handleHasTrackingMsgChange = (e) => {
    this.setState({ hasTrackingMsg: e.target.checked })
  }

  handleIsUnvotedByYouChange = (e) => {
    this.setState({ isUnvotedByYou: e.target.checked })
  }

  handleCreationDateChange = (creationDate) => {
    this.setState({ creationDate })
  }

  handleAuthorChange = (e) => {
    this.setState({ author: e.target.value })
  }

  handleTypeChange = (type) => {
    this.setState({ type })
  }

  handleEndsDateChange = (endsDate) => {
    this.setState({ endsDate })
  }

  handleClearFilter = () => {
    const defaultFiltes = this.props.getDefaultFilters()
    this.setState({ ...defaultFiltes })
    this.props.clearFilters()
  }

  handleApplyFilter = () => {
    const {
      voteResult,
      search,
      status,
      budgetRequested,
      hasTrackingMsg,
      isUnvotedByYou,
      creationDate,
      author,
      type,
      endsDate
    } = this.state
    this.props.updateFilters({
      voteResult,
      search,
      status,
      budgetRequested,
      hasTrackingMsg,
      isUnvotedByYou,
      creationDate,
      author,
      type,
      endsDate
    })
    this.refetch()
  }

  viewOldData = async () => {
    await this.setState((state) => ({
      showOldData: !state.showOldData,
      // go back to page 1 on toggle
      page: 1,
      results: 10,
      total: 0
    }))

    this.refetch()
  }

  ord_render() {
    const PROPOSAL_TYPE = {
      1: I18N.get('council.voting.type.newMotion'),
      2: I18N.get('council.voting.type.motionAgainst'),
      3: I18N.get('council.voting.type.anythingElse'),
      4: I18N.get('council.voting.type.standardTrack'),
      5: I18N.get('council.voting.type.process'),
      6: I18N.get('council.voting.type.information')
    }
    const { canManage, isSecretary, isCouncil } = this.props
    const canCreateProposal = !isCouncil && !isSecretary
    const { isVisitableFilter } = this.state

    const columns = [
      {
        title: I18N.get('council.voting.number'),
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
        title: I18N.get('council.voting.title'),
        dataIndex: 'title',
        width: '30%',
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
        title: I18N.get('council.voting.type'),
        dataIndex: 'type',
        render: (type, item) => PROPOSAL_TYPE[type]
      },
      {
        title: I18N.get('council.voting.author'),
        dataIndex: 'proposedBy'
      },
      {
        title: I18N.get('council.voting.votingEndsIn'),
        dataIndex: 'proposedAt',
        key: 'endsIn',
        render: (proposedAt, item) => this.renderEndsIn(item)
      },
      {
        title: I18N.get('council.voting.voteByCouncil'),
        render: (id, item) => this.voteDataByUser(item)
      },
      {
        title: I18N.get('council.voting.status'),
        render: (id, item) => this.renderStatus(item.status)
      },
      {
        title: I18N.get('council.voting.proposedAt'),
        dataIndex: 'proposedAt',
        render: (proposedAt, doc) =>
          this.renderProposed(doc.published, proposedAt || doc.createdAt)
      }
    ]

    if (canManage) {
      columns.splice(1, 0, {
        dataIndex: 'published',
        render: (published, item, index) =>
          published ? <Icon type="eye" /> : <Icon type="eye-invisible" />
      })
    }

    const statusIndicator = (
      <List>
        <Item status={CVOTE_RESULT.SUPPORT} />
        <span>{I18N.get('council.voting.type.support')}</span>
        <Item status={CVOTE_RESULT.REJECT} />
        <span>{I18N.get('council.voting.type.reject')}</span>
        <Item status={CVOTE_RESULT.ABSTENTION} />
        <span>{I18N.get('council.voting.type.abstention')}</span>
        <ItemUndecided status={CVOTE_RESULT.UNDECIDED} />
        <span>{I18N.get('council.voting.type.undecided')}</span>
      </List>
    )

    // no one can see this button
    const createBtn = canManage &&
      canCreateProposal && (
        <Row type="flex" align="middle" justify="end">
          <Col lg={8} md={12} sm={24} xs={24} style={{ textAlign: 'right' }}>
            <StyledButton
              onClick={this.createAndRedirect}
              className="cr-btn cr-btn-primary"
            >
              {I18N.get('from.CVoteForm.button.add')}
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
          {I18N.get('council.voting.proposalList')}
        </h2>
      </Col>
    )
    const searchInput = (
      <Col lg={8} md={8} sm={12} xs={24}>
        <StyledSearch
          value={this.state.search}
          onChange={this.handleSearchChange}
          onSearch={this.searchChangedHandler}
          placeholder={I18N.get('developer.search.search.placeholder')}
        />
      </Col>
    )
    const btns = (
      <Col lg={8} md={8} sm={12} xs={24}>
        {statusIndicator}
      </Col>
    )
    const filterBtns = (
      <FilterLabel>
        <Row
          type="flex"
          gutter={10}
          align="middle"
          justify="start"
          onClick={this.handleFilter}
        >
          <Col>{I18N.get('elip.fields.filter')}</Col>
          <Col>{isVisitableFilter ? <UpIcon /> : <DownIcon />}</Col>
        </Row>
      </FilterLabel>
    )
    const filterPanel = this.renderFilterPanel(PROPOSAL_TYPE)
    const { list, total, loading, page } = this.state
    // const dataCSV = []
    if (isSecretary) {
      /*
         const itemsCSV = _.map(list, v => [
         v.vid,
         v.title,
         PROPOSAL_TYPE[v.type],
         v.proposedBy,
         this.renderEndsInForCSV(v),
         this.voteDataByUserForCSV(v),
         this.renderStatus(v.status),
         _.replace(
         this.renderProposed(v.published, v.proposedAt || v.createdAt) || '',
         ',',
         ' '
         )
         ])
         dataCSV = _.concat(
         [
         [
         I18N.get('council.voting.number'),
         I18N.get('council.voting.title'),
         I18N.get('council.voting.type'),
         I18N.get('council.voting.author'),
         I18N.get('council.voting.votingEndsIn'),
         I18N.get('council.voting.voteByCouncil'),
         I18N.get('council.voting.status'),
         I18N.get('council.voting.proposedAt')
         ]
         ],
         // itemsCSV
         )
       */
    }

    return (
      <div>
        <Meta title="Cyber Republic - Elastos" />
        <Container>
          {createBtn}
          <Row
            type="flex"
            align="bottom"
            justify="space-between"
            style={{ marginTop: 20 }}
          >
            {title}
            {btns}
          </Row>
          <Row
            type="flex"
            align="middle"
            justify="start"
            gutter={40}
            style={{ marginTop: 20, marginBottom: 20 }}
          >
            {searchInput}
            {filterBtns}
          </Row>
          {isVisitableFilter && filterPanel}
          <Row type="flex" align="middle" justify="end">
            {isSecretary && (
              <CSVLink data={this.state.alllist} style={{ marginBottom: 16 }}>
                {I18N.get('elip.button.exportAsCSV')}
              </CSVLink>
            )}
            {isSecretary && <SplitLabel />}
            <ViewOldDataBtn onClick={this.viewOldData}>
              {this.state.showOldData === false
                ? I18N.get('proposal.btn.viewOldData')
                : I18N.get('proposal.btn.viewNewData')}
            </ViewOldDataBtn>
          </Row>
          <Table
            columns={columns}
            loading={loading}
            dataSource={list}
            rowKey={(record) => record._id}
            pagination={{
              current: page,
              pageSize: 10,
              total, // list && list.length,
              onChange: this.onPageChange
            }}
          />
          {createBtn}
        </Container>
      </div>
    )
  }

  onPageChange = (page, pageSize) => {
    this.loadPage(page, pageSize)
  }

  createAndRedirect = async () => {
    sessionStorage.removeItem('proposalPage')

    const { user } = this.props
    const fullName = userUtil.formatUsername(user)
    const { createDraft } = this.props

    const param = {
      title: 'New Proposal',
      proposedBy: fullName,
      proposer: user._id
    }

    this.ord_loading(true)

    try {
      const res = await createDraft(param)
      this.ord_loading(false)
      this.toEditPage(res._id)
    } catch (error) {
      this.ord_loading(false)
    }
  }

  getQuery = () => {
    const {
      status,
      budgetRequested,
      hasTrackingMsg,
      isUnvotedByYou,
      creationDate,
      author,
      type,
      endsDate,
      voteResult,
      search,
      showOldData
    } = this.state
    const query = {}
    const formatStr = 'YYYY-MM-DD'
    if (search) {
      query.search = search
    }
    if (!_.isEmpty(status)) {
      query.status = status
    }
    if (showOldData) {
      query.old = true
    }
    if (!_.isEmpty(budgetRequested) && budgetRequested > 0) {
      const budget = BUDGET_REQUESTED_OPTIONS[budgetRequested]
      query.budgetLow = budget.budgetLow
      if (budget.budgetHigh) {
        query.budgetHigh = budget.budgetHigh
      }
    }
    if (hasTrackingMsg) {
      query.hasTracking = hasTrackingMsg
    }
    if (isUnvotedByYou) {
      query.voteResult = FILTERS.UNVOTED
    } else {
      query.voteResult = voteResult
    }
    if (!_.isEmpty(creationDate)) {
      query.startDate = moment(creationDate[0]).format(formatStr)
      query.endDate = moment(creationDate[1]).format(formatStr)
    }
    if (!_.isEmpty(author)) {
      query.author = author
    }
    if (!_.isEmpty(type)) {
      query.type = type
    }
    if (!_.isEmpty(endsDate)) {
      query.endsInStartDate = moment(endsDate[0]).format(formatStr)
      query.endsInEndDate = moment(endsDate[1]).format(formatStr)
    }
    return query
  }

  refetch = async () => {
    this.ord_loading(true)
    const PROPOSAL_TYPE = {
      1: I18N.get('council.voting.type.newMotion'),
      2: I18N.get('council.voting.type.motionAgainst'),
      3: I18N.get('council.voting.type.anythingElse'),
      4: I18N.get('council.voting.type.standardTrack'),
      5: I18N.get('council.voting.type.process'),
      6: I18N.get('council.voting.type.information')
    }
    const { listData, canManage } = this.props
    const param = this.getQuery()
    const page = 1
    try {
      const { list: allListData, total: allListTotal } = await listData(
        param,
        canManage
      )
      const dataCSV = []
      dataCSV.push([
        I18N.get('council.voting.number'),
        I18N.get('council.voting.title'),
        I18N.get('council.voting.type'),
        I18N.get('council.voting.author'),
        I18N.get('council.voting.votingEndsIn'),
        I18N.get('council.voting.voteByCouncil'),
        I18N.get('council.voting.status'),
        I18N.get('council.voting.proposedAt')
      ])
      _.map(allListData, (v) => {
        dataCSV.push([
          v.vid,
          v.title,
          PROPOSAL_TYPE[v.type],
          v.proposedBy,
          this.renderEndsInForCSV(v),
          this.voteDataByUserForCSV(v),
          this.renderStatus(v.status),
          _.replace(
            this.renderProposed(v.published, v.proposedAt || v.createdAt) || '',
            ',',
            ' '
          )
        ])
      })

      // const page = sessionStorage.getItem('proposalPage')
      param.page = page
      param.results = 10
      const { list, total } = await listData(param, canManage)
      this.setState({
        list,
        alllist: dataCSV,
        total,
        page: (page && parseInt(page)) || 1
      })
    } catch (error) {
      logger.error(error)
    }

    this.ord_loading(false)
  }

  loadPage = async (page, pageSize) => {
    this.ord_loading(true)
    const { listData, canManage } = this.props
    const query = {
      ...this.getQuery(),
      page,
      results: pageSize
    }
    try {
      const { list, total } = await listData(query, canManage)
      // const page = sessionStorage.getItem('proposalPage')
      this.setState({ list, total, page: (page && parseInt(page)) || 1 })
      sessionStorage.setItem('proposalPage', page)
    } catch (error) {
      logger.error(error)
    }
    this.ord_loading(false)
  }

  searchChangedHandler = (search) => {
    sessionStorage.removeItem('proposalPage')
    this.props.updateFilters({ search })
    this.setState({ search }, this.debouncedRefetch)
  }

  toDetailPage(id) {
    this.props.history.push(`/proposals/${id}`)
  }

  toEditPage(id) {
    this.props.history.push(`/proposals/${id}/edit`)
  }

  renderEndsIn = (item) => {
    return this.renderBaseEndsIn(item)
  }

  renderEndsInForCSV = (item) => {
    return this.renderBaseEndsIn(item, true)
  }

  renderBaseEndsIn = (item, isCSV = false) => {
    if (item.status === CVOTE_STATUS.DRAFT) return null
    // only show when status is PROPOSED
    let endsInFloat = moment
      .duration(
        moment(item.proposedAt || item.createdAt)
          .add(7, 'd')
          .diff(moment())
      )
      .as('days')
    if (item.status == CVOTE_STATUS.NOTIFICATION) {
      endsInFloat = moment
        .duration(
          moment(item.proposedAt || item.createdAt)
            .add(14, 'd')
            .diff(moment())
        )
        .as('days')
    }
    if (
      (item.status !== CVOTE_STATUS.PROPOSED &&
        item.status !== CVOTE_STATUS.NOTIFICATION) ||
      endsInFloat <= 0
    ) {
      return I18N.get('council.voting.votingEndsIn.ended')
    }
    if (endsInFloat > 0 && endsInFloat <= 1) {
      const oneDay = `1 ${I18N.get('council.voting.votingEndsIn.day')}`
      return isCSV ? oneDay : <span style={{ color: 'red' }}>{oneDay}</span>
    }
    return `${Math.ceil(endsInFloat)} ${I18N.get(
      'council.voting.votingEndsIn.days'
    )}`
  }

  renderStatus = (status) => {
    return I18N.get(`cvoteStatus.${status}`) || ''
  }

  renderProposed = (published, createdAt) => {
    const lang = localStorage.getItem('lang') || 'en'
    const format = lang === 'en' ? 'MMM D, YYYY' : 'YYYY-MM-DD'
    return published && moment(createdAt).format(format)
  }

  voteDataByUser = (data) => {
    return this.baseVoteDataByUser(data)
  }

  voteDataByUserForCSV = (data) => {
    return this.baseVoteDataByUser(data, true)
  }

  baseVoteDataByUser = (data, isCSV = false) => {
    const { vote_map: voteMap, voteResult, status } = data
    let voteArr
    if (status === CVOTE_STATUS.DRAFT) return null

    if (!_.isEmpty(voteResult)) {
      voteArr = _.map(voteResult, (item) => {
        if (item.status === CVOTE_CHAIN_STATUS.CHAINED) {
          return CVOTE_RESULT[item.value.toUpperCase()]
        }
        return CVOTE_RESULT.UNDECIDED
      })
    } else if (!_.isEmpty(voteMap)) {
      // deal with old data
      voteArr = _.map(
        voteMap,
        (value) => CVOTE_RESULT[value.toUpperCase()] || CVOTE_RESULT.UNDECIDED
      )
    } else {
      return ''
    }
    const supportNum = _.countBy(voteArr)[CVOTE_RESULT.SUPPORT] || 0
    const percentage = (supportNum * 100) / voteArr.length
    const proposalAgreed = percentage > 50
    const percentageStr =
      percentage.toString() && `${percentage.toFixed(1).toString()}%`
    return isCSV ? (
      percentageStr
    ) : (
      <VoteStats
        percentage={percentageStr}
        values={voteArr}
        yes={proposalAgreed}
      />
    )
  }

  renderFilterPanel = (PROPOSAL_TYPE) => {
    const {
      status,
      budgetRequested,
      hasTrackingMsg,
      isUnvotedByYou,
      creationDate,
      author,
      type,
      endsDate
    } = this.state
    const { isCouncil } = this.props
    const lang = localStorage.getItem('lang') || 'en'
    const rangePickerOptions = {}
    if (lang === 'zh') {
      rangePickerOptions.locale = rangePickerLocale
    }
    const colSpan = isCouncil ? 8 : 12
    return (
      <FilterPanel isCouncil={isCouncil}>
        <Row type="flex" gutter={10} className="filter">
          <Col span={colSpan} className="filter-panel">
            <FilterContent>
              <FilterItem>
                <FilterItemLabel isCouncil={isCouncil}>
                  {I18N.get('proposal.fields.status')}
                </FilterItemLabel>
                <Select
                  className="filter-input"
                  value={status}
                  onChange={this.handleStatusChange}
                >
                  {_.map(CVOTE_STATUS, (value) => (
                    <Select.Option key={value} value={value}>
                      {I18N.get(`cvoteStatus.${value}`)}
                    </Select.Option>
                  ))}
                </Select>
              </FilterItem>
              <FilterItem>
                <FilterItemLabel isCouncil={isCouncil}>
                  {I18N.get('proposal.fields.budgetRequested')}
                </FilterItemLabel>
                <Select
                  className="filter-input"
                  value={budgetRequested}
                  onChange={this.handleBudgetRequestedChange}
                >
                  {_.map(BUDGET_REQUESTED_OPTIONS, (item, key) => (
                    <Select.Option key={key} value={key}>
                      {item.value}
                    </Select.Option>
                  ))}
                </Select>
              </FilterItem>
              <FilterItem className="filter-checkbox">
                <Checkbox
                  checked={hasTrackingMsg}
                  onChange={this.handleHasTrackingMsgChange}
                />
                <CheckboxText>
                  {I18N.get('proposal.fields.hasTrackingMsg')}
                </CheckboxText>
              </FilterItem>
            </FilterContent>
          </Col>
          {isCouncil && (
            <Col span={colSpan} className="filter-panel">
              <FilterContent>
                <FilterItem>
                  <Checkbox
                    checked={isUnvotedByYou}
                    onChange={this.handleIsUnvotedByYouChange}
                  />
                  <CheckboxText>
                    {I18N.get('proposal.fields.isUnvotedByYou')}
                  </CheckboxText>
                </FilterItem>
              </FilterContent>
            </Col>
          )}
          <Col span={colSpan} className="filter-panel">
            <FilterContent>
              <FilterItem>
                <FilterItemLabel isCouncil={isCouncil}>
                  {I18N.get('proposal.fields.creationDate')}
                </FilterItemLabel>
                <RangePicker
                  className="filter-input"
                  onChange={this.handleCreationDateChange}
                  value={creationDate}
                  {...rangePickerOptions}
                />
              </FilterItem>
              <FilterItem>
                <FilterItemLabel isCouncil={isCouncil}>
                  {I18N.get('proposal.fields.author')}
                </FilterItemLabel>
                <div className="filter-input">
                  <Input value={author} onChange={this.handleAuthorChange} />
                </div>
              </FilterItem>
              <FilterItem>
                <FilterItemLabel isCouncil={isCouncil}>
                  {I18N.get('proposal.fields.type')}
                </FilterItemLabel>
                <Select
                  className="filter-input"
                  value={type}
                  onChange={this.handleTypeChange}
                >
                  {_.map(PROPOSAL_TYPE, (value, key) => (
                    <Select.Option key={key} value={key}>
                      {value}
                    </Select.Option>
                  ))}
                </Select>
              </FilterItem>
              <FilterItem>
                <FilterItemLabel isCouncil={isCouncil}>
                  {I18N.get('proposal.fields.endsDate')}
                </FilterItemLabel>
                <RangePicker
                  className="filter-input"
                  onChange={this.handleEndsDateChange}
                  value={endsDate}
                  {...rangePickerOptions}
                />
              </FilterItem>
            </FilterContent>
          </Col>
        </Row>
        <Row type="flex" gutter={30} justify="center" className="filter-btn">
          <Col>
            <FilterClearBtn onClick={this.handleClearFilter}>
              {I18N.get('elip.button.clearFilter')}
            </FilterClearBtn>
          </Col>
          <Col>
            <Button
              className="cr-btn cr-btn-primary"
              onClick={this.handleApplyFilter}
            >
              {I18N.get('elip.button.applyFilter')}
            </Button>
          </Col>
        </Row>
      </FilterPanel>
    )
  }
}
