import React, { Suspense } from 'react'
import ReactDOM from 'react-dom'
import { Helmet } from 'react-helmet'
import _ from 'lodash'
import { Route, Switch } from 'react-router-dom'
import { Provider } from 'react-redux'
import { ConnectedRouter } from 'react-router-redux'
import store from '@/store'
import config from '@/config'
import PageLoading from '@/module/common/PageLoading'
import { api_request, permissions } from './util'

import './boot'
import './style/antd/index.less'
import './style/index.scss'
import './style/mobile.scss'

const middleware = (render, props) => render

/**
 * This is the entry point, we do the following in order
 *
 * 1. Always fetch /api/user/current_user at init, this sets the user data from the token and a DB call
 * 2. Then render the App to ebp-root
 * @returns {*}
 * @constructor
 */
const App = (props) => (
  <div>
    <Helmet>
      <meta name="cr-env" content={process.env.NODE_ENV}/>
      <meta
      name="cr-version-number"
      content={
        process.env.CR_VERSION ? `${process.env.CR_VERSION}` : 'unknown'
      }
      />
      {process.env.NODE_ENV === 'production' && (
      <script defer={true} src="/assets/js/rollbar_prod.js"/>
      )}
      {process.env.NODE_ENV === 'staging' && (
      <script defer={true} src="/assets/js/rollbar_staging.js"/>
      )}
      {process.env.NODE_ENV === 'production' && (
      <script
      async={true}
      src={`https://www.googletagmanager.com/gtag/js?id=${process.env.GA_ID}`}
      />
      )}
      {process.env.NODE_ENV === 'production' && (
      <script>{`window.dataLayer = window.dataLayer || []; function gtag(){dataLayer.push(arguments);} gtag('js', new Date()); gtag('config', '${process.env.GA_ID}');`}</script>
      )}
      {window.location.pathname === '/' && (
      <script defer={true} src="/assets/js/elastos.js"/>
      )}
    </Helmet>
    {props.maintenanceMode ? (
      <div className="maintenance-mode">

        <img src="/assets/images/logo.svg" alt="Cyber Republic" width="20%"/>

        <h3>Maintenance Mode</h3>

      Sorry our website is currently down due to maintenance.

      </div>
    ) : (
      <Suspense fallback={<PageLoading />}>
        <Switch id="ebp-main">
          {_.map(config.router, (item, i) => {
            const props = _.omit(item, ['page', 'path', 'type'])
            const R = item.type || Route
            return (
              <R
                path={item.path}
                key={i}
                exact={true}
                component={item.page}
                {...props}
                />
            )
          })}
        </Switch>
      </Suspense>
    )}
  </div>
)


const render = (maintenanceMode = false) => {

  ReactDOM.render(
    <Provider store={store}>
      <ConnectedRouter middleware={middleware} history={store.history}>
        <App maintenanceMode={maintenanceMode}/>
      </ConnectedRouter>
    </Provider>,
    document.getElementById('ebp-root')
  )
}

if (localStorage.getItem('api-token') && !sessionStorage.getItem('api-token')) {
  sessionStorage.setItem('api-token', localStorage.getItem('api-token'))
}

if (sessionStorage.getItem('api-token')) {
  const userRedux = store.getRedux('user')

  try {
    api_request({
      path: '/api/user/current_user',
      success: data => {
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
        store.dispatch(userRedux.actions.did_update(data.did))
        store.dispatch(userRedux.actions.email_update(data.email))
        store.dispatch(userRedux.actions.username_update(data.username))
        store.dispatch(userRedux.actions.profile_update(data.profile))
        store.dispatch(userRedux.actions.role_update(data.role))
        store.dispatch(userRedux.actions.current_user_id_update(data._id))
        store.dispatch(userRedux.actions.circles_update(_.values(data.circles)))
        store.dispatch(
          userRedux.actions.subscribers_update(_.values(data.subscribers))
        )
        store.dispatch(userRedux.actions.loading_update(false))

        // Segment - pass identify
        const userProfile = data.profile

        // eslint-disable-next-line no-undef
        analytics.identify(data._id, {
          id: data._id,
          title: data.role,
          gender: userProfile.gender,
          createdAt: data.createdAt
        })

        render()
      },
      error: () => {
        sessionStorage.clear()
        localStorage.removeItem('api-token')

        render()
      }
    }).catch((err) => {
      // Promise catch
      // HACK - we don't have a formal maintenance mode yet, but if current_user fails we render

      render(true)
    })
  } catch (err) {
    render(true)
  }

} else {
  render()
}
