import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import _ from 'lodash'
import { CVOTE_STATUS, CVOTE_STATUS_TEXT } from '@/constant'
import { Editor, createEditorState, } from 'medium-draft'
import { convertToRaw, convertFromRaw, convertFromHTML, ContentState, EditorState } from 'draft-js'
import { MEDIUM_DRAFT_TOOLBAR_OPTIONS } from '@/config/constant'

// if using webpack
import 'medium-draft/lib/index.css'

import { Container, Title, Btn } from './style'

const FormItem = Form.Item
const { TextArea } = Input

class C extends BaseComponent {
  constructor(props) {
    super(props)

    let editorState
    if (!props.data) {
      editorState = createEditorState()
    } else if (props.data.contentType === 'MARKDOWN') {
      const content = JSON.parse(_.get(props.data, 'content'))
      console.log('constructor content: ', content)
      editorState = createEditorState(content)
    } else {
      const blocksFromHTML = convertFromHTML(props.data.content)
      const state = ContentState.createFromBlockArray(
        blocksFromHTML.contentBlocks,
        blocksFromHTML.entityMap
      )
      editorState = EditorState.createWithContent(state)
    }

    console.log('constructor editorState: ', editorState)

    this.state = {
      editorState,
    }

    this.refsEditor = React.createRef()
  }

  componentDidMount() {
    this.refsEditor.current && this.refsEditor.current.focus()
  }

  onChange = (editorState) => {
    this.setState({ editorState })
  }

  ord_renderContent() {
    return (
      <Editor
        ref={this.refsEditor}
        placeholder=""
        sideButtons={[]}
        blockButtons={MEDIUM_DRAFT_TOOLBAR_OPTIONS.BLOCK_BUTTONS}
        inlineButtons={MEDIUM_DRAFT_TOOLBAR_OPTIONS.INLINE_BUTTONS}
        editorState={editorState}
        onChange={this.onChange} />
    )
  }
}