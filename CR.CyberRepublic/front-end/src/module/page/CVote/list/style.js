import styled from 'styled-components'
import { Button } from 'antd'
import { grid } from '../common/variable'
import { CVOTE_RESULT_COLOR, CVOTE_RESULT } from '@/constant'
import { breakPoint } from '@/constants/breakPoint'
import { bg } from '@/constants/color'

export const Container = styled.div`
  background: #ffffff;
  text-align: center;
  margin: 50px 108px 80px 108px;
  @media only screen and (max-width: ${grid.sm}) {
    margin: 15px;
  }
`

export const List = styled.div`
  display: flex;
  align-items: center;
  flex: 0 1 300px;
  font-size: 14px;
  justify-content: flex-end;
  @media only screen and (max-width: ${grid.sm}) {
    justify-content: flex-start;
  }
`

export const Item = styled.div`
  flex: 0 0 10px;
  height: 10px;
  box-sizing: border-box;
  margin-right: 2px;
  margin-left: ${props => (props.status === CVOTE_RESULT.SUPPORT ? 0 : '10px')}
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

export const StyledButton = styled(Button)`
  border-radius: 0 !important;
  &.selected {
    color: white !important;
    background-color: ${bg.obsidian} !important;
    border-color: ${bg.obsidian} !important;
  }
`
export const VoteFilter = styled.div`
  margin: 20px 0;
  text-align: right;
  @media only screen and (max-width: ${breakPoint.mobile}) {
    text-align: left;
  }
`
