import React from 'react';
import moment from 'moment/moment'
import _ from 'lodash'
import I18N from '@/I18N'

import './style.scss'

export default ({ data, hideAuthor, postedByText }) => {
  const { displayId, createdAt, descUpdatedAt } = data;
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

  return (
    <div className="c_SuggestionMeta">
      <span>{`#${displayId}`}</span>
      {authorNode}
      <span>{moment(createdAt).format('MMM D, YYYY')}</span>

      {/* there is a hack here, we set descUpdatedAt always so we only are sorting on that field, remove this when we properly sort on a projected field */}
      {descUpdatedAt && (!moment(descUpdatedAt).isSame(createdAt, 'day')) && <span>updated: {moment(descUpdatedAt).format('MMM D, YYYY')}</span>}
    </div>
  )
}
