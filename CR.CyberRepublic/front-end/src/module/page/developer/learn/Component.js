import React from 'react'
import Footer from '@/module/layout/Footer/Container'
import './style.scss'

import MediaQuery from 'react-responsive'
import { MAX_WIDTH_MOBILE, MIN_WIDTH_PC } from '@/config/constant'

import { Col, Row, Tabs, Tree, Divider } from 'antd'

import StandardPage from '../../StandardPage'

// /////////////////
// Content
// /////////////////
import IntroductionDetail from './detail/GettingStarted/Introduction/Component'
import BasicsDetail from './detail/GettingStarted/Basics/Component'
import BlockchainArchitectureDetail from './detail/GettingStarted/BlockchainArchitecture/Component'
import ComponentAssemblyRuntimeDetail from './detail/GettingStarted/ComponentAssemblyRuntime/Component'
import ResourcesDetail from './detail/GettingStarted/Resources/Component'

import MiningTestnetWalletDemoDetail from './detail/Tutorials/MiningTestnetWalletDemo/Component'
import CarrierChatAppDetail from './detail/Tutorials/CarrierDemo/Component'

import BlockchainSmartContractsDetail from './detail/TheFourPillars/BlockchainSmartContracts/Component'
import ElastosCarrierDetail from './detail/TheFourPillars/ElastosCarrier/Component'
import ElastosRuntimeDetail from './detail/TheFourPillars/ElastosRuntime/Component'
import ElastosSDKDetail from './detail/TheFourPillars/ElastosSDK/Component'

import EndToEndDemoDApp from './detail/SmartwebAlpha/EndToEndDemoDApp/Component'

// /////////////////
// Constant
// /////////////////

const TabPane = Tabs.TabPane
const TreeNode = Tree.TreeNode

// /////////////////
// Data: Dummy
// /////////////////
const DEFAULT_ID = '1-1'
const TABS = [
  { id: 1, title: 'Getting Started' },
  { id: 2, title: 'Tutorials' },
  { id: 3, title: 'Smart Contracts' },
  { id: 4, title: 'The Four Pillars' },
  { id: 5, title: 'Smartweb Alpha' }
]
const CONTENTS = [
  {
    tabID: 1,
    content: [
      {
        id: 1,
        topic: 'Introduction',
        detail: IntroductionDetail,
        children: []
      },
      {
        id: 2,
        topic: 'Basics',
        detail: BasicsDetail,
        children: []
      },
      {
        id: 3,
        topic: 'Blockchain Architecture',
        detail: BlockchainArchitectureDetail,
        children: []
      },
      {
        id: 4,
        topic: 'Component Assembly Runtime (CAR)',
        detail: ComponentAssemblyRuntimeDetail,
        children: []
      },
      {
        id: 5,
        topic: 'Resources',
        detail: ResourcesDetail,
        children: []
      }

    ]
  },
  {
    tabID: 2,
    content: [
      {
        id: 1,
        topic: 'Elastos Mining, Testnet and Wallet Demo',
        detail: MiningTestnetWalletDemoDetail,
        children: []
      },
      {
        id: 2,
        topic: 'Elastos Carrier Chat App Demo',
        detail: CarrierChatAppDetail,
        children: []
      }
    ]
  },
  {
    tabID: 3,
    content: [
      {
        id: 1,
        topic: 'Smart Contracts',
        detail: IntroductionDetail,
        children: []
      }
    ]
  },
  {
    tabID: 4,
    content: [
      {
        id: 1,
        topic: 'Blockchain and Smart Contracts',
        detail: BlockchainSmartContractsDetail,
        children: []
      },
      {
        id: 2,
        topic: 'Elastos Carrier',
        detail: ElastosCarrierDetail,
        children: []
      },
      {
        id: 3,
        topic: 'Elastos Runtime',
        detail: ElastosRuntimeDetail,
        children: []
      },
      {
        id: 4,
        topic: 'Elastos SDK',
        detail: ElastosSDKDetail,
        children: []
      }
    ]
  },
  {
    tabID: 5,
    content: [
      {
        id: 1,
        topic: 'End to End Demo dApp',
        detail: EndToEndDemoDApp,
        children: []
      }
    ]
  }
]

// /////////////////
// Function:
// /////////////////

const isEmpty = ([first, ...rest]) => first === undefined

// TopicDetail: displays the detail of a topic
class TopicDetail extends React.Component {

  getTopicContent() {
    const selectedKey = parseInt((this.props.selectedTopic || DEFAULT_ID).split('-').pop())
    const contents = this.props.paneContent ? this.props.paneContent.content : []
    const topicContent = this.getTopicChild(contents, selectedKey)
    return topicContent ? topicContent.detail : null
  }

  getTopicChild(children, id) {
    let result = null

    children.some((child, key) => {
      if (child.id === id) {
        result = child
      } else if (child.children) {
        result = this.getTopicChild(child.children, id)
      }
      if (result) {
        return true
      }
    })
    return result
  }

  render() {
    const Content = this.getTopicContent()
    return (
      <div>
        <article>
          <Content />
        </article>
        <Divider />
        <footer>
          <Row>
            <Col span={12}><a href="#" title="Submit feedback.">Help us improve this page.</a></Col>
            <Col span={12}>Last Updated: 08/12/2018</Col>
          </Row>
          <Divider />
          <Row>
            <Col span={24}><a href="#" title="Next topic">Next topic</a></Col>
          </Row>
        </footer>
      </div>
    )
  }
}

// Topics: produces cateogries and topics
class Topics extends React.Component {
  constructor(p) {
    super(p)
    this.p = p
  }

  buildTreeNodes(paneContent) {

    // consumes a node data
    // produces a leafnode
    const buildLeafNode = (n) => (<TreeNode title={n.topic} key={`${paneContent.tabID}-${n.id}`} />)

    const buildNodeNode = (n) => {
      const childNodes = n.children.map(el => buildLeafNode(el))
      return (
        <TreeNode title={n.topic} key={n.id}>
          {childNodes}
        </TreeNode>
      )
    }

    return paneContent.content.map(el => {
      return isEmpty(el.children) ? buildLeafNode(el) : buildNodeNode(el)
    })

  }

  render() {
    const paneContent = this.p.paneContent
    return (
      <div>
        <Tree defaultSelectedKeys={[DEFAULT_ID]} onSelect={this.p.onTopicSelect} showLine={true} defaultExpandAll={true} >
          {this.buildTreeNodes(paneContent)}
        </Tree>
      </div>
    )
  }
}

// LearningContent: produces layout for each tab
// TODO: add condition to only load conents for tabs
//       that already have content.
class LearningContent extends React.Component {

  constructor(p) {
    super(p)
    this.p = p
    this.state = {
      selectedTopic: DEFAULT_ID
    }
  }

  // getContentByTabID: consumes tabID, content
  // produces the content for tabID
  getContentByTabID(tabID, contents) {
    return contents.find(el => el.tabID === tabID)
  }

    onTopicSelect = (selectionInfo, b, c, d) => {
      const selection = selectionInfo && selectionInfo.length > 0 ? selectionInfo[0] : ''
      if (selection) {
        this.setState({selectedTopic: selection})
      }
    }

    render() {
      const paneContent = this.getContentByTabID(this.p.tab.id, CONTENTS)
      return (
        <Row gutter={16}>
          <MediaQuery minWidth={MIN_WIDTH_PC}>
            <Col span={8}><Topics paneContent={paneContent} onTopicSelect={this.onTopicSelect} /></Col>
            <Col span={16}><TopicDetail paneContent={paneContent} selectedTopic={this.state.selectedTopic} /></Col>
          </MediaQuery>
          <MediaQuery maxWidth={MAX_WIDTH_MOBILE}>
            <Col span={24}><Topics paneContent={paneContent} onTopicSelect={this.onTopicSelect} /></Col>
            <Col span={24}><TopicDetail paneContent={paneContent} selectedTopic={this.state.selectedTopic} /></Col>
          </MediaQuery>
        </Row>
      )
    }
}

// OptionTab: produces tabs with LearningContent on tab
class OptionTab extends React.Component {

  constructor(p) {
    super(p)
    this.p = p
  }

  // buildTab consumes a list of tab-setting objects;
  // it produces a list of TabPanes
  buildTab(tabs) {
    return tabs.map((el, i = 0) => (
      <TabPane tab={el.title} key={(i + 1).toString()}>
        <LearningContent tab={el} />
      </TabPane>
    ))
  }

  render() {
    return (
      <Tabs defaultActiveKey={this.p.defaultActiveKey} type="card">
        {this.buildTab(this.p.tabs)}
      </Tabs>
    )
  }
}

// /////////////////
// Function: main
// /////////////////
export default class extends StandardPage {
    state = {
    }

    ord_renderContent() {
      return (
        <div className="p_DeveloperLearn">
          <div className="p_admin_index ebp-wrap">
            <div className="d_box">
              <div className="p_admin_content">
                <Row className="d_row d_rowTop">
                  <Col span={24}>
                    <OptionTab tabs={TABS} defaultActiveKey="1" />
                  </Col>
                </Row>
              </div>
            </div>
          </div>
          <Footer />
        </div>
      )
    }
}

// /////////////////
// Test
// /////////////////
