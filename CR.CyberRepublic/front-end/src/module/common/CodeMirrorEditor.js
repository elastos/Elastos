import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import { UnControlled as CodeMirror } from 'react-codemirror2'
import 'codemirror/mode/gfm/gfm'
import 'codemirror/lib/codemirror.css'
import 'codemirror/theme/base16-light.css'
import MarkdownPreview from '@/module/common/MarkdownPreview'
import styles from 'styled-components'

class Component extends BaseComponent {
  constructor(props) {
    super(props)
    this.state = {
      value: ''
    }
    this.editor = null
  }

  onChange = (editor, data, value) => {
    const { onChange } = this.props
    if (onChange) onChange(value)
    this.setState({ value })
  }

  onClick = () => {
    const doc = this.editor.getDoc()
    const cursor = doc.getCursor()
    doc.replaceRange(
      '![minion](https://octodex.github.com/images/minion.png)',
      cursor
    )
  }

  ord_render() {
    return (
      <Wrapper>
        <button onClick={this.onClick}>Insert Image</button>
        <CodeMirror
          value={this.props.content}
          options={{
            mode: 'gfm',
            theme: 'base16-light',
            lineWrapping: true
          }}
          onChange={this.onChange}
          editorDidMount={editor => {
            this.editor = editor
          }}
        />
        <MarkdownPreview content={this.state.value} />
      </Wrapper>
    )
  }
}

export default Component

const Wrapper = styles.div`
  .CodeMirror {
    min-height: 450px;
    height: unset;
    border: 1px solid #d9d9d9;
    background: #cccccc33;
  }
  .CodeMirror-wrap pre.CodeMirror-line,
  .CodeMirror-wrap pre.CodeMirror-line-like {
    line-height: 1.5;
  }
  .CodeMirror-scroll {
    padding: 16px 20px 30px 20px;
    overflow: auto !important;
    min-height: 450px;
    margin-right: 0;
  }
`
