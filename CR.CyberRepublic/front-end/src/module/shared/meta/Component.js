import React from 'react'
import moment from 'moment/moment'
import _ from 'lodash'
import I18N from '@/I18N'

import './style.scss'

export default ({ data, hideAuthor, postedByText }) => {
  const { displayId, createdAt, proposedAt } = data
  const author = data.author || `${_.get(data, 'createdBy.profile.firstName', '')} ${_.get(data, 'createdBy.profile.lastName', '')}` || data.username
  const authorNode = hideAuthor ? '' : (
    <span>
      {postedByText || I18N.get('suggestion.postedBy')}
      {' '}
      {author}
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
