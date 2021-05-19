import styled from 'styled-components'
import { breakPoint } from '@/constants/breakPoint'

export const Container = styled.div`
  text-align: left;
  .cr-title-with-icon {
    @media only screen and (max-width: ${breakPoint.mobile}) {
      padding-left: 26px;
      background-position: left 16px top 10px;
    }
  }
  .ant-form-item-label {
    margin-right: 20px;
    min-width: 120px;
    @media only screen and (max-width: ${breakPoint.mobile}) {
      text-align: left;
    }
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
  .ant-form .ant-row .ant-col-17 {
    @media only screen and (max-width: ${breakPoint.mobile}) {
      width: 100%;
    }
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
      .md-RichEditor-blockquote {
        border-left: 5px solid #ccc;
        background-color: unset;
        font-size: 1.1em;
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
  @media only screen and (max-width: ${breakPoint.mobile}) {
    margin-left: 16px;
  }
`
export const Status = styled.div`
  display: inline-block;
  font-size: 16px;
  line-height: 27px;
  text-transform: uppercase;
  color: #fff;
  margin-bottom: 48px;
  background: #be1313;
  height: 27px;
  padding: 0 6px;
  @media only screen and (max-width: ${breakPoint.mobile}) {
    margin-left: 16px;
    margin-bottom: 32px;
    font-size: 14px;
  }
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
export const RadioCardPanel = styled.div`
    display: flex;
    flex-wrap: wrap;
    justify-content: center;
    align-items: stretch;
`
export const RadioCardItem = styled.div`
    border: 1px solid #CCCCCC;
    box-sizing: border-box;
    margin-right: 20px;
    :last-child {
      margin-right: 0px;
    }
    &.small-label {
      width: 27%;
    }
    &.large-label {
      width: 39%;
    }
    &.radio-card-check {
      border-color: #008D85;
      background-color: rgba(29, 233, 182, 0.1);
    }
    @media only screen and (max-width: 1200px) {
      margin-right: 0px;
      margin-bottom: 20px;
      :last-child {
        margin-bottom: 0px;
      }
      &.small-label {
        width: 85%;
      }
      &.large-label {
        width: 85%;
      }
    }
    @media only screen and (max-width: ${breakPoint.mobile}) {
      &.small-label {
        width: 95%;
      }
      &.large-label {
        width: 95%;
      }
    }
`
export const RadioCardLabel = styled.label`
    display: inline-block;
    cursor: pointer;
    padding: 30px;
`
export const RadioCardSpan = styled.span`
  .ant-typography {
    font-family: Synthese;
    font-size: 14px;
    line-height: 20px;
  }
  .radio-card-check {
    font-weight: bold;
    font-size: 25px;
    color: #008D85;
  }
`
export const Part = styled.div`
  .preamble {
    font-family: Synthese;
    font-size: 13px;
    line-height: 18px;
    align-items: center;
    color: #000;
  }
`
export const PartTitle = styled.h4`
  font-family: Synthese;
  font-size: 20px;
  line-height: 28px;
  color: #000;
`
export const PartContent = styled.div``
