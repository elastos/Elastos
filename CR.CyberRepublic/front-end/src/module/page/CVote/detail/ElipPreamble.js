import React from 'react'
import PropTypes from 'prop-types'
import I18N from '@/I18N'
import _ from 'lodash'

import {
  Part,
  PartTitle,
  PartContent
} from './style'

const Component = ({
  vid: elip,
  title,
  username: author,
  discussions,
  status,
  elipType: type,
  createdAt: created,
  requires,
  replaces,
  superseded
}) => {
  const preambles = {elip, title, author, discussions, status, type, created, requires, replaces, superseded}
  return (
    <Part id="preamble">
      <PartTitle>{I18N.get('elip.fields.preamble')}</PartTitle>
      <PartContent className="preamble">
        {_.map(preambles, (v, k) => !_.isEmpty(v) && this.renderPreambleItem(I18N.get(`elip.fields.preambleItems.${k}`), v))}
      </PartContent>
    </Part>
  )
}

const propTypes = {
  vid: PropTypes.number.isRequired,
  title: PropTypes.string.isRequired,
  username: PropTypes.string.isRequired,
  discussions: PropTypes.string,
  status: PropTypes.string.isRequired,
  elipType: PropTypes.string.isRequired,
  createdAt: PropTypes.string.isRequired,
  requires: PropTypes.string,
  replaces: PropTypes.string,
  superseded: PropTypes.string
}

Component.propTypes = propTypes

export default Component
