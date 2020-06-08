import React from 'react'
import _ from 'lodash'
import { Spin } from 'antd'
import MediaQuery from 'react-responsive'
import Footer from '@/module/layout/Footer/Container'
import BackLink from '@/module/shared/BackLink/Component'
import SuggestionForm from '@/module/form/SuggestionForm/Component'
import I18N from '@/I18N'
import { LG_WIDTH } from '@/config/constant'
import StandardPage from '../../StandardPage'
import Meta from '@/module/common/Meta'

import { Container } from './style'

import './style.scss'

export default class extends StandardPage {
  constructor(props) {
    super(props)

    this.state = {
      data: null,
      loading: true,
      error: null
    }
  }

  async componentDidMount() {
    super.componentDidMount()
    const { getDetail, getDraft } = this.props
    const id = _.get(this.props, 'match.params.id')
    const detail = await getDetail(id)
    const draft = await getDraft(id)
    let data
    if (draft && draft.empty) {
      data = detail
    } else {
      data = draft
    }
    this.setState({data, loading: false})
  }

  historyBack = () => {
    const id = this.state.data._id
    this.props.history.push(`/suggestion/${id}`)
  }

  onSubmit = (model) => {
    const id = this.state.data._id
    return this.props.updateSuggestion({ id, ...model, update: true })
      .then(() => this.historyBack())
      .catch(err => this.setState({ error: err }))
  }

  onSaveDraft = async (model) => {
    const id = this.state.data._id
    const rs = await this.props.saveDraft({ id, ...model })
    if (rs) {
      this.historyBack()
    }
  }

  ord_renderContent() {
    if (this.state.loading) {
      return (
        <div className="center">
          <Spin size="large" />
        </div>
      )
    }
    if (_.get(this.state.data, 'signature.data')) {
      return this.historyBack()
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
                link={`/suggestion/${_.get(this.props, 'match.params.id')}`}
                style={{ position: 'relative', left: 0, marginBottom: 15 }}
              />
            </div>
          </MediaQuery>
          <MediaQuery minWidth={LG_WIDTH + 1}>
            <BackLink link="/suggestion" />
          </MediaQuery>

          <div>
            <h2 className="komu-a cr-title-with-icon ">
              {I18N.get('suggestion.title.edit')}
            </h2>
            <SuggestionForm
              isEditMode={true}
              lang={this.props.lang}
              initialValues={this.state.data}
              onSubmit={this.onSubmit}
              onCancel={this.historyBack}
              onSaveDraft={this.onSaveDraft}
            />
          </div>
        </Container>
        <Footer />
      </div>
    )
  }

}
