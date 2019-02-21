import styled from 'styled-components'
import { Icon, Input, Button, Popover } from 'antd'

const { TextArea } = Input;

export const StyledPopover = styled(Popover)`
  background: #F6F9FD;
`
export const Container = styled.div`
  position: relative;
  padding: 10px;
  width: 546px;
`
export const CloseIcon = styled(Icon)`
  position: absolute;
  right: 0;
`
export const Title = styled.h4`

`
export const StyledTextArea = styled(TextArea)`
`
export const Footer = styled.div`
  display: flex;
  justify-content: flex-end;
  margin-top: 20px;
`
export const Btn = styled(Button)`
  border-radius: 0;
  margin-left: 10px;
  background: ${props => (props.type === 'default' && '#fff')
    || (props.type === 'danger' && '#BE1313')
  };
  color: ${props => (props.type === 'default' && 'rgba(3, 30, 40, 0.3)')
    || (props.type === 'danger' && '#fff')
  };
`
