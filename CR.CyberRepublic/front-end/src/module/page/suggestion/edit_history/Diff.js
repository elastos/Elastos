import React from 'react'
import styled from 'styled-components'
import sanitizeHtml from '@/util/html'

const jsdiff = require('diff')
const showdown = require('showdown')
const TurndownService = require('turndown/lib/turndown.browser.umd.js')

const converter = new showdown.Converter()
// text      = '# hello, markdown!',
// html      = converter.makeHtml(text);
const turndownService = new TurndownService()
// const markdown = turndownService.turndown('<h1>Hello world!</h1>')

const fnMap = {
  chars: jsdiff.diffChars,
  words: jsdiff.diffWords,
  sentences: jsdiff.diffSentences,
  json: jsdiff.diffJson,
}

const StyledSpan = styled.span`
  > * {
    padding-bottom: 4px;
    background-color: ${props => props.highlight && 'rgba(29, 233, 182, 0.1)'};
  }
`

export default ({ type, inputA, inputB }) => {
  const diffA = inputA ? turndownService.turndown(inputA) : ''
  const diffB = inputB ? turndownService.turndown(inputB) : ''
  const diff = fnMap[type || 'chars'](diffA, diffB)
  const result = diff.map((part, index) => {
    const spanStyle = {}
    if (part.added || part.removed) {
      spanStyle.color = '#008d85'
      spanStyle.backgroundColor = 'rgba(29, 233, 182, 0.1)'
    }
    if (part.removed) {
      spanStyle.textDecoration = 'line-through'
    }

    return <StyledSpan key={index} highlight={part.added || part.removed} style={spanStyle} dangerouslySetInnerHTML={{ __html: converter.makeHtml(sanitizeHtml(part.value)) }} />
    // return <span key={index} style={spanStyle}>{part.value}</span>
  })
  return (
    <div className="diff-result">
      {result}
    </div>
  )
}
