import React from 'react'
import MediaQuery from 'react-responsive'
import Footer from '@/module/layout/Footer/Container'
import BackLink from '@/module/shared/BackLink/Component'
import SuggestionForm from '@/module/form/SuggestionForm/Component'
import I18N from '@/I18N'
import { LG_WIDTH } from '@/config/constant'
import Meta from '@/module/common/Meta'
import StandardPage from '../../StandardPage'

import { Container } from './style'
import './style.scss'

const LOCALSTORAGE_DRAFT = 'draft-suggestion'

export default class extends StandardPage {
  constructor(props) {
    super(props)
    let draftSuggestion = localStorage.getItem(LOCALSTORAGE_DRAFT)
    if (draftSuggestion) {
      const rs = JSON.parse(draftSuggestion)
      // deal with old budget data
      if (rs.budget && typeof rs.budget === 'string') {
        delete rs.budget
      }
      if (rs.budget && typeof rs.plan === 'string') {
        delete rs.plan
      }
      draftSuggestion = rs
    } else {
      draftSuggestion = {}
    }

    this.state = {
      error: null,
      draftSuggestion
    }
  }

  historyBack = (id) => {
    this.props.history.push(`/suggestion/${id}?new=true`)
  }

  onSubmit = async (model) => {
    const rs = await this.props.createSuggestion(model)
    this.historyBack(rs && rs._id)
    localStorage.removeItem(LOCALSTORAGE_DRAFT)
  }

  onSaveDraft = (model) => {
    localStorage.setItem(LOCALSTORAGE_DRAFT, JSON.stringify(model))
  }

  ord_renderContent() {
    return (
      <div>
        <Meta
          title="Add Suggestion Detail - Cyber Republic"
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
            <h2 className="komu-a cr-title-with-icon">
              {I18N.get('suggestion.title.add')}
            </h2>
            <SuggestionForm
              lang={this.props.lang}
              initialValues={this.state.draftSuggestion}
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
