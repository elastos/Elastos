import React, { Component } from 'react'
import MarkdownPreview from '@/module/common/MarkdownPreview'
import I18N from '@/I18N'
import styled from 'styled-components'
import ResizeObserver from 'resize-observer-polyfill'

class ShowLongText extends Component {
  constructor(props) {
    super(props)
    this.state = {
      toggle: false,
      show: false
    }
  }

  showMore = () => {
    this.setState({ toggle: true })
  }

  showLess = () => {
    this.setState({ toggle: false })
  }

  componentDidMount() {
    const { id } = this.props
    const el = document.querySelector(`#${id}`)
    if (!el) {
      return
    }
    const myObserver = new ResizeObserver((entries) => {
      entries.forEach((entry) => {
        if (entry.contentRect.height < entry.target.scrollHeight) {
          this.setState({ show: true })
        } else {
          this.setState({ show: false })
        }
      })
    })
    myObserver.observe(el)
  }

  render() {
    const { toggle, show } = this.state
    const { text, id } = this.props
    return (
      <Wrapper>
        <div className={toggle ? 'container more' : 'container'} id={id}>
          <MarkdownPreview
            content={text ? text : ''}
            style={{
              p: { margin: 0 },
              '>:last-child': { marginBottom: 0, lineHeight: 1.8 }
            }}
          />
        </div>
        {show && <div>......</div>}
        {show && (
          <Button onClick={this.showMore}>
            {I18N.get('milestone.showMore')}
          </Button>
        )}
        {toggle && (
          <Button onClick={this.showLess}>
            {I18N.get('milestone.showLess')}
          </Button>
        )}
      </Wrapper>
    )
  }
}

export default ShowLongText

const Wrapper = styled.div`
  .container {
    max-height: 125px;
    overflow: hidden;
    line-height: 1.8;
    > div * {
      margin-top: 0;
      margin-bottom: 0;
    }
    ul,
    ol {
      margin-bottom: 0 !important;
    }
    li:last-child {
      margin-bottom: 0 !important;
    }
  }
  .container.more {
    max-height: unset;
    overflow: unset;
    > div * {
      margin-top: 1em;
    }
  }
`
const Button = styled.a`
  text-decoration: underline;
  margin: 16px 0;
  display: block;
`
