import React from 'react'
import styled from 'styled-components'
import { convertMarkdownToHtml } from '@/util/markdown-it'

function MarkedPreview({ content, style }) {
  return (
    <Wrapper
      dangerouslySetInnerHTML={{
        __html: convertMarkdownToHtml(content)
      }}
      style={style}
    />
  )
}

export default MarkedPreview

const Wrapper = styled.div`
  overflow-wrap: break-word;
  color: rgba(0, 0, 0, 0.75);
  font-size: 14px;
  font-family: 'Synthese', 'Montserrat', sans-serif;
  font-variant-ligatures: common-ligatures;
  line-height: 1.8;
  -webkit-font-smoothing: antialiased;
  -moz-osx-font-smoothing: grayscale;
  tab-size: 4;
  &:first-child {
    margin-top: 0;
    padding-top: 0;
  }
  p {
    margin: 1.2em 0;
    padding: 0;
  }
  blockquote,
  pre,
  ul,
  ol,
  dl {
    margin: 1em 0;
    padding: 0;
  }

  ul,
  ol {
    margin-left: 1em;
    margin-right: 1em;
    > li {
      margin: 0 1em 1em;
    }
  }

  h1,
  h2,
  h3,
  h4,
  h5,
  h6 {
    margin: 1.8em 0;
    line-height: 1.33;
    padding: 0;
  }

  h1 {
    font-size: 2em;
  }

  h2 {
    font-size: 1.5em;
  }

  h3 {
    font-size: 1.17em;
  }

  h4 {
    font-size: 1em;
  }

  h5 {
    font-size: 0.83em;
  }

  h1,
  h2 {
    &::after {
      content: '';
      display: block;
      position: relative;
      top: 0.33em;
      border-bottom: 1px solid rgba(128, 128, 128, 0.33);
    }
  }

  ol ul,
  ul ol,
  ul ul,
  ol ol {
    margin: 0;
  }

  dt {
    font-weight: bold;
  }

  a {
    color: #43af92;
    text-decoration: underline;
    text-decoration-skip: ink;
    &:hover,
    &:focus {
      text-decoration: none;
    }
  }

  code,
  pre,
  samp {
    font-family: 'Roboto Mono', 'Lucida Sans Typewriter', 'Lucida Console',
      monaco, Courrier, monospace;
    font-size: 0.85em;
    * {
      font-size: inherit;
    }
  }

  blockquote {
    color: rgba(0, 0, 0, 0.5);
    padding-left: 1.5em;
    border-left: 5px solid rgba(0, 0, 0, 0.1);
  }

  code {
    background-color: rgba(0, 0, 0, 0.05);
    border-radius: 3px;
    padding: 2px 4px;
  }

  hr {
    border: 0;
    border-top: 1px solid rgba(128, 128, 128, 0.33);
    margin: 2em 0;
  }

  pre > code {
    background-color: rgba(0, 0, 0, 0.05);
    display: block;
    padding: 1em;
    -webkit-text-size-adjust: none;
    overflow-x: auto;
    white-space: pre;
  }

  table {
    background-color: transparent;
    border-collapse: collapse;
    border-spacing: 0;
  }

  td,
  th {
    border-right: 1px solid #dcdcdc;
    padding: 8px 12px;
    &:last-child {
      border-right: 0;
    }
  }

  td {
    border-top: 1px solid #dcdcdc;
  }

  mark {
    background-color: #f8f840;
  }

  abbr {
    &[title] {
      border-bottom: 1px dotted #777;
      cursor: help;
    }
  }

  img {
    max-width: 100%;
    margin: 32px 0;
    display: block;
  }

  .task-list-item {
    list-style-type: none;
  }

  .task-list-item-checkbox {
    margin: 0 0.2em 0 -1.3em;
  }

  .footnotes {
    font-size: 0.8em;
    position: relative;
    top: -0.25em;
    vertical-align: top;
  }
  ${props => props.style}
`
