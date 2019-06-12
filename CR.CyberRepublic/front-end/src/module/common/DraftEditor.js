import React from 'react'
import PropTypes from 'prop-types'
import BaseComponent from '@/model/BaseComponent'
import { Editor, createEditorState } from 'medium-draft'
import { convertFromHTML, ContentState, EditorState } from 'draft-js'
import { MEDIUM_DRAFT_TOOLBAR_OPTIONS } from '@/config/constant'

// if using webpack
import 'medium-draft/lib/index.css'

class Component extends BaseComponent {
  constructor(props) {
    super(props)
    console.log('constructor props: ', props)
    this.updateContent()

    this.refsEditor = React.createRef()
  }

  componentDidMount() {
    this.refsEditor.current && this.refsEditor.current.focus()
  }

  componentWillReceiveProps() {
  }

  updateContent = () => {
    const { content, contentType } = this.props
    let editorState
    if (!content) {
      editorState = createEditorState()
    } else if (contentType === 'MARKDOWN') {
      editorState = createEditorState(JSON.parse(content))
    } else {
      const blocksFromHTML = convertFromHTML(content)
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
  }

  onChange = (editorState) => {
    this.setState({ editorState })
    this.props.onChange && this.props.onChange(editorState)
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
