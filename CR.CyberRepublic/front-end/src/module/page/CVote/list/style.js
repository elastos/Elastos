import styled from 'styled-components'
import { proposalStatus } from '../common/color'
import { grid } from '../common/variable'

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
  margin-left: ${props => (props.yes ? 0 : '10px')}

  background-color: ${props => (props.undecided && proposalStatus.undecided)
    || (props.abstained && proposalStatus.abstained)
    || (props.yes && proposalStatus.yes)
    || (props.no && proposalStatus.no)
  }
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
