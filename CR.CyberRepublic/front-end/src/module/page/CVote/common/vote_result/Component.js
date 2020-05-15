import React from 'react'
import PropTypes from 'prop-types'
import I18N from '@/I18N'
import _ from 'lodash'
import OnChain from '../../detail/OnChain'
import Translation from '@/module/common/Translation/Container'

import { Container, ResultRow, Reason, Label, List, Item, Avatar, StyledAvatarIcon } from './style'

const Component = (
    { 
      label, 
      type, 
      dataList,
      id,
      getReviewProposal,
      getReviewProposalUrl,
      voteResult, 
      isCouncil,
      currentUserId      
    }
  ) => {
  const votesNode = _.map(dataList, (data, key) => {
    let voteStatus = voteResult[key].status
    if(voteStatus == undefined || voteStatus == 'failed' || voteStatus == 'unchain') {
      voteStatus = I18N.get(`council.voting.chainStatus.unchain`)
    } 
    if( voteStatus == 'chained'){
      voteStatus = I18N.get(`council.voting.chainStatus.chained`)
    }
    if (voteStatus == 'chaining') {
       voteStatus = I18N.get(`council.voting.chainStatus.chaining`)
    }
    
    let isOwner = data.votedBy === currentUserId

    // const isReject = type === CVOTE_RESULT.REJECT
    const userNode = (
      <Item key={key}>
        {data.avatar ? <Avatar src={data.avatar} alt="voter avatar" /> : <StyledAvatarIcon />}
        <div>{data.name}</div>
        <div>{ isCouncil && data.reason != '' ? voteStatus : null }</div>
        <div style={{ marginTop: '0.5rem'}}>
          { 
          ( isCouncil && isOwner && data.reason != '' ) ? 
          <OnChain 
          getReviewProposal={getReviewProposal}
          getReviewProposalUrl={getReviewProposalUrl}
          id={id} 
          /> : null
          }
          </div>
      </Item>
    )
    
    const googleNode = data.reason && <div style={{ marginTop: '0.5rem'}}><Translation text={data.reason} /></div>
  
    // if (!isReject) return userNode
    // show reason for all vote type
    const reasonNode = (
      <Reason>
        {data.reason.split('\n').map((item) => {
          return (
            <span>
              {item}
              <br/>
            </span>
          )
        })}
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
