import styled from 'styled-components'

export const Container = styled.div`
`

export const ModalBody = styled.div`
`

export const TranslationText = styled.div`
`

export const Footer = styled.div`
  margin-top: 30px;
  font-weight: bold;
  font-size: 12px;
`

export const TranslateButton = styled.div`
  font-size: 13px;
  line-height: normal;
  color: rgba(3, 30, 40, 0.4);
  `

export const LangText = styled.span`
  font-weight: ${props => (props.selectedLang === props.type ? 'bold' : 'normal')};
  text-decoration: underline;
  font-size: 12px;
  cursor: pointer;
`
