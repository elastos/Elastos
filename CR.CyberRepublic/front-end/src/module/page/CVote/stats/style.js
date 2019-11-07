import styled from 'styled-components'
import { CVOTE_RESULT_COLOR } from '@/constant'

export const List = styled.div`
  display: flex;
`
export const Item = styled.div`
  flex: 1 1 auto;
  height: 10px;
  box-sizing: border-box;
  margin-right: 1px;
  margin-left: 0;
  background-color: ${props => CVOTE_RESULT_COLOR[props.status]};
`
export const ItemUndecided = styled(Item)`
  position: relative;
  overflow: hidden;
  &:before {
    content: "";
    position: absolute;
    width: 100%;
    height: 100%;
    top: 0;
    left: 0;
    background: url("/assets/images/bg-line.png") 0 0 repeat;
  }
`

export const Text = styled.div`
  color: ${props => (props.yes ? '#008D85' : '#CED6E3')}
`
