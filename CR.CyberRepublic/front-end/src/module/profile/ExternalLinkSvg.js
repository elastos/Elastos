import React from "react"

function ExternalLinkSvg(props) {
  return (
    <svg width={12} height={12} viewBox="0 0 12 12" fill="none" {...props}>
      <path
        d="M10.667 10.667H1.333V1.333H6V0H0v12h12V6h-1.333v4.667zM7.333 0v1.333h2.394L3.173 7.887l.94.94 6.554-6.554v2.394H12V0H7.333z"
        fill="#008D85"
      />
    </svg>
  )
}

export default ExternalLinkSvg
