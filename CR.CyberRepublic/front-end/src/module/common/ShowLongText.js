import React, { Component } from 'react'
import MarkdownPreview from '@/module/common/MarkdownPreview'
import I18N from '@/I18N'

class ShowLongText extends Component {
  constructor(props) {
    super(props)
    this.state = {
      toggle: true
    }
  }

  showMore = () => {
    const { toggle } = this.state
    this.setState({ toggle: !toggle })
  }

  render() {
    const { toggle } = this.state
    const { text } = this.props
    const arr = text && text.split('\n')
    const content = toggle ? arr && `${arr.slice(0, 5).join('\n')}...` : text
    return (
      <div>
        <MarkdownPreview
          content={content ? content : ''}
          style={{ p: { margin: '1em 0' } }}
        />
        {arr.length > 5 ? (
          <a onClick={this.showMore} style={{ textDecoration: 'underline' }}>
            {toggle
              ? I18N.get('milestone.showMore')
              : I18N.get('milestone.showMore')}
          </a>
        ) : null}
      </div>
    )
  }
}

export default ShowLongText
