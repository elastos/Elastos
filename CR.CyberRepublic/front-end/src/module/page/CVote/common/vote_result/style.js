import styled from 'styled-components'
import { proposalStatus } from '../color'

export const Container = styled.div`
  display: flex;
  align-items: center;
  min-height: 220px;
  margin-bottom: 10px;
  &:last-child :last-child:after {
    border-bottom: none;
  }
`

export const Label = styled.div`
  text-align: right;
  margin-right: 50px;
  flex: 0 0 90px;
`

export const List = styled.div`
  position: relative;
  padding: 40px;
  display: ${props => (props.type === 'no' ? 'block' : 'flex')};
  align-items: center;
  box-sizing: border-box;
  height: 100%;
  border-left: 10px solid;
  border-color: ${props => (props.type === 'no' && proposalStatus.no)
    || (props.type === 'yes' && proposalStatus.yes)
    || (props.type === 'abstained' && proposalStatus.abstained)
    || (props.type === 'undecided' && proposalStatus.undecided)
  }
  &:after {
    content: " ";
    min-width: 500px;
    position: absolute;
    left: 5px;
    right: 5px;
    bottom: -7px;
    border-bottom: 1px solid #E5E5E5;
  }
`

export const Item = styled.div`
  text-align: center;
  margin-left: 28px;
`

export const Avatar = styled.img`
  display: block;
  border-radius: 50%;
  width: 100px;
  height: 100px;
  margin-bottom: 15px;
`

export const RejectRow = styled.div`
  display: flex;
  margin-bottom: 30px;
`

export const Reason = styled.div`
  margin-left: 25px;
  margin-top: 10px;
`
