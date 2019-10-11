import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import { UnControlled as CodeMirror } from 'react-codemirror2'
import 'codemirror/mode/gfm/gfm'
import 'codemirror/lib/codemirror.css'
import 'codemirror/theme/base16-light.css'

class Component extends BaseComponent {
  onChange = (editor, data, value) => {
    const { onChange } = this.props
    if (onChange) onChange(value)
  }

  ord_render() {
    return (
      <CodeMirror
        value={this.props.content}
        options={{
          mode: 'gfm',
          theme: 'base16-light',
          lineWrapping: true
        }}
        autoCursor={false}
        onChange={this.onChange}
      />
    )
  }
}

export default Component
