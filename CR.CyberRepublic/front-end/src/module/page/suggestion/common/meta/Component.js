import React from 'react'
import moment from 'moment/moment'
import _ from 'lodash'
import I18N from '@/I18N'
import styled from 'styled-components'
import { bg, primary } from '@/constants/color'
import { CVOTE_STATUS, SUGGESTION_TAG_TYPE, SUGGESTION_STATUS } from '@/constant'

import {Link} from 'react-router-dom'

import './style.scss'

export default ({ data, hideAuthor, postedByText }) => {

  const { displayId, createdAt, tags, reference, status } = data
  let name = `${_.get(data, 'createdBy.profile.firstName', '')} ${_.get(data, 'createdBy.profile.lastName', '')}`
  if (name === ' ') {
    name = _.get(data, 'createdBy.username')
  }
  const author = data.author || name
  const authorNode = hideAuthor ? '' : (
    <span>
      {postedByText || I18N.get('suggestion.postedBy')}
      {' '}
      {author}
    </span>
  )

  let showReference = false
  let referenceId, referenceStatus
  let linkText

  if (!_.isEmpty(reference)) {

    const lastReference = _.last(reference)

    if (!_.isEmpty(lastReference) && lastReference.status !== CVOTE_STATUS.DRAFT) {
      showReference = true
      referenceId = lastReference._id
      referenceStatus = lastReference.status
      linkText = `${I18N.get('council.voting.proposal')} #${lastReference.vid}`
    }
  }

  return (
    <div>
      <LeftBox className="c_SuggestionMeta">
        <span>{`#${displayId}`}</span>
        {authorNode}
        <span>{moment(createdAt).format('MMM D, YYYY')}</span>
      </LeftBox>
      <RightBox>
        {
          tags.length ? _.map(tags, (tag) => {
            const { type, _id, desc } = tag
            return (
              type === SUGGESTION_TAG_TYPE.INFO_NEEDED ?
              <StatusBadge key={_id}>
                {I18N.get('suggestion.btnText.needMoreInfo')}
              </StatusBadge> :
              <StatusBadgeGood key={_id}>
                {I18N.get('suggestion.tag.type.UNDER_CONSIDERATION')}
              </StatusBadgeGood>
            )
          }) : ''
        }
        {
          showReference && <StatusBadge>
            {`${I18N.get('suggestion.referred')} `}
            <Link to={`/proposals/${referenceId}`}>{linkText}</Link>
            {` (${I18N.get(`cvoteStatus.${referenceStatus}`)})`}
          </StatusBadge>
        }
        {
          status === SUGGESTION_STATUS.ARCHIVED && <StatusBadgeBad>
            {I18N.get('suggestion.archived')}
          </StatusBadgeBad>
        }
      </RightBox>
      <ClearFix/>
    </div>
  )
}

const LeftBox = styled.div`
  float: left;
  width: 50%;
`

const RightBox = styled.div`
  overflow: hidden;
  text-align: right;
`

const StatusBadge = styled.div`
  display: inline-block;
  border: 1px solid ${bg.obsidian};
  color: white;
  background-color: ${bg.navy};
  border-radius: 4px;
  padding: 4px 12px;
  font-weight: 300;
  white-space: nowrap;
`

const StatusBadgeGood = styled(StatusBadge)`
  background-color: ${primary.default};
  border: 1px solid ${primary.light};
`

const StatusBadgeBad = styled(StatusBadge)`
  background-color: ${primary.danger};
  border: 1px solid ${bg.gray};
`


const ClearFix = styled.div`
  clear: both;
`
