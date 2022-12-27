import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import 'react-circular-progressbar/dist/styles.css'
import { CircularProgressbar, buildStyles } from 'react-circular-progressbar'

export default class Component extends BaseComponent {
  ord_render() {
    const { count, limit = 200 } = this.props
    let percentage = (100 * count) / limit
    if (percentage > 100) percentage = 100
    let pathColor = '#1DE9B6'
    let text
    if (limit - count <= 20) {
      text = (
        <span
          style={{
            color: 'rgba(3, 30, 40, 0.3)',
            position: 'absolute',
            right: 23,
            top: 2
          }}
        >
          {limit - count}
        </span>
      )
      pathColor = limit - count > 0 ? '#FF9518' : '#BE1313'
    }

    return (
      <span>
        {text}
        <CircularProgressbar
          value={percentage}
          text={null}
          styles={buildStyles({
            // Rotation of path and trail, in number of turns (0-1)
            rotation: 0.25,

            // Whether to use rounded or flat corners on the ends - can use 'butt' or 'round'
            strokeLinecap: 'butt',

            // Text size
            // textSize: '16px',

            // How long animation takes to go from one percentage to another, in seconds
            pathTransitionDuration: 0.5,

            // Can specify path transition in more detail, or remove it entirely
            // pathTransition: 'none',

            // Colors
            pathColor,
            // textColor: '#f88',
            trailColor: '#d6d6d6'
            // backgroundColor: '#3e98c7',
          })}
        />
      </span>
    )
  }
}
