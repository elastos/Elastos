import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import I18N from '@/I18N'
import {Row, Col} from 'antd'
import _ from 'lodash'
import { DefaultPlayer as Video } from 'react-html5video'
import 'react-html5video/dist/styles.css'

import './style.scss'

export default class extends BaseComponent {
  ord_states() {
    return {
      persist: true,
    }
  }

  ord_render() {
    return (
      <Video autoPlay={true} loop={true}
        controls={['PlayPause', 'Seek', 'Time', 'Volume', 'Fullscreen']}
        onCanPlayThrough={() => {
          // Do stuff
        }}>
        <source src="/assets/video/Elastos.webm" type="video/webm" />
        <source src="/assets/video/Elastos.ogv" type="video/ogg" />
        <source src="/assets/video/Elastos.mp4" type="video/mp4" />
      </Video>
    )
  }
}
