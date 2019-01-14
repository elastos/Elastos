import React from 'react';
import moment from 'moment/moment'
import _ from 'lodash'
import I18N from '@/I18N'

import './style.scss'

export default ({ data, hideAuthor }) => {
  const { displayId, createdAt } = data;
  let name = `${_.get(data, 'createdBy.profile.firstName', '')} ${_.get(data, 'createdBy.profile.lastName', '')}`
  if (name === ' ') {
    name = _.get(data, 'createdBy.username')
  }
  const author = data.author || name
  const authorNode = hideAuthor ? '' : (
    <span>
      {I18N.get('suggestion.postedBy')}
      {' '}
      {author}
    </span>
  )

  return (
    <div className="c_SuggestionMeta">
      <span>{`#${displayId}`}</span>
      {authorNode}
      <span>{moment(createdAt).format('MMM D, YYYY')}</span>
    </div>
  )
}
