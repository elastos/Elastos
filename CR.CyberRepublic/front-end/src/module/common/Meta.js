import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import { Helmet } from 'react-helmet'

export default class extends BaseComponent {
  ord_render() {
    const { desc, title, url } = this.props

    let host = 'https://cyberrepublic.org'
    switch (process.env.NODE_ENV) {
      case 'development':
        host = 'http://localhost:3001'
        break
      case 'staging':
        host = 'https://staging.cyberrepublic.org'
        break
      default:
        break
    }

    const slogan =
      'Cyber Republic is a global, ' +
      'citizen led decentralised community helping build on the new internet on the secure Elastos Smartweb'

    const description = desc || slogan

    const meta = [
      {
        name: 'description',
        content: description
      },
      {
        property: 'og:title',
        content: title || 'Cyber Republic - Elastos'
      },
      {
        property: 'og:description',
        content: description
      },
      {
        property: 'og:image',
        content: 'https://www.cyberrepublic.org/assets/images/cr_landing.png'
      },
      { property: 'og:url', content: url ? `${host}${url}` : host },
      { property: 'og:site_name', content: 'Cyber Republic' },
      { property: 'twitter:image:alt', content: 'Cyber Republic Logo' },
      { name: 'twitter:card', content: description }
    ]

    return <Helmet title={title} meta={meta} />
  }
}
