import styled from 'styled-components'
import { CVOTE_RESULT, CVOTE_RESULT_COLOR } from '@/constant'
import { breakPoint } from '@/constants/breakPoint'
import AvatarIcon from './AvatarIcon'

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
  @media only screen and (max-width: ${breakPoint.mobile}) {
    flex: 0 0 60px;
    margin-right: 3px;
    font-size: 0.8em;
  }
`

export const List = styled.div`
  position: relative;
  padding: 40px 20px;
  display: block;
  align-items: center;
  box-sizing: border-box;
  height: 100%;
  border-left: 10px solid;
  border-color: ${props => CVOTE_RESULT_COLOR[props.type]};
  &:after {
    content: " ";
    min-width: 500px;
    position: absolute;
    left: 5px;
    right: 5px;
    bottom: -7px;
    border-bottom: 1px solid #E5E5E5;
  }
  @media only screen and (max-width: ${breakPoint.mobile}) {
    padding: 5px;
  }
`

export const Item = styled.div`
  width: 200px;
  flex-shrink: 0;
  display: flex;
  flex-direction: column;
  align-items: center;
  text-align: center;
  .status {
    color: rgba(3,30,40,0.4);
    font-size: 13px;
    margin-top: 8px;
    font-weight: 600;
  }
`

export const Avatar = styled.img`
  display: block;
  border-radius: 50%;
  width: 100px;
  height: 100px;
  margin-bottom: 15px;
  @media only screen and (max-width: ${breakPoint.mobile}) {
    width: 50px;
    height: 50px;
  }
`

export const StyledAvatarIcon = styled(AvatarIcon)`
  display: block;
  fill: #ddd;
  width: 100px;
  height: 100px;
  margin-bottom: 15px;
  @media only screen and (max-width: ${breakPoint.mobile}) {
    width: 50px;
    height: 50px;
  }
`

export const ResultRow = styled.div`
  display: flex;
  margin-bottom: 30px;
`

export const Reason = styled.div`
  margin-left: 25px;
  margin-top: 10px;
  @media only screen and (max-width: ${breakPoint.mobile}) {
    margin-left: 10px;
    margin-top: 0px;
  }
`
