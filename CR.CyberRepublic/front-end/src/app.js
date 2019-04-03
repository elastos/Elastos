import React from 'react'
import ReactDOM from 'react-dom'
import { Helmet } from 'react-helmet'
import _ from 'lodash'
import { Route, Switch } from 'react-router-dom'
import { Provider } from 'react-redux'
import { ConnectedRouter } from 'react-router-redux'
import store from '@/store'
import config from '@/config'
import { api_request, permissions } from './util'
import AutoLinks from 'quill-auto-links'
import { Quill } from 'react-quill'

import './boot'
import './style/index.scss'
import './style/mobile.scss'

Quill.register('modules/autoLinks', AutoLinks)

const middleware = (render, props) => render

const App = () => (
  <div>
    <Helmet>
      <meta name="cr-env" content={process.env.NODE_ENV} />
      <meta name="cr-version-number" content={process.env.CR_VERSION ? `${process.env.CR_VERSION}` : 'unknown'} />
      {process.env.NODE_ENV === 'production' && <script defer src="/assets/js/rollbar_prod.js" />}
      {process.env.NODE_ENV === 'staging' && <script defer src="/assets/js/rollbar_staging.js" />}
      {process.env.NODE_ENV === 'production' && <script async src={`https://www.googletagmanager.com/gtag/js?id=${process.env.GA_ID}`} />}
      {process.env.NODE_ENV === 'production' && <script>{`window.dataLayer = window.dataLayer || []; function gtag(){dataLayer.push(arguments);} gtag('js', new Date()); gtag('config', '${process.env.GA_ID}');`}</script>}
      {window.location.pathname === '/' && <script defer src="/assets/js/elastos.js" />}
      {/*
      <script>{
          (function() {
              window.Intercom("update");
          })()
      }</script>
      */}
    </Helmet>
    <Switch id="ebp-main">
      {
        _.map(config.router, (item, i) => {
          const props = _.omit(item, ['page', 'path', 'type'])
          const R = item.type || Route
          return (
            <R path={item.path} key={i} exact component={item.page} {...props} />
          )
        })
      }
    </Switch>
  </div>
)

const render = () => {
  ReactDOM.render(
    (
      <Provider store={store}>
        <ConnectedRouter middleware={middleware} history={store.history}>
          <App />
        </ConnectedRouter>
      </Provider>
    ),
    document.getElementById('ebp-root'),
  )
}

if (localStorage.getItem('api-token') && !sessionStorage.getItem('api-token')) {
  sessionStorage.setItem('api-token', localStorage.getItem('api-token'))
}

if (sessionStorage.getItem('api-token')) {
  const userRedux = store.getRedux('user')
  api_request({
    path: '/api/user/current_user',
    success: (data) => {
      // store user in redux
      const is_admin = permissions.isAdmin(data.role)
      const is_leader = permissions.isLeader(data.role)
      const is_council = permissions.isCouncil(data.role)
      const is_secretary = permissions.isSecretary(data.role)

      store.dispatch(userRedux.actions.is_leader_update(is_leader))
      store.dispatch(userRedux.actions.is_admin_update(is_admin))
      store.dispatch(userRedux.actions.is_council_update(is_council))
      store.dispatch(userRedux.actions.is_secretary_update(is_secretary))

      store.dispatch(userRedux.actions.is_login_update(true))
      store.dispatch(userRedux.actions.email_update(data.email))
      store.dispatch(userRedux.actions.username_update(data.username))
      store.dispatch(userRedux.actions.profile_update(data.profile))
      store.dispatch(userRedux.actions.role_update(data.role))
      store.dispatch(userRedux.actions.current_user_id_update(data._id))
      store.dispatch(userRedux.actions.circles_update(_.values(data.circles)))
      store.dispatch(userRedux.actions.subscribers_update(_.values(data.subscribers)))
      store.dispatch(userRedux.actions.loading_update(false))

      // Segment - pass identify
      const userProfile = data.profile

      analytics.identify(data._id, {
        id: data._id,
        title: data.role,
        gender: userProfile.gender,
        createdAt: data.createdAt,
      })

      render()
    },
    error: () => {
      sessionStorage.clear()
      localStorage.removeItem('api-token')
      render()
    },
  })
} else {
  render()
}
