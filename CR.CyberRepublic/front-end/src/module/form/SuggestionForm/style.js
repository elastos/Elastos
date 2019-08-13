import styled from 'styled-components'
import { Button } from 'antd'

export const Container = styled.div`
  text-align: left;
  .ant-form-item-label {
    text-align: left;
  }
  .ant-form-item-required:before {
    display: none;
  }
  .ant-select-selection,
  .ant-input {
    border-radius: 0;
  }
  .ant-form-item-label label {
    white-space: normal;
    /* line-height: 1.4rem; */
    display: block;
  }
  .ant-tabs {
    overflow: initial;
  }
  .md-RichEditor-root {
    padding: 15px 40px;
    margin-top: 0;
    border: 1px solid #d9d9d9;
    .md-RichEditor-editor {
      padding: 8px 16px;
    }
    .md-add-button {
      display: flex;
      justify-content: center;
      align-items: center;
    }
    figure.md-block-image {
      background: none;
    }
    figure.md-block-image figcaption .public-DraftStyleDefault-block {
      text-align: left;
    }
  }
`

export const Title = styled.h2`
`

export const Btn = styled(Button)`
  width: 100%;
  background: #66bda3;
  color: #fff;
  border-color: #009999;
  border-radius: 0;
`

export const CirContainer = styled.div`
  width: 18px;
  position: absolute;
  bottom: 50px;
  right: 50px;
  z-index: 10;
`

export const Text = styled.div`
  text-align: center;
`

export const TabText = styled.span`
  ${props => props.hasErr && `
    color: red;
  `}
`

export const TabPaneInner = styled.div`
  margin: 40px;
`

export const Note = styled.div`
  margin-bottom: 15px;
`

export const NoteHighlight = styled.span`
  color: red;
`
