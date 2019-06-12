import React from 'react'
import PropTypes from 'prop-types'
import _ from 'lodash'
import BaseComponent from '@/model/BaseComponent'
import { Editor, createEditorState, } from 'medium-draft'
import { convertFromHTML, ContentState, EditorState } from 'draft-js'

class Component extends BaseComponent {
  ord_render() {
    const { contentType, content } = this.props
    let editorState
    if (contentType === 'MARKDOWN') {
      editorState = createEditorState(JSON.parse(content))
    } else {
      const blocksFromHTML = convertFromHTML(content)
      const state = ContentState.createFromBlockArray(
        blocksFromHTML.contentBlocks,
        blocksFromHTML.entityMap
      )
      editorState = EditorState.createWithContent(state)
    }
    const contentNode = (
      <Editor
        ref={this.refsEditor}
        placeholder=""
        editorEnabled={false}
        sideButtons={[]}
        editorState={editorState}
        onChange={null} />
    )
    return contentNode
  }
}

const propTypes = {
  contentType: PropTypes.string.isRequired,
  content: PropTypes.string.isRequired,
}

Component.propTypes = propTypes


export default Component
