import React from 'react'
import '../../style.scss'

export default class extends React.Component {

  render () {
    return (
      <div className="p_developerLearnDetail">
        <h3>Part 1</h3>
        <div className="videoWrapper">
          <iframe src="https://www.youtube.com/embed/N0dnpN8suv0" frameBorder="0" allow="autoplay; encrypted-media" allowFullScreen={true} />
        </div>
        <h3>Part 2</h3>
        <div className="videoWrapper">
          <iframe src="https://www.youtube.com/embed/IwacFZgYOEc" frameBorder="0" allow="autoplay; encrypted-media" allowFullScreen={true} />
        </div>
      </div>
    )
  }
}
