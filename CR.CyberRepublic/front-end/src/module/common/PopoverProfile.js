import React from 'react'
import { Popover, Button } from 'antd'
import _ from 'lodash'
import I18N from '@/I18N'
import { Link } from 'react-router-dom'
import styled from 'styled-components'
import userUtil from '@/util/user'

export default ({ owner, curUser }) => {
  const name = userUtil.formatUsername(owner)
  const userId = _.get(owner, '_id')
  const email = _.get(owner, 'email')
  const isAuthor = curUser.current_user_id === userId
  const did = _.get(owner, 'did')
  const content = (
    <PopoverContent>
      <div>
        <span className="label email">
          {I18N.get('profile.popover.email')}:
        </span>
        <span className="value email">{email}</span>
        <Button
          type="primary"
          onClick={() => {
            let el = document.createElement('input')
            document.body.appendChild(el)
            el.value = email
            el.select()
            document.execCommand('copy')
            document.body.removeChild(el)
          }}
        >
          {I18N.get('profile.popover.copy')}
        </Button>
      </div>
      {did && did.id && (
        <div>
          <span className="label">{I18N.get('profile.popover.did')}:</span>
          <span className="value">{did.id}</span>
        </div>
      )}
      <div>
        <span className="label">{I18N.get('profile.popover.name')}:</span>
        <span className="value">{name}</span>
      </div>
      {(isAuthor ||
        curUser.is_admin ||
        curUser.is_secretary ||
        curUser.is_council) && (
        <Link to={`/member/${userId}`}>
          {I18N.get('profile.popover.viewProfile')}
        </Link>
      )}
    </PopoverContent>
  )
  return (
    <Popover content={content} trigger="click" placement="top">
      <ItemTextName>{name}</ItemTextName>
    </Popover>
  )
}

const PopoverContent = styled.div`
  padding: 16px 16px 8px;
  > div {
    display: flex;
    align-items: center;
    margin-bottom: 8px;
    .label {
      width: 60px;
      flex-shrink: 0;
    }
    .value {
      color: rgba(0, 0, 0, 0.45);
    }
    .value.email {
      background: #f5f5f5;
      padding: 6px;
    }
  }
  a {
    display: inline-block;
    margin-top: 16px;
  }
`

export const ItemTextName = styled.a`
  font-weight: 200;
`
