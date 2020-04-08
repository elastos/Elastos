import React from 'react'
import moment from 'moment/moment'
import _ from 'lodash'
import I18N from '@/I18N'
import PopoverProfile from '@/module/common/PopoverProfile'
import './style.scss'

export default ({ data, hideAuthor, postedByText, user, content }) => {
  const { version, createdAt, descUpdatedAt, createdBy } = data
  const authorNode = hideAuthor ? (
    ''
  ) : (
    <span>
      {postedByText || I18N.get('suggestion.postedBy')}
      {' '}
      <PopoverProfile owner={createdBy} curUser={user} />
    </span>
  )

  return (
    <div className="c_SuggestionHistoryMeta">
      <span>
        Version
        {' '}
        {`${version / 10}${version % 10 === 0 ? '.0' : ''}`}
      </span>
      <span>
        {I18N.get('suggestion.fields.preambleSub.created')}
        {' '}
        {moment(createdAt).format('MMM D, YYYY HH:mm A')}
      </span>

      {content}
      {/**
        * there is a hack here,
        * we set descUpdatedAt always so we only are sorting on that field,
        * remove this when we properly sort on a projected field
          {descUpdatedAt && !moment(descUpdatedAt).isSame(createdAt, 'day') && (
          <span>
          {I18N.get('suggestion.fields.preambleSub.updated')}
          {' '}
          {moment(descUpdatedAt).format('MMM D, YYYY')}
          </span>
          )}
        */}
    </div>
  )
}
