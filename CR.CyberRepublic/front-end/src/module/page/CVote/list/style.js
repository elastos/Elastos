import styled from 'styled-components'
import { Button, Input, Col } from 'antd'
import { grid } from '../common/variable'
import { CVOTE_RESULT_COLOR, CVOTE_RESULT } from '@/constant'
import { breakPoint } from '@/constants/breakPoint'
import { bg } from '@/constants/color'

const Search = Input.Search

export const Container = styled.div`
  background: #ffffff;
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
  margin-left: ${props => (props.status === CVOTE_RESULT.SUPPORT ? 0 : '10px')};
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
export const StyledSearch = styled(Search)`
  .ant-input {
    border-radius: 0;
  }
`
export const VoteFilter = styled.div`
  margin: 20px 0;
  text-align: right;
  @media only screen and (max-width: ${breakPoint.mobile}) {
    text-align: left;
  }
`

export const FilterLabel = styled(Col)`
  color: #008D85;
  cursor: pointer;
`

export const FilterPanel = styled.div`
  .filter {
    margin-top: 20px;
  }
  .filter-btn {
    margin-top: 36px;
    margin-bottom: 58px;
  }
  .filter-input {
    width: ${props => (props.isCouncil ? '60%' : '50%')};
    ${props => (props.isCouncil ? 'padding-right: 15px;' : '')}
  }
`

export const FilterClearBtn = styled.div`
  text-align: center;
  min-width: 155px;
  height: 40px;
  line-height: 40px;
  color: rgba(3, 30, 40, 0.3);
  cursor: pointer;
`


export const FilterItem = styled.div`
  display: flex;
  justify-content: flex-start;
  align-items: center;
  padding-left: 15px;
  padding-bottom: 10px;
  &.filter-checkbox {
    padding-top: 10px;
  }
  :first-child {
    padding-top: 20px;
  }
  :last-child {
    padding-bottom: 20px;
  }
`
export const FilterContent = styled.div`
  background: #F6F9FD;
  height: 100%;
`

export const FilterItemLabel = styled.div`
  width: ${props => (props.isCouncil ? '40%' : '25%')};;
  font-family: Synthese;
  font-size: 14px;
  line-height: 20px;
  color: #000;

  :after {
    content: ':';
  }
`

export const CheckboxText = styled.span`
  margin-left: 10px;
`

export const SplitLabel = styled.span`
  padding: 0 16px;
  margin-bottom: 16px;
  color: rgba(3, 30, 40, 0.3);
  :after {
    content: '|';
  }
`

export const ViewOldDataBtn = styled.div`
  margin-bottom: 16px;
  color: #43af92;
  &:hover {
    cursor: pointer;
    color: #388582;
  }
`
