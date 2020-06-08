import React from 'react'
import { Link } from 'react-router-dom'
import _ from 'lodash'
import moment from 'moment/moment'
import styled from 'styled-components'
import {
  Pagination,
  Modal,
  Button,
  Col,
  Row,
  Select,
  Spin,
  DatePicker,
  Checkbox,
  Input
} from 'antd'
import rangePickerLocale from 'antd/es/date-picker/locale/zh_CN'
import URI from 'urijs'
import I18N from '@/I18N'
import { loginRedirectWithQuery, logger } from '@/util'
import StandardPage from '@/module/page/StandardPage'
import Footer from '@/module/layout/Footer/Container'
import SuggestionForm from '@/module/form/SuggestionForm/Container'
import ActionsContainer from '../common/actions/Container'
import MetaContainer from '../common/meta/Container'
import Meta from '@/module/common/Meta'
import TagsContainer from '../common/tags/Container'
import { SUGGESTION_STATUS, SUGGESTION_TAG_TYPE } from '@/constant'
import { breakPoint } from '@/constants/breakPoint'
import MarkdownPreview from '@/module/common/MarkdownPreview'
import { ReactComponent as UpIcon } from '@/assets/images/icon-up.svg'
import { ReactComponent as DownIcon } from '@/assets/images/icon-down.svg'
import SuggestionPopupNotification from '@/module/common/SuggestionPopupNotification/Container'
import PageHeader from './PageHeader'
import SearchBox from './SearchBox'

import './style.scss'

const { RangePicker } = DatePicker

const SORT_BY = {
  createdAt: 'createdAt',
  likesNum: 'likesNum',
  activeness: 'activeness',
  viewsNum: 'viewsNum'
}
const DEFAULT_SORT = SORT_BY.createdAt

const BUDGET_REQUESTED_OPTIONS = {
  1: { value: '0 - 100 (ELA)', budgetLow: 0, budgetHigh: 100 },
  2: { value: '100 - 1000 (ELA)', budgetLow: 100, budgetHigh: 1000 },
  3: { value: '> 1000 (ELA)', budgetLow: 1000 }
}

/**
 * This uses new features such as infinite scroll and pagination, therefore
 * we do some different things such as only loading the data from the server
 */
export default class extends StandardPage {
  constructor(props) {
    super(props)

    const { isVisitableFilter } = this.props
    const {
      referenceStatus,
      infoNeeded,
      underConsideration,
      search,
      filter,
      status,
      budgetRequested,
      creationDate,
      author,
      type
    } = this.props.filters
    const uri = URI(props.location.search || '')

    // we use the props from the redux store if its retained
    this.state = {
      showForm: uri.hasQuery('create'),
      showArchived: false,
      showDidModal: uri.hasQuery('create'),
      showOldData: false,
      // named status since we eventually want to use a struct of statuses to filter on
      referenceStatus,
      infoNeeded,
      underConsideration,
      isDropdownActionOpen: false,
      showMobile: false,
      results: 10,
      total: 0,
      search,
      filter,
      status,
      budgetRequested,
      creationDate,
      author,
      type,
      isVisitableFilter
    }
    this.debouncedRefetch = _.debounce(this.refetch.bind(this), 300)
  }

  componentDidMount() {
    super.componentDidMount()
    this.refetch()
  }

  componentWillUnmount() {
    this.props.resetAll()
  }

  handleFilter = () => {
    const { isVisitableFilter } = this.state
    this.setState({ isVisitableFilter: !isVisitableFilter })
  }

  handleFilterChange = (filter) => {
    this.setState({ filter })
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

  handleUnderConsiderationChange = (e) => {
    this.setState({ underConsideration: e.target.checked })
  }

  handleInfoNeededChange = (e) => {
    this.setState({ infoNeeded: e.target.checked })
  }

  handleReferenceStatusChange = (e) => {
    this.setState({ referenceStatus: e.target.checked })
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

  handleClearFilter = () => {
    const defaultFiltes = this.props.getDefaultFilters()
    this.setState({ ...defaultFiltes })
    this.props.clearFilters()
  }

  handleApplyFilter = () => {
    const {
      referenceStatus,
      infoNeeded,
      underConsideration,
      search,
      filter,
      status,
      budgetRequested,
      creationDate,
      author,
      type
    } = this.state
    this.props.updateFilters({
      referenceStatus,
      infoNeeded,
      underConsideration,
      search,
      filter,
      status,
      budgetRequested,
      creationDate,
      author,
      type
    })
    this.refetch()
  }

  handleExportAsCSV = () => {
    const { exportAsCSV } = this.props
    const query = this.getQuery()
    exportAsCSV(query).then((response) => {
      window.location.href = URL.createObjectURL(response)
    })
  }

  ord_renderContent() {
    const { isVisitableFilter, search, filter } = this.state
    const { isSecretary } = this.props
    const headerNode = this.renderHeader()
    const filterNode = this.renderFilters()
    const filterPanel = this.renderFilterPanel()
    const createForm = this.renderCreateForm()
    const listNode = this.renderList()
    const sortActionsNode = this.renderSortActions()
    const didModal = this.renderDidModal()
    const uri = URI(this.props.location.search || '')
    const popupEndTime = new Date().setTime(1591718400000)
    const nowDate = new Date().getTime()

    return (
      <div>
        { popupEndTime < nowDate ? null : <SuggestionPopupNotification />}
        <Meta title="Cyber Republic - Elastos" />
        <div className="suggestion-header">{headerNode}</div>
        <SuggestionContainer className="p_SuggestionList">
          <Row
            type="flex"
            justify="space-between"
            align="middle"
            style={{ margin: '24px 0 48px' }}
          >
            <Col xs={24} sm={12} style={{ paddingTop: 24 }}>
              <SearchBox
                search={this.handleSearch}
                onChange={this.handleSearchChange}
                value={search}
                filterValue={filter}
                onFilterChange={this.handleFilterChange}
              />
            </Col>
            {filterNode}
            <Col xs={24} sm={8} style={{ textAlign: 'right', paddingTop: 24 }}>
              <Button
                onClick={this.showCreateForm}
                className="btn-create-suggestion"
              >
                {I18N.get('suggestion.add')}
              </Button>
            </Col>
          </Row>
          {isVisitableFilter && filterPanel}
          <Row
            type="flex"
            justify="space-between"
            align="middle"
            style={{ borderBottom: '1px solid #E5E5E5' }}
          >
            <Col md={24} xl={12} style={{ paddingBottom: 24 }}>
              {sortActionsNode}
            </Col>
            <Col
              md={24}
              xl={12}
              style={{ paddingBottom: 24, textAlign: 'right' }}
            >
              <Button
                type="link"
                className="btn-link"
                onClick={this.toggleArchivedList}
              >
                {this.state.showArchived === false
                  ? I18N.get('suggestion.viewArchived')
                  : I18N.get('suggestion.viewAll')}
              </Button>
              {isSecretary && <SplitLabel />}
              {isSecretary && (
                <Button
                  type="link"
                  className="btn-link"
                  onClick={this.handleExportAsCSV}
                >
                  {I18N.get('elip.button.exportAsCSV')}
                </Button>
              )}
              <SplitLabel />
              <Button
                type="link"
                className="btn-link"
                onClick={this.viewOldData}
              >
                {this.state.showOldData === false
                  ? I18N.get('suggestion.btn.viewOldData')
                  : I18N.get('suggestion.btn.viewNewData')}
              </Button>
            </Col>
          </Row>
          <Row gutter={24} style={{ marginTop: 32 }}>
            <Col span={24}>{listNode}</Col>
          </Row>
          {!uri.hasQuery('create') && didModal}
          {createForm}
        </SuggestionContainer>
        <Footer />
      </div>
    )
  }

  handleSearch = (filter, search) => {
    this.props.updateFilters({ search, filter })
    this.setState({ search, filter }, this.debouncedRefetch)
  }

  onFormSubmit = async (param) => {
    try {
      const rs = await this.props.create(param)
      this.setState({ showForm: false })
      if (rs && rs._id) {
        this.props.history.push(`/suggestion/${rs._id}?new=true`)
      }
    } catch (error) {
      logger.error(error)
    }
  }

  renderCreateForm = () => {
    const props = {
      onCancel: this.hideCreateForm,
      onSubmit: this.onFormSubmit
    }
    const { isLogin, user } = this.props
    if (isLogin && !_.get(user, 'did.id')) {
      return this.renderDidModal()
    }
    return (
      <Modal
        className="project-detail-nobar"
        maskClosable={false}
        visible={this.state.showForm}
        onCancel={this.hideCreateForm}
        footer={null}
        width="70%"
      >
        {this.state.showForm && <SuggestionForm {...props} />}
      </Modal>
    )
  }

  renderDidModal = () => {
    const { history } = this.props
    return (
      <Modal
        className="project-detail-nobar"
        maskClosable={false}
        visible={this.state.showDidModal}
        onCancel={this.hideDidModal}
        footer={null}
        width={500}
      >
        <div style={{ textAlign: 'center', padding: 16 }}>
          <div style={{ marginBottom: 24, fontSize: 16, color: '#000' }}>
            {I18N.get('suggestion.msg.associateDidFirst')}
          </div>
          <Button
            className="cr-btn cr-btn-primary"
            onClick={() => {
              history.push('/profile/info')
            }}
          >
            {I18N.get('suggestion.btn.associateDid')}
          </Button>
        </div>
      </Modal>
    )
  }

  hideDidModal = () => {
    this.setState({ showDidModal: false })
  }

  showCreateForm = () => {
    const { isLogin, history, user } = this.props
    if (!isLogin) {
      const query = { create: true }
      loginRedirectWithQuery({ query })
      history.push('/login')
      return
    }
    if (isLogin && !_.get(user, 'did.id')) {
      this.setState({ showDidModal: true })
      return
    }
    this.props.history.push('/suggestion/create')
  }

  hideCreateForm = () => {
    this.setState({ showForm: false })
  }

  toggleArchivedList = async () => {
    await this.setState((prevState) => ({
      showArchived: !prevState.showArchived,

      // go back to page 1 on toggle
      page: 1,
      results: 10,
      total: 0
    }))

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

  renderHeader() {
    return (
      <div>
        <SuggestionContainer className="title komu-a cr-title-with-icon">
          {this.props.header || I18N.get('suggestion.title').toUpperCase()}
        </SuggestionContainer>

        <HeaderDiagramContainer>
          <SuggestionContainer>
            <PageHeader />
            <HeaderDesc>
              {I18N.get('suggestion.intro.1')}
              <Link to="/proposals">
                {I18N.get('suggestion.intro.1.proposals')}
              </Link>
              {I18N.get('suggestion.intro.1.1')}
              <br />
              <br />
              {I18N.get('suggestion.intro.3')}
              {localStorage.getItem('lang') === 'en' ? (
                <a
                  href="https://www.cyberrepublic.org/docs/#/guide/suggestions"
                  target="_blank"
                >
                  https://www.cyberrepublic.org/docs/#/guide/suggestions
                </a>
              ) : (
                <a
                  href="https://www.cyberrepublic.org/docs/#/zh/guide/suggestions"
                  target="_blank"
                >
                  https://www.cyberrepublic.org/docs/#/zh/guide/suggestions
                </a>
              )}
            </HeaderDesc>
          </SuggestionContainer>
        </HeaderDiagramContainer>
      </div>
    )
  }

  renderSortActions() {
    const SORT_BY_TEXT = {
      createdAt: I18N.get('suggestion.new'),
      likesNum: I18N.get('suggestion.likes'),
      viewsNum: I18N.get('suggestion.mostViews'),
      activeness: I18N.get('suggestion.activeness')
    }
    const sortBy = this.props.sortBy || DEFAULT_SORT
    return (
      <div>
        {I18N.get('suggestion.sort')}:{' '}
        <Select
          name="type"
          style={{ width: 200, marginLeft: 16 }}
          onChange={this.onSortByChanged}
          value={sortBy}
        >
          {_.map(SORT_BY, (value) => (
            <Select.Option key={value} value={value}>
              {SORT_BY_TEXT[value]}
            </Select.Option>
          ))}
        </Select>
      </div>
    )
  }

  renderFilters() {
    const { isVisitableFilter } = this.state
    return (
      <FilterLabel xs={24} sm={2} style={{ paddingTop: 24 }}>
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
  }

  renderFilterPanel() {
    const {
      referenceStatus,
      infoNeeded,
      underConsideration,
      status,
      budgetRequested,
      creationDate,
      author,
      type
    } = this.state
    const typeMap = {
      1: I18N.get('suggestion.form.type.newMotion'),
      2: I18N.get('suggestion.form.type.motionAgainst'),
      3: I18N.get('suggestion.form.type.anythingElse')
    }
    const lang = localStorage.getItem('lang') || 'en'
    const rangePickerOptions = {}
    if (lang === 'zh') {
      rangePickerOptions.locale = rangePickerLocale
    }
    return (
      <FilterPanel>
        <Row type="flex" gutter={10} className="filter">
          <Col span={8} className="filter-panel">
            <FilterContent>
              <FilterItem>
                <FilterItemLabel>
                  {I18N.get('suggestion.fields.status')}
                </FilterItemLabel>
                <Select
                  className="filter-input"
                  value={status}
                  onChange={this.handleStatusChange}
                >
                  {_.map(SUGGESTION_STATUS, (value) => (
                    <Select.Option key={value} value={value}>
                      {I18N.get(`suggestion.status.${value}`)}
                    </Select.Option>
                  ))}
                </Select>
              </FilterItem>
              <FilterItem>
                <FilterItemLabel>
                  {I18N.get('suggestion.fields.budgetRequested')}
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
            </FilterContent>
          </Col>
          <Col span={8} className="filter-panel">
            <FilterContent>
              <FilterItem>
                <Checkbox
                  checked={underConsideration}
                  onChange={this.handleUnderConsiderationChange}
                />
                <CheckboxText>
                  {I18N.get('suggestion.tag.type.UNDER_CONSIDERATION')}
                </CheckboxText>
              </FilterItem>
              <FilterItem className="filter-checkbox">
                <Checkbox
                  checked={infoNeeded}
                  onChange={this.handleInfoNeededChange}
                />
                <CheckboxText>
                  {I18N.get('suggestion.tag.type.INFO_NEEDED')}
                </CheckboxText>
              </FilterItem>
              <FilterItem className="filter-checkbox">
                <Checkbox
                  checked={referenceStatus}
                  onChange={this.handleReferenceStatusChange}
                />
                <CheckboxText>
                  {I18N.get('suggestion.tag.type.ADDED_TO_PROPOSAL')}
                </CheckboxText>
              </FilterItem>
            </FilterContent>
          </Col>
          <Col span={8} className="filter-panel">
            <FilterContent>
              <FilterItem>
                <FilterItemLabel>
                  {I18N.get('suggestion.fields.creationDate')}
                </FilterItemLabel>
                <RangePicker
                  className="filter-input"
                  onChange={this.handleCreationDateChange}
                  value={creationDate}
                  {...rangePickerOptions}
                />
              </FilterItem>
              <FilterItem>
                <FilterItemLabel>
                  {I18N.get('suggestion.fields.author')}
                </FilterItemLabel>
                <div className="filter-input">
                  <Input value={author} onChange={this.handleAuthorChange} />
                </div>
              </FilterItem>
              <FilterItem>
                <FilterItemLabel>
                  {I18N.get('suggestion.fields.type')}
                </FilterItemLabel>
                <Select
                  className="filter-input"
                  value={type}
                  onChange={this.handleTypeChange}
                >
                  {_.map(typeMap, (value, key) => (
                    <Select.Option key={key} value={key}>
                      {value}
                    </Select.Option>
                  ))}
                </Select>
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

  onInfoNeededChange = async (e) => {
    const { onTagsIncludedChanged, tagsIncluded, changePage } = this.props
    tagsIncluded.infoNeeded = e.target.checked

    await changePage(1)
    await onTagsIncludedChanged(tagsIncluded)
    await this.refetch()
  }

  onUnderConsiderationChange = async (e) => {
    const { onTagsIncludedChanged, tagsIncluded, changePage } = this.props
    tagsIncluded.underConsideration = e.target.checked

    await changePage(1)
    await onTagsIncludedChanged(tagsIncluded)
    await this.refetch()
  }

  // checked = boolean
  onReferenceStatusChange = async (e) => {
    const { onReferenceStatusChanged } = this.props

    // the first onReferenceStatusChanged is the props fn from Container
    await this.setState({ referenceStatus: e.target.checked })
    await onReferenceStatusChanged(e.target.checked)
    await this.refetch()
  }

  renderList() {
    const { dataList, loading } = this.props
    const loadingNode = (
      <div className="center">
        <Spin size="large" />
      </div>
    )
    const paginationNode = this.renderPagination()
    let result = loadingNode
    if (!loading) {
      if (_.isEmpty(dataList)) {
        result = <NoData>{I18N.get('suggestion.nodata')}</NoData>
      } else {
        result = _.map(dataList, (data) => this.renderItem(data))
      }
    }

    return (
      <div>
        <div className="list-container">{result}</div>
        {paginationNode}
      </div>
    )
  }

  renderItem = (data) => {
    const href = `/suggestion/${data._id}`
    const actionsNode = this.renderActionsNode(data, this.refetch)
    const metaNode = this.renderMetaNode(data)
    const title = <ItemTitle to={href}>{data.title}</ItemTitle>
    const tagsNode = this.renderTagsNode(data)
    return (
      <div key={data._id} className="item-container">
        {metaNode}
        {title}
        {tagsNode}
        <ShortDesc>
          <MarkdownPreview content={data.abstract} />
          {_.isArray(data.link) &&
            data.link.map((link) => {
              return (
                <ItemLinkWrapper key={link}>
                  <a target="_blank" href={link}>
                    {link}
                  </a>
                </ItemLinkWrapper>
              )
            })}
        </ShortDesc>

        {actionsNode}
      </div>
    )
  }

  onPageChanged = (page) => {
    const { changePage } = this.props
    changePage(page)
    this.loadPage(page)
  }

  renderPagination() {
    const { total, page } = this.props
    const { results } = this.state
    const props = {
      pageSize: results,
      total,
      current: page,
      onChange: this.onPageChanged
    }
    return <Pagination {...props} className="cr-pagination" />
  }

  renderMetaNode = (detail) => (
    <MetaContainer data={detail} user={this.props.user} />
  )

  renderTagsNode = (detail) => <TagsContainer data={detail} />

  renderActionsNode = (detail, refetch) => (
    <ActionsContainer data={detail} listRefetch={refetch} />
  )

  onSortByChanged = async (sortBy) => {
    await this.props.onSortByChanged(sortBy)
    await this.refetch()
  }

  /**
   * Builds the query from the current state
   */
  getQuery = () => {
    const sortBy = this.props.sortBy || DEFAULT_SORT
    const { page } = this.props
    const {
      referenceStatus,
      infoNeeded,
      underConsideration,
      search,
      filter,
      status,
      budgetRequested,
      creationDate,
      author,
      type
    } = this.state
    const { results } = this.state
    const query = {
      page,
      results
    }
    let included = ''

    if (this.state.showArchived) {
      query.status = SUGGESTION_STATUS.ARCHIVED
    }

    if (this.state.showOldData) {
      query.old = true
    }

    if (infoNeeded) {
      included = SUGGESTION_TAG_TYPE.INFO_NEEDED
    }
    if (underConsideration) {
      if (_.isEmpty(included)) {
        included = SUGGESTION_TAG_TYPE.UNDER_CONSIDERATION
      } else {
        included = `${included},${SUGGESTION_TAG_TYPE.UNDER_CONSIDERATION}`
      }
    }

    if (!_.isEmpty(included)) {
      query.tagsIncluded = included
    }

    // sending a boolean to be handled by the backend
    query.referenceStatus = referenceStatus

    if (!_.isEmpty(status)) {
      query.status = status
    }

    if (!_.isEmpty(budgetRequested) && budgetRequested > 0) {
      const budget = BUDGET_REQUESTED_OPTIONS[budgetRequested]
      query.budgetLow = budget.budgetLow
      if (budget.budgetHigh) {
        query.budgetHigh = budget.budgetHigh
      }
    }

    if (!_.isEmpty(creationDate)) {
      const formatStr = 'YYYY-MM-DD'
      query.startDate = moment(creationDate[0]).format(formatStr)
      query.endDate = moment(creationDate[1]).format(formatStr)
    }

    if (!_.isEmpty(author)) {
      query.author = author
    }

    if (!_.isEmpty(type)) {
      query.type = type
    }

    // TODO
    if (sortBy) {
      query.sortBy = sortBy
    }

    if (search) {
      query.search = search
    }

    if (filter) {
      query.filter = filter
    }
    return query
  }

  /**
   * Refetch the data based on the current state retrieved from getQuery
   */
  refetch = () => {
    const query = this.getQuery()
    this.props.getList(query)
  }

  loadPage = async (page) => {
    const query = {
      ...this.getQuery(),
      page,
      results: this.state.results
    }

    this.setState({ loadingMore: true })

    try {
      await this.props.loadMore(query)
    } catch (e) {
      // Do not update page in state if the call fails
      logger.error(e)
    }

    this.setState({ loadingMore: false })
  }

  gotoDetail(id) {
    this.props.history.push(`/suggestion/${id}`)
  }
}

const HeaderDiagramContainer = styled.div`
  background-color: #162f45;
  padding-top: 36px;
  padding-bottom: 36px;
  img {
    max-height: 250px;
    @media only screen and (max-width: ${breakPoint.lg}) {
      width: 100%;
    }
  }
`

const ItemTitle = styled(Link)`
  font-size: 20px;
  color: black;
  transition: all 0.3s;
  font-weight: 400;
  text-decoration: none;
  margin: 8px 0;
  display: block;
  &:hover {
    color: $link_color;
  }
`

const ItemLinkWrapper = styled.div`
  margin-top: 8px;
  display: block;
`

const ShortDesc = styled.div`
  margin-top: 8px;
  font-weight: 200;
  .md-RichEditor-editor .public-DraftEditor-content {
    min-height: 10px;
  }
  .md-RichEditor-root {
    padding: 0;
    figure.md-block-image {
      background: none;
    }
    figure.md-block-image figcaption .public-DraftStyleDefault-block {
      text-align: left;
    }
    .public-DraftEditor-content {
      padding: 0px 15px;
    }
  }
`

const HeaderDesc = styled.div`
  font-weight: 200;
  padding: 24px 0;
  color: #fff;
  word-break: break-all;
`

const SuggestionContainer = styled.div`
  max-width: 1200px;
  margin: 0 auto;

  @media only screen and (max-width: ${breakPoint.xl}) {
    margin: 0 5%;
  }
`

const CheckboxText = styled.span`
  margin-left: 10px;
`

const NoData = styled.div`
  text-align: center;
  padding: 25px 0;
`

const FilterLabel = styled(Col)`
  color: #008d85;
  cursor: pointer;
`

const FilterPanel = styled.div`
  .filter {
    margin-top: 20px;
  }
  .filter-btn {
    margin-top: 36px;
    margin-bottom: 58px;
  }
  .filter-input {
    width: 60%;
    padding-right: 15px;
  }
`

const FilterClearBtn = styled.div`
  text-align: center;
  min-width: 155px;
  height: 40px;
  line-height: 40px;
  color: rgba(3, 30, 40, 0.3);
  cursor: pointer;
`

const FilterItem = styled.div`
  display: flex;
  justify-content: flex-start;
  align-items: center;
  padding-left: 15px;
  padding-bottom: 10px;
  &.filter-checkbox {
    padding-top: 10px;
  }
  :first-child {
    padding-top: 20px;
  }
  :last-child {
    padding-bottom: 20px;
  }
`
const FilterContent = styled.div`
  background: #f6f9fd;
  height: 100%;
`

const FilterItemLabel = styled.div`
  width: 40%;
  font-family: Synthese;
  font-size: 14px;
  line-height: 20px;
  color: #000;

  :after {
    content: ':';
  }
`

const SplitLabel = styled.span`
  color: rgba(3, 30, 40, 0.3);
  :after {
    content: '|';
  }
`
