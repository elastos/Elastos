import styled from 'styled-components'
import { Link } from 'react-router-dom'

export const StyledLink = styled(Link)`
  display: flex;
  position: absolute;
  left: 30px;
  height: 16px;
  line-height: 16px;
  align-items: center;
  color: #000;
`
export const Arrow = styled.img`
  transform: rotate(180deg);
  width: 10px;
`
export const Text = styled.span`
  margin-left: 10px;
`
