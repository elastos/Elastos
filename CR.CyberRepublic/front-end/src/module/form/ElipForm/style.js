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
    padding-left: 20px;
    padding-right: 20px;
    height: 42px;
  }
  .ant-form-item-label label {
    white-space: normal;
    display: block;
    font-size: 18px;
  }
  .md-RichEditor-root {
    padding: 0;
    margin-top: 0;
    border: 1px solid #d9d9d9;
    .md-RichEditor-editor {
      padding: 20px;
      .public-DraftEditor-content {
        min-height: 320px;
        margin: 0;
        padding: 0;
      }
    }
    .md-side-toolbar {
      background: #fff;
      left: -16px;
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
export const Actions = styled.div`
  margin-top: 60px;
  display: flex;
  justify-content: center;
`
export const Label = styled.div`
  font-size: 11px;
  line-height: 19px;
  color: rgba(3, 30, 40, 0.4);
`
export const Status = styled.div`
  display: inline-block;
  font-size: 16px;
  line-height: 27px;
  text-transform: uppercase;
  color: #fff;
  margin-bottom: 42px;
  background: #be1313;
  height: 27px;
  padding: 0 6px;
`
