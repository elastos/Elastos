import React from 'react'
import PropTypes from 'prop-types'
import BaseComponent from '@/model/BaseComponent'
import { Editor, createEditorState, StringToTypeMap, Block, HANDLED, NOT_HANDLED, resetBlockWithType, getCurrentBlock } from 'medium-draft'
import { convertFromHTML, ContentState, EditorState } from 'draft-js'
import { MEDIUM_DRAFT_TOOLBAR_OPTIONS } from '@/config/constant'
import { CONTENT_TYPE } from '@/constant'

// if using webpack
import 'medium-draft/lib/index.css'

const newTypeMap = {
  ...StringToTypeMap,
  '## ': Block.H2,
  '### ': Block.H3,
  '#### ': Block.H4,
  '##### ': Block.H5,
  '###### ': Block.H6,
}
delete newTypeMap['##']

class Component extends BaseComponent {
  constructor(props) {
    super(props)
    this.updateContent()

    this.refsEditor = React.createRef()
  }

  componentDidMount() {
    this.refsEditor.current && this.refsEditor.current.focus()
  }

  updateContent = () => {
    const { content, contentType } = this.props
    let editorState
    if (!content) {
      editorState = createEditorState()
    } else if (contentType === CONTENT_TYPE.MARKDOWN) {
      editorState = createEditorState(JSON.parse(content))
    } else {
      const blocksFromHTML = convertFromHTML(content)
      const state = ContentState.createFromBlockArray(
        blocksFromHTML.contentBlocks,
        blocksFromHTML.entityMap
      )
      editorState = EditorState.createWithContent(state)
    }

    this.state = {
      editorState,
    }
  }

  onChange = (editorState) => {
    this.setState({ editorState })
    this.props.onChange && this.props.onChange(editorState)
  }

  handleBeforeInput = (editorState, inputString, onChange) => {
    const mapping = newTypeMap
    // const selection = editorState.getSelection()
    const block = getCurrentBlock(editorState)
    const blockType = block.getType()
    if (blockType.indexOf(Block.ATOMIC) === 0) {
      return NOT_HANDLED
    }
    // const blockLength = block.getLength()
    // if (selection.getAnchorOffset() > 1 || blockLength > 1) {
    //   return NOT_HANDLED
    // }
    // const blockTo = mapping[block.getText()[0] + inputString]
    const blockTo = mapping[block.getText() + inputString]
    if (!blockTo) {
      return NOT_HANDLED
    }
    const finalType = blockTo.split(':')
    if (finalType.length < 1 || finalType.length > 3) {
      return NOT_HANDLED
    }
    let fType = finalType[0]
    if (finalType.length === 1) {
      if (blockType === finalType[0]) {
        return NOT_HANDLED
      }
    } else if (finalType.length === 2) {
      if (blockType === finalType[1]) {
        return NOT_HANDLED
      }
      if (blockType === finalType[0]) {
        fType = finalType[1]
      }
    } else if (finalType.length === 3) {
      if (blockType === finalType[2]) {
        return NOT_HANDLED
      }
      if (blockType === finalType[0]) {
        fType = finalType[1]
      } else {
        fType = finalType[2]
      }
    }
    onChange(resetBlockWithType(editorState, fType, {
      text: '',
    }))
    return HANDLED
  }

  ord_render() {
    const { editorEnabled } = this.props
    return (
      <Editor
        ref={this.refsEditor}
        placeholder=""
        editorEnabled={editorEnabled}
        sideButtons={[]}
        blockButtons={MEDIUM_DRAFT_TOOLBAR_OPTIONS.BLOCK_BUTTONS}
        inlineButtons={MEDIUM_DRAFT_TOOLBAR_OPTIONS.INLINE_BUTTONS}
        editorState={this.state.editorState}
        onChange={this.onChange}
        beforeInput={this.handleBeforeInput}
      />
    )
  }
}

const propTypes = {
  contentType: PropTypes.string,
  content: PropTypes.string,
  editorEnabled: PropTypes.bool,
}

Component.propTypes = propTypes


export default Component
