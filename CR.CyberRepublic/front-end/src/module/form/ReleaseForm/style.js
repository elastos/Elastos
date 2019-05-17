import styled from 'styled-components'
import { Form } from 'antd'

const FormItem = Form.Item

export const Container = styled.div`
`

export const StyledFormItem = styled(FormItem)`
  input, textarea {
    border-radius: 0;
    &::placeholder {
      font-size: 14px;
      color: rgba(0, 0, 0, 0.65);
      font-style: italic;
    }
  }
`

export const StyledFormDesc = styled(FormItem)`
  min-height: 300px;
  margin-bottom: 30px;
  .ql-container {
    height: 245px;
  }
`
