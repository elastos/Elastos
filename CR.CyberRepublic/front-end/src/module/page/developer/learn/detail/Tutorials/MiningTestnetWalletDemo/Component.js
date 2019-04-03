import React from 'react'
import '../../style.scss'

export default class extends React.Component {

  render () {
    return (
      <div className="p_developerLearnDetail">
        <h3>Part 1</h3>
        <div className="videoWrapper">
          <iframe src="https://www.youtube.com/embed/yKQHflT8Kuo" frameBorder="0" allow="autoplay; encrypted-media" allowFullScreen={true} />
        </div>
        <h3>Part 2</h3>
        <div className="videoWrapper">
          <iframe src="https://www.youtube.com/embed/HzrnhrU_i78" frameBorder="0" allow="autoplay; encrypted-media" allowFullScreen={true} />
        </div>
        <h3>Part 3</h3>
        <div className="videoWrapper">
          <iframe src="https://www.youtube.com/embed/hSMuTUp_bWo" frameBorder="0" allow="autoplay; encrypted-media" allowFullScreen={true} />
        </div>
        <h3>Part 4</h3>
        <div className="videoWrapper">
          <iframe src="https://www.youtube.com/embed/a-IIlgY4Jwc" frameBorder="0" allow="autoplay; encrypted-media" allowFullScreen={true} />
        </div>
      </div>
    )
  }
}
