import React from 'react'

const GenderSvg = props => (
  <svg width={14} height={12} fill="none" {...props}>
    <path
      fillRule="evenodd"
      clipRule="evenodd"
      d="M13.495 10.75l-2.598-4.5a.5.5 0 00-.866 0l-2.598 4.5a.5.5 0 00.433.75h5.196a.5.5 0 00.433-.75zm-1.299-.25l-1.732-3-1.732 3h3.464z"
      fill="#031E28"
    />
    <circle cx={10.5} cy={2.5} r={2} stroke="#031E28" />
    <path
      fillRule="evenodd"
      clipRule="evenodd"
      d="M7.031 6.75l-2.598 4.5a.5.5 0 01-.866 0L.969 6.75A.5.5 0 011.402 6h5.196a.5.5 0 01.433.75zM5.732 7L4 10 2.268 7h3.464z"
      fill="#031E28"
    />
    <circle cx={4.1} cy={2.5} r={2} stroke="#031E28" />
  </svg>
)

export default GenderSvg
