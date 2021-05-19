import React from 'react'
import I18N from '@/I18N'
import '../../style.scss'

export default class extends React.Component {

  render () {
    const part11 = I18N.get('getting_started.part11')

    return (
      <div className="p_developerLearnDetail" dangerouslySetInnerHTML={{__html: part11}} />
    )
  }
}
