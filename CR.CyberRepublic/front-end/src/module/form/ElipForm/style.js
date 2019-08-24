import styled from 'styled-components'
import { Button } from 'antd'

export const Container = styled.div`
  text-align: left;
  .ant-form-item-label {
    margin-right: 20px;
    min-width: 120px;
  }
  .ant-form-item-required:before {
    display: none;
  }
  .ant-select-selection,
  .ant-input {
    border-radius: 0;
    background: rgba(204, 204, 204, 0.2);
    height: 42px;
  }
  .ant-form-item-label label {
    white-space: normal;
    display: block;
    font-size: 18px;
  }
  .md-RichEditor-root {
    padding: 15px 40px;
    margin-top: 0;
    border: 1px solid #d9d9d9;
    .md-RichEditor-editor {
      padding: 8px 16px;
      .public-DraftEditor-content {
        min-height: 320px;
      }
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

export const Title = styled.h2``

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
  bottom: 0;
  right: 6px;
  z-index: 10;
`
export const Text = styled.div`
  text-align: center;
`
export const Note = styled.div`
  margin-bottom: 15px;
`
export const NoteHighlight = styled.span`
  color: red;
`
export const Actions = styled.div`
  margin-top: 60px;
  display: flex;
  justify-content: center;
`
