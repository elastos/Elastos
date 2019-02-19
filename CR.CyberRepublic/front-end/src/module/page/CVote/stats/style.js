import styled from 'styled-components'
import { proposalStatus } from '../common/color'

export const List = styled.div`
  display: flex;
`
export const Item = styled.div`
  flex-basis: 33.3%;
  height: 10px;
  box-sizing: border-box;
  border-right: 1px solid white;
  margin-left: 0;
  background-color: ${props => (props.status === 'undecided' && proposalStatus.undecided)
    || (props.status === 'abstained' && proposalStatus.abstained)
    || (props.status === 'yes' && proposalStatus.yes)
    || (props.status === 'no' && proposalStatus.no)
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

export const Text = styled.div`
  color: ${props => (props.yes ? '#008D85' : '#CED6E3')}
`
