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
    const { toggle } = this.state
    this.setState({ toggle: !toggle, show: true })
  }

  componentDidMount() {
    const { id } = this.props
    const el = document.querySelector(`#${id}`)
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
    const content = text
    return (
      <Wrapper>
        <div className={toggle ? 'container more' : 'container'} id={id}>
          <MarkdownPreview
            content={content ? content : ''}
            style={{ p: { margin: 0 } }}
          />
        </div>

        {show ? (
          <a onClick={this.showMore} style={{ textDecoration: 'underline' }}>
            {toggle
              ? I18N.get('milestone.showLess')
              : I18N.get('milestone.showMore')}
          </a>
        ) : null}
      </Wrapper>
    )
  }
}

export default ShowLongText

const Wrapper = styled.div`
  .container {
    height: 120px;
    overflow: hidden;
    line-height: 1.5;
  }
  .container.more {
    height: unset;
    overflow: unset;
  }
`
