import React from 'react'

function CloseIcon(props) {
  return (
    <svg width={10} height={9} viewBox="0 0 10 9" fill="none" {...props}>
      <g opacity={0.5} stroke="#000">
        <path transform="rotate(44.999 -.707 1.707)" d="M0-.5h11.313" />
        <path transform="rotate(-44.999 11.633 2.645)" d="M0-.5h11.313" />
      </g>
    </svg>
  )
}

export default CloseIcon
