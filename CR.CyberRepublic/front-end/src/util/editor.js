import { createEditorState } from 'medium-draft'
import mediumDraftExporter from 'medium-draft/lib/exporter'

export const getHTML = (data, key) => {
  const { contentType } = data
  const content = _.get(data, key, '')
  let editorState
  if (content && contentType === 'MARKDOWN') {
    editorState = createEditorState(JSON.parse(content))
    return mediumDraftExporter(editorState.getCurrentContent())
  }
  return content
}
