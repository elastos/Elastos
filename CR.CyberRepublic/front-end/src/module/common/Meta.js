import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import { Helmet } from 'react-helmet'

export default class extends BaseComponent {
  ord_render() {
    return (
      <Helmet>
        <meta name="description" content="Cyber Republic is a global, citizen led decentralised community helping build on the new internet on the secure Elastos Smartweb" />
        {/* <!-- facebook and twitter --> */}
        <meta property="og:title" content="Cyber Republic - Elastos" />
        <meta property="og:description" content="Cyber Republic is a global, citizen led decentralised community helping build on the new internet on the secure Elastos Smartweb" />
        <meta property="og:image" content="https://www.cyberrepublic.org/assets/images/cr_landing.png" />
        <meta property="og:url" content="https://cyberrepublic.org" />
        <meta name="twitter:card" content="Cyber Republic is a global, citizen led decentralised community helping build on the new internet on the secure Elastos Smartweb" />
        <meta name="twitter:image:alt" content="Cyber Republic Logo" />
        <meta property="og:site_name" content="Cyber Republic" />
      </Helmet>
    )
  }
}
