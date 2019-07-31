import React from 'react'
import ReactDOMServer from 'react-dom/server'
import { Helmet } from 'react-helmet'
import _ from 'lodash'
import { Row, Col, Spin, Divider, Modal, Input, Button } from 'antd'
import { Link } from 'react-router-dom'
import MediaQuery from 'react-responsive'
import moment from 'moment/moment'
import Comments from '@/module/common/comments/Container'
import Footer from '@/module/layout/Footer/Container'
import BackLink from '@/module/shared/BackLink/Component'
import Translation from '@/module/common/Translation/Container'
import SuggestionForm from '@/module/form/SuggestionForm/Component'
import ProposalForm from '@/module/page/CVote/create/Container'
import I18N from '@/I18N'
import { LG_WIDTH } from '@/config/constant'
import { CVOTE_STATUS, SUGGESTION_TAG_TYPE } from '@/constant'
import { getSafeUrl } from '@/util/url'
import sanitizeHtml from '@/util/html'
import { ReactComponent as CommentIcon } from '@/assets/images/icon-info.svg'
import StandardPage from '../../StandardPage'
import ActionsContainer from '../common/actions/Container'
import MetaContainer from '../common/meta/Container'
import Meta from '@/module/common/Meta'

import {
  Container,
  Title,
  CoverImg,
  ShortDesc,
  DescLabel,
  Label,
  LabelPointer,
  Desc,
  BtnGroup,
  StyledButton,
  DescBody,
  CouncilComments,
  IconWrap
} from './style'

import './style.scss'

const { TextArea } = Input

export default class extends StandardPage {
  constructor(props) {
    super(props)

    this.state = {
      data: null,
      loading: true,
      error: null
    }
  }

  componentDidMount() {
    super.componentDidMount()
    this.props.getDetail(_.get(this.props, 'match.params.id'))
      .then(data => this.setState({ data, loading: false }))
      .catch(err => this.setState({ error: err, loading: false }))
  }

  historyBack = () => {
    const id = this.state.data._id
    this.props.history.push(`/suggestion/${id}`)
  }

  onSubmit = (model) => {
    const id = this.state.data._id
    return this.props.updateSuggestion({ id, ...model })
      .then(() => this.historyBack())
      .catch(err => this.setState({ error: err }))
  }

  ord_renderContent() {
    if (this.state.loading) {
      return (
        <div className="center">
          <Spin size="large" />
        </div>
      )
    }

    return (
      <div>
        <Meta
          title="Edit Suggestion Detail - Cyber Republic"
          url={this.props.location.pathname}
        />

        <Container className="c_SuggestionDetail">
          <MediaQuery maxWidth={LG_WIDTH}>
            <div>
              <BackLink
                link="/suggestion"
                style={{ position: 'relative', left: 0, marginBottom: 15 }}
              />
            </div>
          </MediaQuery>
          <MediaQuery minWidth={LG_WIDTH + 1}>
            <BackLink link="/suggestion" />
          </MediaQuery>

          <div>
            <Title className="komu-a cr-title-with-icon ">
              {I18N.get('from.CVoteForm.button.add')}
            </Title>
            <SuggestionForm
              initialValues={this.state.data}
              onSubmit={this.onSubmit}
              onCancel={this.historyBack}
            />
          </div>
        </Container>
        <Footer />
      </div>
    )
  }

}
