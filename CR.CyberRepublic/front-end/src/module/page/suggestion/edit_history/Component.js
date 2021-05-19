import React from 'react'
import _ from 'lodash'
import { Spin, Layout, Button } from 'antd'
import moment from 'moment/moment'
import I18N from '@/I18N'
import BackLink from '@/module/shared/BackLink/Component'
import styled from 'styled-components'
import difflib from 'difflib'
import StandardPage from '../../StandardPage'
import Diff from './Diff'
import DetailPage from '../detail/Component'
import MetaContainer from '../common/historymeta/Container'
import { Container, Header, Title, List, Item } from './style'
import './style.scss'

export default class extends DetailPage {
  constructor() {
    super()
    this.state = {
      version: 0
    }
  }

  componentDidMount() {
    this.refetch()
  }

  componentWillUnmount() {
    this.props.resetEditHistory()
  }

  diffObject(obj1, obj2) {
    if (!obj1) return null
    if (!obj2) return obj1
    const diffObj = Object.assign(new obj1.constructor(), obj1)
    let f
    let str
    let myobj = obj1
    if (obj2.length > obj1.length) {
      myobj = obj2
    }
    for (const k in myobj) {
      if (obj1.hasOwnProperty(k) && obj2.hasOwnProperty(k)) {
        if ((typeof obj1[k]) === (typeof obj2[k])) {
          if (typeof obj1[k] === 'object') {
            if (k === 'milestone'
            ) {
              diffObj[k] = obj2[k]
            } else {
              diffObj[k] = this.diffObject(obj1[k], obj2[k])
            }
          } else if (typeof obj1[k] === 'string') {
            if (k === '_id'
                || k === 'title'
                || k === 'createdAt'
                || k === 'updatedAt'
                || k === 'elaAddress'
                || k === 'type'
                || k === 'amount'
                || k === 'contentType'
                || k === 'milestoneKey'
                || k === 'date'
                || k === 'member'
                || k === 'role'
                || k === 'reference'
                || k === 'comments'
                || k === 'dislikes'
                || k === 'likes'
                || k === 'link'
                || k === 'subscribers'
                || k === 'tags'
            ) {
              diffObj[k] = obj2[k]
            } else {
              const da = difflib.ndiff(obj1[k].split(''), obj2[k].split(''))
              f = ' '
              str = ''
              for (let i = 0; i < da.length; i++) {
                const c0 = da[i][0]
                const c2 = da[i][2]
                f = c0
                if (f === '-') {
                  str += '<span style=\'background-color: rgba(252, 192, 192, 0.2)\'>'
                } else if (f === '+') {
                  str += '<span style=\'background-color: #1DE9B633\'>'
                }
                str += c2
                if (f !== ' ') {
                  str += '</span>'
                }
              }
              diffObj[k] = str
            }
          } else {
            diffObj[k] = obj2[k]
          }
        } else {
          diffObj[k] = obj2[k]
        }
      } else {
        diffObj[k] = obj2[k]
      }
    }
    return diffObj
  }

  ord_renderContent() {
    const { dataList, match, loading, detail } = this.props
    const {version} = detail
    const id = _.get(match, 'params.id')
    let content
    if (loading) {
      content = <div className="center"><Spin size="large" /></div>
    } else if (_.isEmpty(dataList)) {
      content = null
    } else if (!_.isEmpty(dataList)) {
      content = this.renderList()
    }
    if (!this.state.version && version > 0) {
      this.setState({version})
    }
    if (!this.state.version
        || !dataList
        || dataList.length === 0) {
      return content
    }

    const currDetail = dataList.find((e) => {
      if (e.version === version) {
        return e
      }
      return null
    })

    const selectedDetail = dataList.find((e) => {
      if (e.version === this.state.version) {
        return e
      }
      return null
    })

    const diffDetail = this.diffObject(currDetail, selectedDetail)

    content = this.renderDetail(diffDetail)

    const headerNode = this.renderHeader()
    const historyList = this.renderHistoryList()
    return (
      <Layout>
        <Layout.Content>
          <Container>
            <BackLink link={`/suggestion/${id}`} />
            {content}
          </Container>
        </Layout.Content>
        <Layout.Sider width="345">
          <Siderbar>
            {headerNode}
            {historyList}
          </Siderbar>
        </Layout.Sider>
      </Layout>
    )
  }

  renderTitleButton = () => {
    const { detail, currentUserId, isAdmin } = this.props
    const signature = _.get(detail, 'signature.data')
    const isOwner = currentUserId === _.get(detail, 'createdBy._id') || isAdmin
    return !signature && isOwner && (
      <Button
        onClick={this.handleRevertVersion}
        className="btn-create-suggestion"
        htmlType="button"
        style={{ marginRight: 10 }}
      >
        {I18N.get('suggestion.form.button.revertVersion')}
      </Button>
    )
  }

  renderMetaNode() {
    const { dataList, detail, user } = this.props
    let {version} = detail
    if (version < 10) version = 10
    if (this.state.version) version = this.state.version
    const sDetail = dataList.find((e) => {
      if (e.version === version) {
        return e
      }
    })
    return sDetail && (
      <MetaContainer
        data={sDetail}
        user={user}
        content={this.renderTitleButton()}
      />
    )
  }

  renderHeader() {
    return (
      <Header className="historyTitle">{this.props.header || I18N.get('suggestion.editHistory')}</Header>
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
        <span>{moment(updatedAt).format('MMM D, YYYY HH:mm A')}</span>
      </div>
    )
    return (
      <Item key={_id}>
        {metaNode}
        <Title><Diff inputA={dataOld.title} inputB={dataNew.title} type="words" /></Title>
        <Diff inputA={dataOld.desc} inputB={dataNew.desc} type="words" />
      </Item>
    )
  }

  renderHistoryList() {
    const { dataList, detail: { version } } = this.props

    return (
      <HistoryList className="historyList">
        {dataList && dataList.map((item) => (
          <div key={item.version}
               className={`versionItem ${item.version === this.state.version ? 'versionSelected' : ''}`}
               onClick={(e) => {
                 this.setState({version: item.version})
               }}
          >
            <div/>
            <VersionValue>
              Version
              {' '}
              {item.version / 10}
              {item.version % 10 === 0 ? '.0' : ''}
              {' '}
              <VersionTag>{item.version === version ? 'Current version' : null}</VersionTag>
            </VersionValue>
            <VersionTime>{moment(item.createdAt).format('MMM D, YYYY HH:mm A')}</VersionTime>
          </div>
        ))}
      </HistoryList>
    )
  }

  handleRevertVersion = () => {
    const { match, revertVersion } = this.props
    const id = _.get(match, 'params.id')
    revertVersion(id, this.state.version)
  }

  refetch = async () => {
    const { match, resetEditHistory, getEditHistories } = this.props
    const id = _.get(match, 'params.id')
    resetEditHistory()
    getEditHistories({ id })
  }
}

const Siderbar = styled.div`
  display: block;
  height: 100%;
  background-color: #FFF;
  box-shadow: 0px 0px 20px rgba(0, 0, 0, 0.1);
`

const HistoryList = styled.div`
  height: 100%;
  background-color: #FFF
`
const VersionValue = styled.p`
  padding: 0;
  font-family: Synthese;
  font-size: 17px;
  line-height: 28px;
`

const VersionTag = styled.i`
  font-size: 14px;
  font-weight: normal;
`

const VersionTime = styled.p`
  padding: 0;
  font-family: Synthese;
  font-size: 14px;
  line-height: 28px;
`
