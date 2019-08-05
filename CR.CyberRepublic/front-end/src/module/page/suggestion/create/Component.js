import React from 'react'
import MediaQuery from 'react-responsive'
import Footer from '@/module/layout/Footer/Container'
import BackLink from '@/module/shared/BackLink/Component'
import SuggestionForm from '@/module/form/SuggestionForm/Component'
import I18N from '@/I18N'
import { LG_WIDTH } from '@/config/constant'
import Meta from '@/module/common/Meta'
import StandardPage from '../../StandardPage'

import { Container, Title } from './style'
import './style.scss'

const LOCALSTORAGE_DRAFT = 'draft-suggestion';

export default class extends StandardPage {
  constructor(props) {
    super(props)

    const draftSuggestion = localStorage.getItem(LOCALSTORAGE_DRAFT);
    this.state = {
      error: null,
      draftSuggestion: draftSuggestion ? JSON.parse(draftSuggestion) : {}
    }
  }

  historyBack = () => {
    this.props.history.push('/suggestion')
  }

  onSubmit = (model) => {
    return this.props.createSuggestion(model)
      .then(() => this.historyBack())
      .then(() => localStorage.removeItem(LOCALSTORAGE_DRAFT))
      .catch(err => this.setState({ error: err }))
  }

  onSaveDraft = (model) => {
    localStorage.setItem(LOCALSTORAGE_DRAFT, JSON.stringify(model));
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
