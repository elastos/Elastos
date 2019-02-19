import React from 'react'
import PropTypes from 'prop-types'
import I18N from '@/I18N'

import { StyledLink, Arrow, Text } from './style'

const Component = ({ link }) => (
  <StyledLink to={link}>
    <Arrow src="/assets/images/arrow.svg" />
    <Text>{I18N.get('link.back')}</Text>
  </StyledLink>
)

const propTypes = {
  link: PropTypes.string.isRequired,
}

Component.propTypes = propTypes


export default Component
