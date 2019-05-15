import assign from 'lodash/assign'
import saniHtml from 'sanitize-html'

// Basic configuration
const CONFIG = {
  allowedTags: ['p', 'div', 'h3', 'h4', 'h5', 'h6', 'ul', 'ol', 'li', 'a', 'em', 'strong', 'br', 'span', 'b', 'i'],
  allowedAttributesByTags: {a: ['href', 'title'], img: ['src', 'alt', 'title'], span: ['class']},
  allowedSchemas: ['http', 'https'],
}

export default (data, settings) => {
  const defaultSettings = {
    allowedTags: CONFIG.allowedTags,
    allowedAttributes: CONFIG.allowedAttributesByTags,
    allowedSchemes: CONFIG.allowedSchemas,
  }
  settings = assign(defaultSettings, settings)
  return saniHtml(data, settings)
}
