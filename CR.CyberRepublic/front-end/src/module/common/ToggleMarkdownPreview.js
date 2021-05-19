import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import styled from 'styled-components'

class ToggleMarkdownPreview extends BaseComponent {
  onClick = e => {
    this.props.togglePreview()
  }

  ord_render() {
    return (
      <Wrapper>
        <label onClick={this.onClick}>
          <figure>
            <svg
              xmlns="http://www.w3.org/2000/svg"
              viewBox="0 0 24 24"
              class="icon"
            >
              <path d="M 11.9994,8.99813C 10.3424,8.99813 8.99941,10.3411 8.99941,11.9981C 8.99941,13.6551 10.3424,14.9981 11.9994,14.9981C 13.6564,14.9981 14.9994,13.6551 14.9994,11.9981C 14.9994,10.3411 13.6564,8.99813 11.9994,8.99813 Z M 11.9994,16.9981C 9.23841,16.9981 6.99941,14.7591 6.99941,11.9981C 6.99941,9.23714 9.23841,6.99813 11.9994,6.99813C 14.7604,6.99813 16.9994,9.23714 16.9994,11.9981C 16.9994,14.7591 14.7604,16.9981 11.9994,16.9981 Z M 11.9994,4.49813C 6.99741,4.49813 2.72741,7.60915 0.99941,11.9981C 2.72741,16.3871 6.99741,19.4981 11.9994,19.4981C 17.0024,19.4981 21.2714,16.3871 22.9994,11.9981C 21.2714,7.60915 17.0024,4.49813 11.9994,4.49813 Z "></path>
            </svg>
          </figure>
        </label>
      </Wrapper>
    )
  }
}

export default ToggleMarkdownPreview

const Wrapper = styled.div`
  label {
    cursor: pointer;
    display: inline-block;
    z-index: 99;
    overflow: hidden;
    margin-left: 8px;
  }

  label figure {
    width: 20px;
    height: 20px;
    background-color: #008d85;
    border-radius: 50%;
    display: flex;
    justify-content: center;
    align-items: center;
  }

  label svg {
    fill: #fff;
    width: 14px;
    height: 14px;
  }
`
