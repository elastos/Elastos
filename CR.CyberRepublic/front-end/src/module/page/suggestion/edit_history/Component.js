import React from 'react';
import _ from 'lodash'
import moment from 'moment/moment'
import I18N from '@/I18N'
import BackLink from "@/module/shared/BackLink/Component"
import StandardPage from '../../StandardPage'
import Diff from './Diff'

import { Container, Header, Title, List, Item } from './style'

export default class extends StandardPage {
  componentDidMount() {
    this.refetch()
  }

  componentWillUnmount() {
    this.props.resetDetail()
  }

  ord_renderContent() {
    const { dataList, match } = this.props
    const id = _.get(match, 'params.id')

    if (_.isEmpty(dataList)) {
      // return <div className="center">{I18N.get('suggestion.nodata')}</div>
      return null
    }
    const listNode = this.renderList()

    const headerNode = this.renderHeader()
    return (
      <Container>
        <BackLink link={`/suggestion/${id}`} />
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
    const result = []

    for (let i = dataList.length - 1; i > 0; i -= 1) {
      const item = this.renderItem(dataList[i - 1], dataList[i])
      result.push(item)
    }
    return <List>{result}</List>
  }

  renderItem = (dataOld, dataNew) => {
    const { _id, updatedAt } = dataNew
    const metaNode = (
      <div>
        <span style={{ marginRight: 5 }}>{I18N.get('suggestion.edited')}</span>
        <span>{moment(updatedAt).format('MMM D, YYYY')}</span>
      </div>
    )
    return (
      <Item key={_id}>
        {metaNode}
        <Title><Diff inputA={dataOld.title} inputB={dataNew.title} type="words" /></Title>
        <Diff inputA={dataOld.desc} inputB={dataNew.desc} type="words" />
        {/* <Desc dangerouslySetInnerHTML={{ __html: <Diff inputA={dataOld.desc} inputB={dataNew.desc} type="words" /> }} /> */}
      </Item>
    )
  }

  refetch = async () => {
    const { match, resetDetail, getDetail } = this.props
    const id = _.get(match, 'params.id')
    await resetDetail()
    getDetail({ id, incViewsNum: false })
  }

}
