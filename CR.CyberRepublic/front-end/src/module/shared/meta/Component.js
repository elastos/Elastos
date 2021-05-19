import React from 'react'
import moment from 'moment/moment'
import I18N from '@/I18N'
import PopoverProfile from '@/module/common/PopoverProfile'
import './style.scss'

export default ({ data, hideAuthor, postedByText, user }) => {
  const { displayId, createdAt, proposedAt } = data

  const authorNode = hideAuthor ? '' : (
    <span>
      {postedByText || I18N.get('meta.postedBy')}{' '}
      <PopoverProfile owner={data.proposer} curUser={user} />
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
