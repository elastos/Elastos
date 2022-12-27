import React from 'react'

const Square = props => (
  <svg width={156} height={156} fill="none" {...props}>
    <path
      d="M50.877 50.485H156v105.228H50.877V50.485zM.444 0h50.432v50.484H.444V0z"
      fill="#172C3B"
    />
    <g opacity={0.6} stroke="#1DE9B6" strokeWidth={1.25} strokeMiterlimit={10}>
      <path
        opacity={0.6}
        d="M129.273 77.239l-51.669 51.72M129.273 91.065l-37.854 37.894M129.273 104.891l-24.041 24.067M129.273 118.721l-10.228 10.238M87.833 77.238L77.604 87.477M101.646 77.238l-24.042 24.066M115.459 77.238l-37.855 37.893"
      />
    </g>
  </svg>
)

export default Square
