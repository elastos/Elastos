import React from 'react'
import PropTypes from 'prop-types'
import _ from 'lodash'
import styled from 'styled-components'
import Translation from '@/module/common/Translation/Container'
import { breakPoint } from '@/constants/breakPoint'
import { ELIP_VOTE_STATUS_COLOR } from '@/constant'
import AvatarIcon from './AvatarIcon'

const Component = ({ label, type, dataList }) => {
  const votesNode = _.map(dataList, (data, key) => {
    const userNode = (
      <Item key={key}>
        {data.avatar ? <Avatar src={data.avatar} alt="voter avatar" /> : <StyledAvatarIcon />}
        <div>{data.name}</div>
      </Item>
    )

    const googleNode = data.reason && <div style={{ marginTop: '0.5rem'}}><Translation text={data.reason} /></div>

    // if (!isReject) return userNode
    // show reason for all vote type
    const reasonNode = (
      <Reason>
        {data.reason}
        {googleNode}
      </Reason>
    )

    return (
      <ResultRow key={key}>
        {userNode}
        {reasonNode}
      </ResultRow>
    )
  })

  return (
    <Container>
      <Label>{label}</Label>
      <List type={type}>{votesNode}</List>
    </Container>
  )
}

const propTypes = {
  label: PropTypes.string.isRequired,
  type: PropTypes.string.isRequired,
  dataList: PropTypes.array.isRequired,
}

Component.propTypes = propTypes

export default Component


const Container = styled.div`
  display: flex;
  align-items: center;
  min-height: 220px;
  margin-bottom: 10px;
  &:last-child :last-child:after {
    border-bottom: none;
  }
`

const Label = styled.div`
  text-align: right;
  margin-right: 50px;
  flex: 0 0 90px;
  @media only screen and (max-width: ${breakPoint.mobile}) {
    flex: 0 0 60px;
    margin-right: 3px;
    font-size: 0.8em;
  }
`

const List = styled.div`
  position: relative;
  padding: 40px;
  display: block;
  align-items: center;
  box-sizing: border-box;
  height: 100%;
  border-left: 10px solid;
  border-color: ${props => ELIP_VOTE_STATUS_COLOR[props.type]};
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

const Item = styled.div`
  text-align: center;
  margin-left: 28px;
  @media only screen and (max-width: ${breakPoint.mobile}) {
    margin-left: 5px;
  }
`

const Avatar = styled.img`
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

const StyledAvatarIcon = styled(AvatarIcon)`
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

const ResultRow = styled.div`
  display: flex;
  margin-bottom: 30px;
`

const Reason = styled.div`
  margin-left: 25px;
  margin-top: 10px;
  @media only screen and (max-width: ${breakPoint.mobile}) {
    margin-left: 10px;
    margin-top: 0px;
  }
`
