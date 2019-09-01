import styled from 'styled-components'
import { Button, Input } from 'antd'
import { breakPoint } from '@/constants/breakPoint'
import { bg } from '@/constants/color'

const Search = Input.Search

export const Container = styled.div`
  background: #ffffff;
  margin: 50px 108px 80px 108px;
  @media only screen and (max-width: ${breakPoint.mobile}) {
    margin: 15px;
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
  margin: 20px 0;
  text-align: right;
  @media only screen and (max-width: ${breakPoint.mobile}) {
    text-align: left;
  }
`
