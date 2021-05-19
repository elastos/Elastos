import React from 'react'
import ElastosArchitecture from './elastos_architecture.png'
import '../../style.scss'
import I18N from '@/I18N'

export default class extends React.Component {

  render () {
    const part1 = I18N.get('getting_started.part1')
    const part2 = I18N.get('getting_started.part2')
    return (
      <div className="p_developerLearnDetail">
        <span dangerouslySetInnerHTML={{__html: part1}} />

        <img src={ElastosArchitecture} />
        <span dangerouslySetInnerHTML={{__html: part2}} />
      </div>
    )
  }
}
