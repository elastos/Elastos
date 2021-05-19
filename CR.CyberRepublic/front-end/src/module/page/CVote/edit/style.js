import styled from 'styled-components'
import { grid } from '../common/variable'

export const Container = styled.div`
  padding: 0 50px 80px;
  width: 80vw;
  margin: 80px auto 0;
  background: #ffffff;
  text-align: left;
  @media only screen and (max-width: ${grid.sm}) {
    margin: 15px;
  }
`
