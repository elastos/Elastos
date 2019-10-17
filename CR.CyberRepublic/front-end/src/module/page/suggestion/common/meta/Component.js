import React from 'react'
import moment from 'moment/moment'
import _ from 'lodash'
import I18N from '@/I18N'
import PopoverProfile from '../PopoverProfile'
import './style.scss'

export default ({ data, hideAuthor, postedByText }) => {
  const { displayId, createdAt, descUpdatedAt } = data
  const authorNode = hideAuthor ? '' : (
    <span>
      {postedByText || I18N.get('suggestion.postedBy')} <PopoverProfile data={data} meta={true} />
    </span>
  )

  return (
    <div className="c_SuggestionMeta">
      <span>{`#${displayId}`}</span>
      {authorNode}
      <span>
        {I18N.get('suggestion.fields.preambleSub.created')}{' '}
        {moment(createdAt).format('MMM D, YYYY')}
      </span>

      {/* there is a hack here, we set descUpdatedAt always so we only are sorting on that field, remove this when we properly sort on a projected field */}
      {descUpdatedAt && !moment(descUpdatedAt).isSame(createdAt, 'day') && (
        <span>
          {I18N.get('suggestion.fields.preambleSub.updated')}{' '}
          {moment(descUpdatedAt).format('MMM D, YYYY')}
        </span>
      )}
    </div>
  )
}
