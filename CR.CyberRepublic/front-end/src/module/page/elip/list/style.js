import styled from 'styled-components'
import { Button, Input, Col } from 'antd'
import { breakPoint } from '@/constants/breakPoint'
import { bg } from '@/constants/color'

const Search = Input.Search

export const Container = styled.div`
  background: #ffffff;
  margin: 50px 108px 80px 108px;
  @media only screen and (max-width: ${breakPoint.mobile}) {
    margin: 16px;
  }
`

export const List = styled.div`
  display: flex;
  align-items: center;
  flex: 0 1 300px;
  font-size: 14px;
  justify-content: flex-end;
  @media only screen and (max-width: ${breakPoint.mobile}) {
    justify-content: flex-start;
  }
`

export const StyledButton = styled(Button)`
  border-radius: 0 !important;
  color: rgba(3, 30, 40, 0.3) !important;
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
export const Filter = styled.div`
  margin-left: 64px;
  @media only screen and (max-width: ${breakPoint.mobile}) {
    margin-left: 0;
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
    width: 50%;
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
  padding-bottom: 10px;
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
  width: 25%;
  padding-left: 15px;
  font-family: Synthese;
  font-size: 14px;
  line-height: 20px;
  color: #000;

  :after {
    content: ':';
  }
`
