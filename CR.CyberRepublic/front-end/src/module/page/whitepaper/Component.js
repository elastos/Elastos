import React from 'react'
import I18N from '@/I18N'
import { Collapse } from 'antd'
import StandardPage from '../StandardPage'
import Footer from '@/module/layout/Footer/Container'
import styled from 'styled-components'

const WHITEPAPER_KEYS = ['abstract', '1', '2', '3', '4', '5', '6', '7', '8', '9', '10', 'A', 'B']

export default class extends StandardPage {
  ord_renderContent() {
    return (
      <div>
        <Wrapper lang={this.props.lang}>
          <div class="title komu-a cr-title-with-icon">
            {I18N.get('navigation.whitepaper')}
          </div>
          <Collapse
            defaultActiveKey={['abstract']}
            bordered={false}
            expandIconPosition="right"
            expandIcon={() => (
              <img
                src="/assets/images/whitepaper-arrow.svg"
                className="arrow"
                width="26"
                height="15"
              />
            )}
          >
            {WHITEPAPER_KEYS.map(k => (
              <Collapse.Panel
                header={I18N.get(`whitepaper.${k}.title`)}
                key={k}
              >
                <div
                  dangerouslySetInnerHTML={{
                    __html: I18N.get(`whitepaper.${k}.body`)
                  }}
                />
              </Collapse.Panel>
            ))}
          </Collapse>
        </Wrapper>
        <Footer />
      </div>
    )
  }
}

const Wrapper = styled.div`
  max-width: 1200px;
  margin: auto;
  .title {
    font-size: 50px;
    line-height: 50px;
    padding: 25px 0 0 25px;
    margin: 70px 0 0 0;
  }
  .ant-collapse.ant-collapse-borderless .ant-collapse-item {
    border-bottom: 1px solid #e5e5e5;
    padding: 13px 0;
  }
  .ant-collapse .ant-collapse-item .ant-collapse-header {
    color: #7f7f7f;
    padding: 12px 0 12px 32px;
    font-size: 20px;
    line-height: 28px;
    .ant-collapse-arrow {
      transition: transform 500ms;
    }
  }
  .ant-collapse .ant-collapse-item-active {
    background: #f6f9fd;
  }
  .ant-collapse .ant-collapse-item-active .ant-collapse-header {
    color: #000;
    .ant-collapse-arrow {
      transform: rotate(180deg);
    }
  }
  .ant-collapse-content-box {
    ul, ol {
      padding-left: 1em;
      list-style: none;
    }
    p {
      text-indent: ${props => props.lang === 'en' ? 0 : '2em'};
    }
    p, ul, ol {
      margin: 0 32px;
      line-height: 1.8;
      li {
        padding-left: 1em;
        padding-bottom: 20px;
        span {
          color: #333;
          margin-right: 10px;
        }
      }
    }
    h3 {
      margin: 0 32px;
      font-size: 17px;
      color: #000;
    }
    h3 ~ p, h3 ~ ol, h3 ~ ul {
      margin: 0 48px;
    }
    ul > li {
      &::before {
        content: ' ';
        width: 6px;
        height: 6px;
        border-radius: 3px;
        background: #333;
        display: inline-block;
        vertical-align: middle;
        margin-right: 10px;
      }
    }
    img {
      padding: 48px;
      margin-bottom: 24px;
    }
  }
`
