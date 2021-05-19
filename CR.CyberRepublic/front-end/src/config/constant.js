import React from 'react'

export const RECAPTCHA_KEY = '6LeH0DIUAAAAAMfp3kJQdiW0y-4VsIM-y53GRBBD'
export const MIN_LENGTH_PASSWORD = 8

// Responsive layout
export const MAX_WIDTH_MOBILE = 768
export const MIN_WIDTH_PC = 769
export const LG_WIDTH = 992

export const EMPOWER_MAX_BUSINESS = 4
export const EMPOWER_MAX_MARKETING = 4
export const EMPOWER_MAX_LEGAL = 2
export const EMPOWER_MAX_DESIGNER = 2
export const EMPOWER_MAX_VIDEOGRAPHER = 2
export const EMPOWER_MAX_WRITER_CONTENT = 3
export const EMPOWER_MAX_WRITER_TECHNICAL = 3
export const EMPOWER_MAX_DAPP_ANALYST = 5
export const EMPOWER_MAX_REGIONAL_EVANGELIST = 10
export const MAX_LENGTH_COMMENT = 2048

// medium-draft toolbar configs
export const MEDIUM_DRAFT_TOOLBAR_OPTIONS = {
  BLOCK_BUTTONS: [
    {
      label: 'H1',
      style: 'header-one',
      icon: 'header',
      description: 'Heading 1',
    },
    {
      label: 'H2',
      style: 'header-two',
      icon: 'header',
      description: 'Heading 2',
    },
    {
      label: 'H3',
      style: 'header-three',
      icon: 'header',
      description: 'Heading 3',
    },
    {
      label: 'H4',
      style: 'header-four',
      icon: 'header',
      description: 'Heading 4',
    },
    {
      label: 'H5',
      style: 'header-five',
      icon: 'header',
      description: 'Heading 5',
    },
    {
      label: 'H6',
      style: 'header-six',
      icon: 'header',
      description: 'Heading 6',
    },
    {
      label: (
        <svg width="10.83" height="10" viewBox="0 0 13 12">
          <g stroke="none" strokeWidth="1" fill="none" fillRule="evenodd">
            <g transform="translate(-357.000000, -255.000000)" fill="#FFFFFF">
              <g transform="translate(260.000000, 165.000000)">
                <g transform="translate(0.000000, 75.000000)">
                  <g transform="translate(19.000000, 0.000000)">
                    <path d="M90.500768,15 L91,15.56 C88.9631235,17.0533408 87.9447005,18.666658 87.9447005,20.4 C87.9447005,21.8800074 88.75012,23.1466614 90.3609831,24.2 L87.5453149,27 C85.9211388,25.7866606 85.109063,24.346675 85.109063,22.68 C85.109063,20.3199882 86.90628,17.7600138 90.500768,15 Z M83.3917051,15 L83.890937,15.56 C81.8540605,17.0533408 80.8356375,18.666658 80.8356375,20.4 C80.8356375,21.8800074 81.6344006,23.1466614 83.2319508,24.2 L80.4362519,27 C78.8120759,25.7866606 78,24.346675 78,22.68 C78,20.3199882 79.7972171,17.7600138 83.3917051,15 Z" />
                  </g>
                </g>
              </g>
            </g>
          </g>
        </svg>
      ),
      style: 'blockquote',
      icon: 'quote-right',
      description: 'Blockquote',
    },
    {
      label: 'UL',
      style: 'unordered-list-item',
      icon: 'list-ul',
      description: 'Unordered List',
    },
    {
      label: 'OL',
      style: 'ordered-list-item',
      icon: 'list-ol',
      description: 'Ordered List',
    },
    {
      label: '✓',
      style: 'todo',
      description: 'Todo List',
    },
  ],
  INLINE_BUTTONS: [
    {
      label: 'B',
      style: 'BOLD',
      icon: 'bold',
      description: 'Bold',
    },
    {
      label: 'I',
      style: 'ITALIC',
      icon: 'italic',
      description: 'Italic',
    },
    {
      label: 'U',
      style: 'UNDERLINE',
      icon: 'underline',
      description: 'Underline',
    },
    {
      label: 'Hi',
      style: 'HIGHLIGHT',
      description: 'Highlight selection',
    },
    {
      label: (
        <svg width="20" height="15" viewBox="0 0 14 14">
          <g stroke="none" strokeWidth="1" fill="none" fillRule="evenodd">
            <g transform="translate(-468.000000, -254.000000)" stroke="#FFFFFF">
              <g transform="translate(260.000000, 165.000000)">
                <g transform="translate(0.000000, 75.000000)">
                  <g transform="translate(19.000000, 0.000000)">
                    <g transform="translate(196.424621, 21.424621) rotate(45.000000) translate(-196.424621, -21.424621) translate(193.424621, 13.924621)">
                      <path d="M0.5,5.69098301 L0.5,2 C0.5,1.82069363 0.550664909,1.51670417 0.697213595,1.2236068 C0.927818928,0.762396132 1.32141313,0.5 2,0.5 L4,0.5 C4.67858687,0.5 5.07218107,0.762396132 5.3027864,1.2236068 C5.44933509,1.51670417 5.5,1.82069363 5.5,2 L5.5,6 C5.5,6.67858687 5.23760387,7.07218107 4.7763932,7.3027864 C4.53586606,7.42304998 4.28800365,7.47874077 4.1077327,7.49484936 L0.5,5.69098301 Z" />
                      <path d="M0.5,12.690983 L0.5,9 C0.5,8.82069363 0.550664909,8.51670417 0.697213595,8.2236068 C0.927818928,7.76239613 1.32141313,7.5 2,7.5 L4,7.5 C4.67858687,7.5 5.07218107,7.76239613 5.3027864,8.2236068 C5.44933509,8.51670417 5.5,8.82069363 5.5,9 L5.5,13 C5.5,13.6785869 5.23760387,14.0721811 4.7763932,14.3027864 C4.53586606,14.42305 4.28800365,14.4787408 4.1077327,14.4948494 L0.5,12.690983 Z" transform="translate(3.000000, 11.000000) scale(-1, -1) translate(-3.000000, -11.000000) " />
                    </g>
                  </g>
                </g>
              </g>
            </g>
          </g>
        </svg>
      ),
      style: 'hyperlink',
      icon: 'link',
      description: 'Add a link',
    },
  ]
}

export const LINKIFY_OPTION = {
  defaultProtocol: 'https',
  target: {
    url: '_self' // Does not work for some reason
  }
}
export const LANGUAGES = {
  english: 'en',
  chinese: 'zh',
}
