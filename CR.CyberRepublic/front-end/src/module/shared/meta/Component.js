import React from 'react'
import moment from 'moment/moment'
import I18N from '@/I18N'
import userUtil from '@/util/user'

import './style.scss'

export default ({ data, hideAuthor, postedByText }) => {
  const { displayId, createdAt, proposedAt } = data

  const authorNode = hideAuthor ? '' : (
    <span>
      {postedByText || I18N.get('suggestion.postedBy')}
      {' '}
      {userUtil.getUserDisplayName(data.proposer)}
    </span>
  )

  return (
    <div className="c_Meta">
      <span>{`#${displayId}`}</span>
      {authorNode}
      <span>{moment(proposedAt || createdAt).format('MMM D, YYYY')}</span>
    </div>
  )
}
