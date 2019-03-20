import React from 'react';
import _ from 'lodash'
import moment from 'moment/moment'
import I18N from '@/I18N'
import StandardPage from '../../StandardPage';

import { Container, Header, Title, Desc, List, Item } from './style'

export default class extends StandardPage {
  componentDidMount() {
    this.refetch()
  }

  componentWillUnmount() {
    this.props.resetDetail()
  }

  ord_renderContent() {
    const { dataList } = this.props
    const listNode = this.renderList()

    if (_.isEmpty(dataList)) {
      return <div className="center">{I18N.get('suggestion.nodata')}</div>
    }

    const headerNode = this.renderHeader()
    return (
      <Container>
        {headerNode}
        {listNode}
      </Container>
    )
  }

  renderHeader() {
    return (
      <Header className="title komu-a">{this.props.header || I18N.get('suggestion.editHistory').toUpperCase()}</Header>
    )
  }

  renderList() {
    const { dataList } = this.props
    const result = _.map(dataList, data => this.renderItem(data))
    return <List>{result}</List>
  }

  renderItem = (data) => {
    const { _id, title, desc, link, updatedAt } = data
    const metaNode = (
      <div>
        <span style={{ marginRight: 5 }}>{I18N.get('suggestion.edited')}</span>
        <span>{moment(updatedAt).format('MMM D, YYYY')}</span>
      </div>
    )
    return (
      <Item key={_id}>
        {metaNode}
        <Title>{title}</Title>
        <Desc dangerouslySetInnerHTML={{ __html: desc }} />
      </Item>
    )
  }

  refetch = async () => {
    const { match, resetDetail, getDetail } = this.props
    const id = _.get(match, 'params.id')
    await resetDetail()
    getDetail({ id, incViewsNum: false })
  }

  linkSuggestionDetail(suggestionId) {
    this.props.history.push(`/suggestion/${suggestionId}`)
  }
}
