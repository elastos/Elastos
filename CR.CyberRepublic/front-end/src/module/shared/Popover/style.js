import styled from 'styled-components'
import { Icon, Input, Button, Popover } from 'antd'

const { TextArea } = Input

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
  &.has-error {
    border-color: #f5222d;
  }
`
export const Footer = styled.div`
  display: flex;
  justify-content: flex-end;
  margin-top: 20px;
`
export const Btn = styled(Button)`
  margin-left: 10px;
  background-color: ${props => (props.type === 'default' && '#fff!important')
    || (props.type === 'danger' && '#BE1313!important')
    || (props.type === 'primary' && '#008D85!important')
};

  border-color: ${props => (props.type === 'default' && props.colored && '#66bda3!important')
};

color: ${props => (props.type === 'default' && props.colored && '#66bda3!important')
    || (props.type === 'danger' && '#fff!important')
};
`
export const DataExplain = styled.div`
  min-height: 22px;
  margin-top: -2px;
  color: #f5222d;
  font-size: 14px;
  line-height: 1.5;
`
