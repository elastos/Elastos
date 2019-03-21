import React from 'react'
import styled from 'styled-components'

const jsdiff = require('diff')

const fnMap = {
  chars: jsdiff.diffChars,
  words: jsdiff.diffWords,
  sentences: jsdiff.diffSentences,
  json: jsdiff.diffJson,
};

const StyledSpan = styled.span`
  p {
    padding-bottom: 4px;
    background-color: ${props => props.highlight && 'rgba(29, 233, 182, 0.1)'};
  }
`

export default ({ type, inputA, inputB }) => {
  const diff = fnMap[type || 'chars'](inputA, inputB);
  const result = diff.map((part, index) => {
    const spanStyle = {}
    if (part.added || part.removed) {
      spanStyle.color = '#008d85'
      spanStyle.backgroundColor = 'rgba(29, 233, 182, 0.1)'
    }
    if (part.removed) {
      spanStyle.textDecoration = 'line-through'
    }

    return <StyledSpan key={index} highlight={part.added || part.removed} style={spanStyle} dangerouslySetInnerHTML={{ __html: part.value }} />
    // return <span key={index} style={spanStyle}>{part.value}</span>
  });
  return (
    <div className="diff-result">
      {result}
    </div>
  );
}
