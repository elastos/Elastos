import styled from 'styled-components'
import { Button } from 'antd'

export const Container = styled.div`
  text-align: left;
  .ant-form-item-label {
    text-align: left;
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
    padding: 15px 30px;
    margin-top: 0;
    border: 1px solid #d9d9d9;
  }

`

export const Title = styled.h2`
  text-align: center;
`

export const Btn = styled(Button)`
  width: 100%;
  background: #66bda3;
  color: #fff;
  border-color: #009999;
  border-radius: 0;
`

export const Text = styled.div`
  text-align: center;
`
