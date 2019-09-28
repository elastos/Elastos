import { createEditorState } from 'medium-draft'
import mediumDraftExporter from 'medium-draft/lib/exporter'
import { convertFromHTML, ContentState, EditorState } from 'draft-js'
import _ from 'lodash'
import { logger } from '@/util'

export const getHTML = (data, key) => {
  const { contentType } = data
  const content = _.get(data, key, '')
  let editorState
  if (!content) {
    editorState = createEditorState()
  } else if (contentType === 'MARKDOWN') {
    try {
      editorState = createEditorState(JSON.parse(content))
    } catch (err) {}
  }

  if (!editorState) {
    try {
      const blocksFromHTML = convertFromHTML(content)
      if (!blocksFromHTML.contentBlocks) {
        editorState = createEditorState()
      } else {
        const state = ContentState.createFromBlockArray(
          blocksFromHTML.contentBlocks,
          blocksFromHTML.entityMap
        )
        editorState = EditorState.createWithContent(state)
      }
    } catch (err) {
      logger.error(err)
    }
  }

  return mediumDraftExporter(editorState.getCurrentContent())
}
