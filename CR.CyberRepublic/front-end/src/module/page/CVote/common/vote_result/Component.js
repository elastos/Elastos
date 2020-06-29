import React from 'react'
import PropTypes from 'prop-types'
import I18N from '@/I18N'
import _ from 'lodash'
import OnChain from '../../detail/OnChain'
import Translation from '@/module/common/Translation/Container'
import { Avatar } from 'antd'
import { USER_AVATAR_DEFAULT } from '@/constant'
import { Container, ResultRow, Reason, Label, List, Item } from './style'

const Component = ({
  label,
  type,
  dataList,
  id,
  getReviewProposal,
  getReviewProposalUrl,
  updateProposal,
  isProposed,
  isCouncil,
  currentUserId
}) => {
  const votesNode = _.map(dataList, (data, key) => {
    let voteStatus = data.status
    if (
      voteStatus == undefined ||
      voteStatus == 'failed' ||
      voteStatus == 'unchain'
    ) {
      voteStatus = I18N.get(`council.voting.chainStatus.unchain`)
    }
    if (voteStatus == 'chained') {
      voteStatus = I18N.get(`council.voting.chainStatus.chained`)
    }
    if (voteStatus == 'chaining') {
      voteStatus = I18N.get(`council.voting.chainStatus.chaining`)
    }
    let isOwner = data.votedBy && data.votedBy === currentUserId
    const avatarName = data.name.split(' ')
    const userNode = (
      <Item key={key}>
        {data.avatar || avatarName[0] == 'undefined' ? (
          <Avatar
            size={64}
            src={data.avatar || USER_AVATAR_DEFAULT}
            alt="voter avatar"
          />
        ) : (
          <Avatar
            className="comment-avatar pull-left"
            style={{
              backgroundColor: '#000',
              fontSize: 24
            }}
            shape="circle"
            size={64}
          >
            {`${avatarName[0] &&
              avatarName[0].toUpperCase().substr(0, 1)}${avatarName[1] &&
              avatarName[1].toUpperCase().substr(0, 1)}`}
          </Avatar>
        )}
        <div>{data.didName}</div>
        <div className="status">{data.reason !== '' ? voteStatus : null}</div>
        <div style={{ marginTop: '0.5rem' }}>
          {isProposed &&
          isCouncil &&
          isOwner &&
          (data.status === 'unchain' || data.status === undefined) &&
          data.reason !== '' ? (
            <OnChain
              getReviewProposal={getReviewProposal}
              getReviewProposalUrl={getReviewProposalUrl}
              updateProposal={updateProposal}
              id={id}
            />
          ) : null}
        </div>
      </Item>
    )

    const googleNode = data.reason && (
      <div style={{ marginTop: '0.5rem' }}>
        <Translation text={data.reason} />
      </div>
    )

    // if (!isReject) return userNode
    // show reason for all vote type
    const reasonNode = (
      <Reason>
        {data.reason.split('\n').map((item, key) => {
          return (
            <span key={key}>
              {item}
              <br />
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
  id: PropTypes.string.isRequired,
  getReviewProposal: PropTypes.func.isRequired,
  getReviewProposalUrl: PropTypes.func.isRequired,
  updateProposal: PropTypes.func.isRequired,
  isCouncil: PropTypes.bool.isRequired,
  isProposed: PropTypes.bool.isRequired,
  currentUserId: PropTypes.string.isRequired
}

Component.propTypes = propTypes

export default Component
