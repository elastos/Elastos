import React from 'react'
import I18N from '@/I18N'
import { Collapse } from 'antd'
import StandardPage from '../StandardPage'
import Footer from '@/module/layout/Footer/Container'
import './styles.scss'

const WHITEPAPER_KEYS = ['abstract', '1', '2', '3', '4', '5', '6', '7', '8', '9', '10', 'A', 'B'];

export default class extends StandardPage {
  ord_renderContent() {
    return (
      <div>
        <div className="whitepaper">
          <div class="title komu-a cr-title-with-icon">{I18N.get('navigation.whitepaper')}</div>
          <Collapse defaultActiveKey={['abstract']} bordered={false} expandIconPosition="right" expandIcon={() => (
            <img src="/assets/images/whitepaper-arrow.svg" className="arrow" width="26" height="15" />
          )}>
            {WHITEPAPER_KEYS.map(k => (
              <Collapse.Panel header={I18N.get(`whitepaper.${k}.title`)} key={k}>
                <div dangerouslySetInnerHTML={{__html: I18N.get(`whitepaper.${k}.body`)}} />
              </Collapse.Panel>
            ))}
          </Collapse>
        </div>
        <Footer />
      </div>
    )
  }
}
