import styled from 'styled-components'
import { getSassColor } from '@/util/styled-components'

// Extract Sass variables into a JS object
const styleObj = require('sass-extract-loader!@/style/color.scss')

export const Container = styled.div`
  background: ${getSassColor(styleObj, '$bg_lighter_gray')};
  padding: 30px;
  margin-bottom: 30px;
  h2 {
    font-size: 30px;
  }
`

export const List = styled.div`
  margin-bottom: 20px;
  min-height: 550px;
`
