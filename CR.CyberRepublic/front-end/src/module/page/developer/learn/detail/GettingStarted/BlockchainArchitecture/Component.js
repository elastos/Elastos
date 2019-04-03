import React from 'react'
import MergedMining from './merged_mining.png'
import P2P from './p2p.png'
import Chains from './chains.png'
import SideChains from './side.png'
import SideChains2 from './side2.png'
import SideChains3 from './side3.png'
import I18N from '@/I18N'

import '../../style.scss'

export default class extends React.Component {

  render () {
    const part3 = I18N.get('getting_started.part3')
    const part4 = I18N.get('getting_started.part4')
    const part5 = I18N.get('getting_started.part5')
    const part6 = I18N.get('getting_started.part6')
    const part7 = I18N.get('getting_started.part7')
    const part8 = I18N.get('getting_started.part8')
    const part9 = I18N.get('getting_started.part9')

    return (
      <div className="p_developerLearnDetail">
        <span dangerouslySetInnerHTML={{__html: part3}} />
        <img src={MergedMining} />
        <span dangerouslySetInnerHTML={{__html: part4}} />
        <img src={P2P} />
        <span dangerouslySetInnerHTML={{__html: part5}} />
        <img src={Chains} />
        <span dangerouslySetInnerHTML={{__html: part6}} />
        <img src={SideChains} />
        <span dangerouslySetInnerHTML={{__html: part7}} />
        <img src={SideChains2} />
        <span dangerouslySetInnerHTML={{__html: part8}} />
        <img src={SideChains3} />
        <span dangerouslySetInnerHTML={{__html: part9}} />
      </div>
    )
  }
}
