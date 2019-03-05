import styled from 'styled-components'
import { grid } from '../common/variable'

export const Container = styled.div`
  text-align: center;
  @media only screen and (max-width: ${grid.sm}) {
    margin: 15px;
  }
`
