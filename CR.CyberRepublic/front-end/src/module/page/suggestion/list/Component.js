import React from 'react'
import { Link } from 'react-router-dom'
import _ from 'lodash'
import styled from 'styled-components'
import {
  Pagination, Modal, Button, Col, Row, Select, Spin,
} from 'antd'
import URI from 'urijs'
import I18N from '@/I18N'
import { loginRedirectWithQuery } from '@/util'
import StandardPage from '../../StandardPage'
import Footer from '@/module/layout/Footer/Container'
import MySuggestion from '../my_list/Container'
import SuggestionForm from '@/module/form/SuggestionForm/Container'
import ActionsContainer from '../common/actions/Container'
import MetaContainer from '../common/meta/Container'
import suggestionImg from '@/assets/images/SuggestionToProposal.png'
import suggestionZhImg from '@/assets/images/SuggestionToProposal.zh.png'
import { breakPoint } from '@/constants/breakPoint'
import { text, bg } from '@/constants/color'

import MediaQuery from 'react-responsive'
import { MAX_WIDTH_MOBILE, MIN_WIDTH_PC, LG_WIDTH } from '@/config/constant'

import './style.scss'

const SORT_BY = {
  createdAt: 'createdAt',
  likesNum: 'likesNum',
  activeness: 'activeness',
  viewsNum: 'viewsNum'
}
const DEFAULT_SORT = SORT_BY.createdAt

// this is used by the CSS too - TODO: move everything to styled-components
const SIDE_PADDING = '108px'

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
      isDropdownActionOpen: false,
      showMobile: false,
      page: 1,
      results: 10,
      total: 0,
    }
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
    const addButtonNode = this.renderAddButton()
    const actionsNode = this.renderHeaderActions()
    const mySuggestionNode = this.renderMySuggestion()
    const createForm = this.renderCreateForm()
    const listNode = this.renderList()

    return (
      <div>
        <div className="suggestion-header">
          {headerNode}
        </div>
        <SuggestionContainer className="p_SuggestionList">
          <MediaQuery maxWidth={LG_WIDTH}>
            <Row>
              <Col>
                {addButtonNode}
                {/* mySuggestionNode */}
              </Col>
            </Row>
            <Row>
              <Col>
                <br />
              </Col>
            </Row>
            <Row>
              <Col>
                {actionsNode}
                {listNode}
              </Col>
            </Row>
          </MediaQuery>
          <MediaQuery minWidth={LG_WIDTH + 1}>
            <Row gutter={24}>
              <Col span={16}>{actionsNode}</Col>
              <Col span={8}>{addButtonNode}</Col>
            </Row>
            <Row gutter={24}>
              <Col span={24}>
                {listNode}
              </Col>
              {/* <Col span={8}>{mySuggestionNode}</Col> */}
            </Row>
          </MediaQuery>
          {createForm}
        </SuggestionContainer>
        <Footer />
      </div>
    )
  }

  onFormSubmit = async (param) => {
    try {
      await this.props.create(param)
      this.showCreateForm()
      this.refetch()
    } catch (error) {
      // console.log(error)
    }
  }

  renderCreateForm = () => {
    const props = {
      onFormCancel: this.showCreateForm,
      onFormSubmit: this.onFormSubmit,
    }

    return (
      <Modal
        className="project-detail-nobar"
        maskClosable={false}
        visible={this.state.showForm}
        onOk={this.showCreateForm}
        onCancel={this.showCreateForm}
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
    const { showForm } = this.state
    if (!isLogin) {
      const query = { create: true }
      loginRedirectWithQuery({ query })
      history.push('/login')
      return
    }
    this.setState({
      showForm: !showForm,
    })
  }

  renderHeader() {
    return (
      <div>
        <SuggestionContainer className="title komu-a cr-title-with-icon">{this.props.header || I18N.get('suggestion.title').toUpperCase()}</SuggestionContainer>

        <HeaderDiagramContainer>
          <SuggestionContainer>
            <img src={I18N.getLang() === 'zh' ? suggestionZhImg : suggestionImg}/>
          </SuggestionContainer>
        </HeaderDiagramContainer>

        <SuggestionContainer>
          <HeaderDesc>
            {I18N.get('suggestion.intro.1')}
            <Link to="/proposals">{I18N.get('suggestion.intro.1.proposals')}</Link>
            {I18N.get('suggestion.intro.1.1')}
            <br/>
            <br/>
            {I18N.get('suggestion.intro.3')}
            {localStorage.getItem('lang') === 'en' ?
              <a href="https://www.cyberrepublic.org/docs/#/guide/suggestions" target="_blank">https://www.cyberrepublic.org/docs/#/guide/suggestions</a> :
              <a href="https://www.cyberrepublic.org/docs/#/zh/guide/suggestions" target="_blank">https://www.cyberrepublic.org/docs/#/zh/guide/suggestions</a>
            }
          </HeaderDesc>
        </SuggestionContainer>
      </div>
    )
  }

  // list header
  renderHeaderActions() {
    const SORT_BY_TEXT = {
      createdAt: I18N.get('suggestion.new'),
      likesNum: I18N.get('suggestion.likes'),
      viewsNum: I18N.get('suggestion.mostViews'),
      activeness: I18N.get('suggestion.activeness'),
    }
    const sortBy = this.props.sortBy || DEFAULT_SORT
    return (
      <div className="header-actions-container">
        <div>
          <h2 className="title komu-a">{I18N.get('suggestion.listTitle').toUpperCase()}</h2>
        </div>
        <MediaQuery maxWidth={LG_WIDTH}>
          {I18N.get('suggestion.sort')}: &nbsp;
          <Select
            name="type"
            style={{width: 200}}
            onChange={this.onSortByChanged}
            value={sortBy}
          >
            {_.map(SORT_BY, value => (
              <Select.Option key={value} value={value}>
                {SORT_BY_TEXT[value]}
              </Select.Option>
            ))}
          </Select>
        </MediaQuery>
        <MediaQuery minWidth={LG_WIDTH + 1}>
          {I18N.get('suggestion.sort')}: &nbsp;
          <Button.Group className="filter-group">
            {_.map(SORT_BY, value => (
              <Button
                key={value}
                onClick={() => this.onSortByChanged(value)}
                className={(sortBy === value && 'cr-strikethrough') || ''}
              >
                {SORT_BY_TEXT[value]}
              </Button>
            ))}
          </Button.Group>
        </MediaQuery>
      </div>
    )
  }

  renderAddButton() {
    return (
      <AddButtonContainer className="pull-right filter-group btn-create-suggestion">
        <Button onClick={this.showCreateForm}>
          {I18N.get('suggestion.add')}
        </Button>
      </AddButtonContainer>
    )
  }

  renderList() {
    const { dataList, loading } = this.props
    const loadingNode = <div className="center"><Spin size="large" /></div>
    const paginationNode = this.renderPagination()
    let result = loadingNode
    if (!loading) {
      if (_.isEmpty(dataList)) {
        result = <div className="center">{I18N.get('suggestion.nodata')}</div>
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
    const actionsNode = this.renderActionsNode(data)
    const metaNode = this.renderMetaNode(data)
    const title = <ItemTitle to={href} className="title-link">{data.title}</ItemTitle>
    return (
      <div key={data._id} className="item-container">
        {metaNode}
        {title}
        <ShortDesc>
          {data.shortDesc}
        </ShortDesc>
        {actionsNode}
      </div>
    )
  }

  renderPagination() {
    const { total } = this.props
    const { results, page } = this.state
    const props = {
      pageSize: results,
      total,
      current: page,
      onChange: this.loadPage,
    }
    return <Pagination {...props} className="cr-pagination" />
  }

  renderMetaNode = detail => <MetaContainer data={detail} />

  renderActionsNode = detail => <ActionsContainer data={detail} />

  renderMySuggestion = () => <MySuggestion />

  onSortByChanged = async (sortBy) => {
    await this.props.onSortByChanged(sortBy)
    await this.refetch()
  }

  /**
   * Builds the query from the current state
   */
  getQuery = () => {
    const sortBy = this.props.sortBy || DEFAULT_SORT
    const { page, results } = this.state
    const query = {
      page,
      results,
    }
    // TODO
    if (sortBy) {
      query.sortBy = sortBy
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
      this.setState({ page })
    } catch (e) {
      // Do not update page in state if the call fails
    }

    this.setState({ loadingMore: false })
  }

  gotoDetail(id) {
    this.props.history.push(`/suggestion/${id}`)
  }
}

const HeaderDiagramContainer = styled.div`
  background-color: #162f45;
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
  color: ${text.newGray};
  transition: all 0.3s;
  font-weight: 400;
  text-decoration: none;
  margin-top: 8px;
  margin-bottom: 4px;
  display: block;
  &:hover {
    color: $link_color;
  }
  
  background-color: ${bg.blue};
  
  padding: 4px 8px;
  border: 1px solid #e4effd;
  border-radius: 4px;
`

const ShortDesc = styled.div`
  font-weight: 200;
  padding: 4px 8px 0; 
`

const HeaderDesc = styled.div`
  width: 60%;
  font-weight: 200;
  padding: 24px 0;
`


const SuggestionContainer = styled.div`
  max-width: 1200px;
  margin: 0 auto;
  
  @media only screen and (max-width: ${breakPoint.xl}) {
    margin: 0 5%;
  }
`

const AddButtonContainer = styled.div`
  padding-top: 24px;  
`
