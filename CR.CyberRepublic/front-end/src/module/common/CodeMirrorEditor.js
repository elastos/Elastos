import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import { Controlled as CodeMirror } from 'react-codemirror2'
import 'codemirror/mode/gfm/gfm'
import 'codemirror/lib/codemirror.css'
import 'codemirror/theme/base16-light.css'
import MarkdownPreview from '@/module/common/MarkdownPreview'
import styled from 'styled-components'
import UploadBase64Image from '@/module/common/UploadBase64Image'
import ToggleMarkdownPreview from '@/module/common/ToggleMarkdownPreview'

class Component extends BaseComponent {
  constructor(props) {
    super(props)
    this.state = {
      value: this.props.content ? this.props.content : '',
      show: false
    }
    this.editor = null
  }

  insertImage = base64 => {
    const doc = this.editor.getDoc()
    const cursor = doc.getCursor()
    doc.replaceRange(`\n![image](${base64})\n`, cursor)
  }

  togglePreview = () => {
    this.setState({ show: !this.state.show })
  }

  onChange = (editor, data, value) => {
    const { onChange, callback, activeKey } = this.props
    if (onChange) onChange(value)
    if (callback) callback(activeKey)
  }

  ord_render() {
    const { show, value } = this.state
    return (
      <Wrapper>
        <Toolbar>
          <UploadBase64Image insertImage={this.insertImage} />
          <ToggleMarkdownPreview togglePreview={this.togglePreview} />
        </Toolbar>
        {show === false ? (
          <CodeMirror
            value={value}
            options={{
              mode: 'gfm',
              theme: 'base16-light',
              lineWrapping: true
            }}
            onBeforeChange={(editor, data, value) => {
              this.setState({ value })
            }}
            editorDidMount={editor => {
              this.editor = editor
            }}
            onChange={this.onChange}
          />
        ) : (
          <Preview>
            <MarkdownPreview content={value} />
          </Preview>
        )}
      </Wrapper>
    )
  }
}

export default Component

const Wrapper = styled.div`
  .CodeMirror {
    min-height: 450px;
    height: unset;
    border: 1px solid #d9d9d9;
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
  /* hide long long base64 string */
  .cm-s-base16-light .cm-image.cm-link ~ span.cm-string {
    overflow: hidden;
    white-space: nowrap;
    text-overflow: ellipsis;
    max-width: 20%;
    display: inline-block;
    vertical-align: bottom;
  }
`
const Toolbar = styled.div`
  display: flex;
  margin-bottom: -16px;
  justify-content: flex-end;
`

const Preview = styled.div`
  min-height: 450px;
  padding: 20px;
  background: rgba(204, 204, 204, 0.2);
`
