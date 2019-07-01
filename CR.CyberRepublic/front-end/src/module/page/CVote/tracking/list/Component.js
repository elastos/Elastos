import React from 'react'
import _ from 'lodash'
import moment from 'moment/moment'
import BaseComponent from '@/model/BaseComponent'
import DraftEditor from '@/module/common/DraftEditor'
import {
  Table, Row, Col, Button, Input, List
} from 'antd'
import I18N from '@/I18N'
import { Link } from 'react-router-dom'
import { CONTENT_TYPE, DATE_FORMAT } from '@/constant'
import styled from 'styled-components'
import { breakPoint } from '@/constants/breakPoint'
import { bg } from '@/constants/color'

export default class extends BaseComponent {
  constructor(p) {
    super(p)

    this.state = {
      loading: true,
      publicList: undefined,
      privateList: undefined,
    }
  }

  async componentDidMount() {
    this.refetch()
  }

  ord_render() {
    const { canManage, isCouncil } = this.props
    const title = this.renderTitle()
    const publicListNode = this.renderPublicList()
    const privateListNode = this.renderPrivateList()
    return (
      <Container>
        {title}
        {publicListNode}
        {privateListNode}
      </Container>
    )
  }

  renderTitle() {
    return <ContentTitle id="tracking">{I18N.get('proposal.fields.tracking')}</ContentTitle>
  }

  renderPublicList() {
    const { publicList } = this.state
    if (!publicList) return null
    return (
      <List
        itemLayout="horizontal"
        grid={{ column: 1 }}
        dataSource={publicList}
        renderItem={item => (
          <StyledItem actions={[]}>
            <DraftEditor
              content={item.content}
              contentType={CONTENT_TYPE.MARKDOWN}
              editorEnabled={false}
            />
            <StyledDate>{moment(item.createdAt).format(DATE_FORMAT)}</StyledDate>
          </StyledItem>
        )}
      />
    )
  }

  renderPrivateList() {
    const { privateList } = this.state
    if (!privateList) return null
    return (
      <List
        itemLayout="horizontal"
        grid={{ column: 1 }}
        split={false}
        dataSource={privateList}
        renderItem={item => (
          <List.Item actions={[]}>
            <DraftEditor
              content={item.content}
              contentType={CONTENT_TYPE.MARKDOWN}
              editorEnabled={false}
            />
          </List.Item>
        )}
      />
    )
  }

  getQuery = () => {
    const query = {
      proposalId: _.get(this.props, 'proposal._id')
    }
    console.log('query: ', query)
    return query
  }

  refetch = async () => {
    this.ord_loading(true)
    const { listData, canManage, currentUserId, proposal } = this.props
    const param = this.getQuery()
    const isAuthorized = canManage || currentUserId === _.get(proposal, '_id')
    try {
      const publicData = await listData(param, false)
      this.setState({ publicList: publicData.list })
      if (isAuthorized) {
        const privateData = await listData(param, isAuthorized)
        this.setState({ privateList: privateData.list })
      }
    } catch (error) {
      // do sth
    }

    this.ord_loading(false)
  }
}

export const Container = styled.div`

`
export const StyledList = styled.div`

`
export const StyledItem = styled(List.Item)`
  background: rgba(29, 233, 182, 0.1);
  border: 1px solid rgba(0, 141, 133, 0.2)!important;
  margin: 10px auto;
  padding-left: 20px;
  .md-RichEditor-root {
    background: none;
    padding-left: 20px!important;
  }
`

export const ContentTitle = styled.h4`
  font-size: 20px;
  padding-bottom: 0;
`

export const StyledDate = styled.div`
  font-size: 12px;
  color: rgba(3, 30, 40, 0.4);
  padding: 10px 20px;
`
