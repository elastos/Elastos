import React from 'react'
import Footer from '@/module/layout/Footer/Container'
import './style.scss'
import { Row } from 'antd'
import StandardPage from '../../StandardPage'

export default class extends StandardPage {

  ord_renderContent () {
    return (
      <div className="p_About">
        <div className="ebp-header-divider" />

        <div className="ebp-page-title">
          <Row className="d_row d_rowGrey">
            <h3 className="page-header">
                            About Us
            </h3>
          </Row>
        </div>
        <div className="ebp-page">
          <Row className="d_row">
            <h4>
                            We are a diverse democratic group of leaders, developers, organizers and designers
              <br/>
                            formed to promote Elastos in our communities. Membership is open to everyone.
            </h4>

            <p>
                            Although everyone is welcome to join this site is primarily targeted towards contributors and developers.
            </p>

            <b>Guides:</b>

            <ul>
              <li>
                                Beginner's Guide:
                {' '}
                <a target="_blank" href="https://www.reddit.com/r/Elastos/comments/7xihw9/elastos_a_beginners_guide">https://www.reddit.com/r/Elastos/comments/7xihw9/elastos_a_beginners_guide</a>
              </li>
            </ul>

            <b>Resources:</b>

            <ul>
              <li>
Official Website:
                <a target="_blank" href="https://www.elastos.org">https://www.elastos.org</a>
              </li>
              <li>
Elastos News:
                <a target="_blank" href="https://elanews.net">https://elanews.net</a>
              </li>
              <li>
Roadmap:
                <a target="_blank" href="https://www.elastos.org/roadmap">https://www.elastos.org/roadmap</a>
              </li>
              <li>
Elastos Carrier:
                <a target="_blank" href="https://medium.com/elastos/elastos-carrier-explanation-development-status-b468199c1aa4?source=linkShare-7e59912129a9-1521164555">Medium Elastos Carrier Article</a>
              </li>
              <li>
Elastos Wallet:
                <a target="_blank" href="https://wallet.elastos.org">https://wallet.elastos.org</a>
              </li>
              <li>
Wallet FAQ:
                <a target="_blank" href="https://medium.com/@elastos/elastos-wallet-faq-f717291fd7ae">https://medium.com/@elastos/elastos-wallet-faq-f717291fd7ae</a>
              </li>
              <li>
Using the Wallet:
                <a target="_blank" href="https://medium.com/@elastos/the-elastos-wallet-68797064d8dd">https://medium.com/@elastos/the-elastos-wallet-68797064d8dd</a>
              </li>
              <li>
Blockchain Explorer:
                <a target="_blank" href="https://blockchain.elastos.org/blocks">https://blockchain.elastos.org/blocks</a>
              </li>
              <li>
GitHub:
                <a target="_blank" href="https://github.com/elastos">https://github.com/elastos</a>
              </li>
              <li>
Telegram Official News:
                <a target="_blank" href="https://t.me/elastos_org">https://t.me/elastos_org</a>
              </li>
              <li>
Telegram Community:
                <a target="_blank" href="https://t.me/elastosgroup">https://t.me/elastosgroup</a>
              </li>
            </ul>
          </Row>
        </div>
        <Footer/>
      </div>
    )
  }
}
