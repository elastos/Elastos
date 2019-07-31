import React from 'react'
import PropTypes from 'prop-types'
import BaseComponent from '@/model/BaseComponent'
import _ from 'lodash'
import { Editor, createEditorState, StringToTypeMap, Block, HANDLED, NOT_HANDLED, resetBlockWithType, getCurrentBlock } from 'medium-draft'
import { convertFromHTML, convertFromRaw, ContentState, EditorState } from 'draft-js'
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

    this.refsEditor = React.createRef()
  }

  componentDidMount() {
    this.refsEditor.current && this.refsEditor.current.focus()
  }

  generateEditorState = () => {
    const { value, contentType } = this.props
    let editorState = null;
    if (!value) {
      editorState = createEditorState()
    } else if (_.isObject(value)) {
      editorState = value
    } else if (contentType === CONTENT_TYPE.MARKDOWN) {
      try {
        editorState = createEditorState(JSON.parse(value))
      } catch(err) {}
    }

    if (!editorState) {
      const blocksFromHTML = convertFromHTML(value)
      const state = ContentState.createFromBlockArray(
        blocksFromHTML.contentBlocks,
        blocksFromHTML.entityMap
      )
      editorState = EditorState.createWithContent(state)
    }

    return editorState
  }

  onChange = (editorState) => {
    const { onChange, callback } = this.props

    if (onChange) onChange(editorState)
    if (callback) callback()
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
    return (
      <Editor
        {...this.props}
        ref={this.refsEditor}
        placeholder=""
        sideButtons={[]}
        blockButtons={MEDIUM_DRAFT_TOOLBAR_OPTIONS.BLOCK_BUTTONS}
        inlineButtons={MEDIUM_DRAFT_TOOLBAR_OPTIONS.INLINE_BUTTONS}
        editorState={this.generateEditorState()}
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
