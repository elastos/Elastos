import React from 'react'
import { Link } from 'react-router-dom'
import _ from 'lodash'
import styled from 'styled-components'
import {
  Pagination, Modal, Button, Col, Row, Select, Spin, Checkbox, Input
} from 'antd'
import URI from 'urijs'
import I18N from '@/I18N'
import { loginRedirectWithQuery, logger } from '@/util'
import StandardPage from '@/module/page/StandardPage'
import Footer from '@/module/layout/Footer/Container'
import SuggestionForm from '@/module/form/SuggestionForm/Container'
import ActionsContainer from '../common/actions/Container'
import MetaContainer from '../common/meta/Container'
import TagsContainer from '../common/tags/Container'
import { SUGGESTION_STATUS, CONTENT_TYPE, SUGGESTION_TAG_TYPE } from '@/constant'
import { breakPoint } from '@/constants/breakPoint'
import DraftEditor from '@/module/common/DraftEditor'
import PageHeader from './PageHeader'

import './style.scss'

const SORT_BY = {
  createdAt: 'createdAt',
  likesNum: 'likesNum',
  activeness: 'activeness',
  viewsNum: 'viewsNum'
}
const DEFAULT_SORT = SORT_BY.createdAt

/**
 * This uses new features such as infinite scroll and pagination, therefore
 * we do some different things such as only loading the data from the server
 */
export default class extends StandardPage {
  constructor(props) {
    super(props)

    const uri = URI(props.location.search || '')

    // we use the props from the redux store if its retained
    this.state = {
      showForm: uri.hasQuery('create'),
      showArchived: false,

      // named status since we eventually want to use a struct of statuses to filter on
      referenceStatus: false,
      isDropdownActionOpen: false,
      showMobile: false,
      results: 10,
      total: 0,
      search: ''
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

  ord_renderContent() {
    const headerNode = this.renderHeader()
    const filterNode = this.renderFilters()
    const createForm = this.renderCreateForm()
    const listNode = this.renderList()
    const sortActionsNode = this.renderSortActions()
    return (
      <div>
        <div className="suggestion-header">
          {headerNode}
        </div>
        <SuggestionContainer className="p_SuggestionList">
          <Row
            type="flex"
            justify="space-between"
            align="middle"
            style={{margin: '24px 0 48px'}}
          >
            <Col xs={24} sm={12} style={{ paddingTop: 24 }}>
              <Input.Search
                defaultValue={this.state.search}
                onSearch={this.handleSearch}
                placeholder={I18N.get('suggestion.form.search')}
              />
            </Col>
            <Col xs={24} sm={12} style={{textAlign: 'right', paddingTop: 24}}>
              <Button onClick={this.toggleArchivedList} className="btn-view-archived">
                {this.state.showArchived === false ?
                  I18N.get('suggestion.viewArchived') :
                  I18N.get('suggestion.viewAll')
                }
              </Button>
              <Button onClick={this.showCreateForm} className="btn-create-suggestion">
                {I18N.get('suggestion.add')}
              </Button>
            </Col>
          </Row>
          <Row
            type="flex"
            justify="space-between"
            align="middle"
            style={{ borderBottom: '1px solid #E5E5E5'}}
          >
            <Col md={24} xl={18} style={{ paddingBottom: 24 }}>{filterNode}</Col>
            <Col md={24} xl={6} style={{ paddingBottom: 24, textAlign: 'right' }}>{sortActionsNode}</Col>
          </Row>
          
          <Row gutter={24} style={{marginTop: 32}}>
            <Col span={24}>
              {listNode}
            </Col>
          </Row>
          {createForm}
        </SuggestionContainer>
        <Footer />
      </div>
    )
  }

  handleSearch = search => {
    this.setState({ search }, this.debouncedRefetch)
  }

  onFormSubmit = async (param) => {
    try {
      await this.props.create(param)
      this.setState({ showForm: false })
      this.refetch()
    } catch (error) {
      logger.error(error)
    }
  }

  renderCreateForm = () => {
    const props = {
      onCancel: this.hideCreateForm,
      onSubmit: this.onFormSubmit
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
        { this.state.showForm
          && <SuggestionForm {...props} />
        }
      </Modal>
    )
  }

  showCreateForm = () => {
    const { isLogin, history } = this.props
    if (!isLogin) {
      const query = { create: true }
      loginRedirectWithQuery({ query })
      history.push('/login')
      return
    }
    this.props.history.push('/suggestion/create')
  }

  hideCreateForm = () => {
    this.setState({ showForm: false })
  }

  toggleArchivedList = async () => {
    await this.setState(prevState => ({
      showArchived: !prevState.showArchived,

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
        <SuggestionContainer
          className="title komu-a cr-title-with-icon">
          {this.props.header || I18N.get('suggestion.title').toUpperCase()}
        </SuggestionContainer>

        <HeaderDiagramContainer>
          <SuggestionContainer>
            <PageHeader />
            <HeaderDesc>
              {I18N.get('suggestion.intro.1')}
              <Link to="/proposals">{I18N.get('suggestion.intro.1.proposals')}</Link>
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
      activeness: I18N.get('suggestion.activeness'),
    }
    const sortBy = this.props.sortBy || DEFAULT_SORT
    return (
      <div>
        {I18N.get('suggestion.sort')}: {' '}
        <Select
          name="type"
          style={{width: 200, marginLeft: 16}}
          onChange={this.onSortByChanged}
          value={sortBy}
        >
          {_.map(SORT_BY, value => (
            <Select.Option key={value} value={value}>
              {SORT_BY_TEXT[value]}
            </Select.Option>
          ))}
        </Select>
      </div>
    )
  }

  renderFilters() {
    const {
      tagsIncluded: {
        infoNeeded,
        underConsideration
      }
    } = this.props
    
    return (
      <Row type="flex" align="middle">
        <Col xs={24} sm={24} md={2}>
          {I18N.get('suggestion.tag.show')}:
        </Col>
        <Col xs={24} sm={24} md={7}>
          <Checkbox defaultChecked={underConsideration} onChange={this.onUnderConsiderationChange} />
          <CheckboxText>{I18N.get('suggestion.tag.type.UNDER_CONSIDERATION')}</CheckboxText>
        </Col>
        <Col xs={24} sm={24} md={7}>
          <Checkbox defaultChecked={infoNeeded} onChange={this.onInfoNeededChange} />
          <CheckboxText>{I18N.get('suggestion.tag.type.INFO_NEEDED')}</CheckboxText>
        </Col>
        <Col xs={24} sm={24} md={7}>
          <Checkbox defaultChecked={this.state.referenceStatus} onChange={this.onReferenceStatusChange} />
          <CheckboxText>{I18N.get('suggestion.tag.type.ADDED_TO_PROPOSAL')}</CheckboxText>
        </Col>
      </Row>
    )
  }

  onInfoNeededChange = async (e) => {
    const {onTagsIncludedChanged, tagsIncluded, changePage} = this.props
    tagsIncluded.infoNeeded = e.target.checked

    await changePage(1)
    await onTagsIncludedChanged(tagsIncluded)
    await this.refetch()
  }

  onUnderConsiderationChange = async (e) => {
    const {onTagsIncludedChanged, tagsIncluded, changePage} = this.props
    tagsIncluded.underConsideration = e.target.checked

    await changePage(1)
    await onTagsIncludedChanged(tagsIncluded)
    await this.refetch()
  }

  // checked = boolean
  onReferenceStatusChange = async (e) => {

    const {onReferenceStatusChanged} = this.props

    // the first onReferenceStatusChanged is the props fn from Container
    await this.setState({referenceStatus: e.target.checked})
    await onReferenceStatusChanged(e.target.checked)
    await this.refetch()
  }

  renderList() {
    const {dataList, loading} = this.props
    const loadingNode = <div className="center"><Spin size="large"/></div>
    const paginationNode = this.renderPagination()
    let result = loadingNode
    if (!loading) {
      if (_.isEmpty(dataList)) {
        result = <NoData>{I18N.get('suggestion.nodata')}</NoData>
      } else {
        result = _.map(dataList, data => this.renderItem(data))
      }
    }

    return (
      <div>
        <div className="list-container">
          {result}
        </div>
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
          <DraftEditor value={data.abstract} editorEnabled={false} contentType={CONTENT_TYPE.MARKDOWN} />
          {_.isArray(data.link) && (data.link.map((link) => {
            return <ItemLinkWrapper key={link}><a target="_blank" href={link}>{link}</a></ItemLinkWrapper>
          }))}
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
      onChange: this.onPageChanged,
    }
    return <Pagination {...props} className="cr-pagination" />
  }

  renderMetaNode = detail => <MetaContainer data={detail} user={this.props.user} />

  renderTagsNode = detail => <TagsContainer data={detail} />

  renderActionsNode = (detail, refetch) => <ActionsContainer data={detail} listRefetch={refetch}/>

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
    const { results, referenceStatus, search } = this.state
    const query = {
      status: this.state.showArchived ? SUGGESTION_STATUS.ARCHIVED : SUGGESTION_STATUS.ACTIVE,
      page,
      results
    }
    const {
      tagsIncluded: {
        infoNeeded,
        underConsideration
      }
    } = this.props
    let included = ''

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

    // TODO
    if (sortBy) {
      query.sortBy = sortBy
    }

    if (search) {
      query.search = search
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
      results: this.state.results,
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
