import React from 'react'
import BaseComponent from '@/model/BaseComponent'
import { message } from 'antd'
import I18N from '@/I18N'
import styled from 'styled-components'
import PropTypes from 'prop-types'

const IMAGE_SIZE = {
  MAX_WIDTH: 720,
  MAX_HEIGHT: 720
}

function resizeImage(img) {
  const canvas = document.createElement('canvas')

  let width = img.width
  let height = img.height

  // calculate the width and height, constraining the proportions
  if (width > height) {
    if (width > IMAGE_SIZE.MAX_WIDTH) {
      height = Math.round((height *= IMAGE_SIZE.MAX_WIDTH / width))
      width = IMAGE_SIZE.MAX_WIDTH
    }
  } else {
    if (height > IMAGE_SIZE.MAX_HEIGHT) {
      width = Math.round((width *= IMAGE_SIZE.MAX_HEIGHT / height))
      height = IMAGE_SIZE.MAX_HEIGHT
    }
  }

  // resize the canvas and draw the image data into it
  canvas.width = width
  canvas.height = height
  const ctx = canvas.getContext('2d')
  ctx.drawImage(img, 0, 0, width, height)

  // get the base64 data from canvas as 70% JPG
  return canvas.toDataURL('image/jpeg', 0.7)
}

class UploadBase64Image extends BaseComponent {
  onChange = e => {
    const file = e.target.files[0]
    if (!file) {
      return
    }
    // check if the uploaded file is an image
    if (file.type && !file.type.includes('image/')) {
      message.error(I18N.get('image.upload.type.error'))
      return
    }

    if (file.size > 502400) {
      message.error(I18N.get('image.upload.size.error'))
      return
    }

    const blobURL = URL.createObjectURL(file)
    const image = new Image()
    image.src = blobURL

    image.onload = async () => {
      // send it to canvas to compress and convert format
      const base64 = resizeImage(image)
      // release the blob url
      URL.revokeObjectURL(blobURL)
      this.props.insertImage(base64)
    }
  }

  ord_render() {
    const { name } = this.props
    return (
      <Wrapper>
        <input
          type="file"
          id={name}
          className="upload-base64"
          onChange={this.onChange}
        />
        <label htmlFor={name}>
          <figure>
            <svg
              xmlns="http://www.w3.org/2000/svg"
              viewBox="0 0 24 24"
              class="icon"
            >
              <path d="M 12.9994,8.99807L 18.4994,8.99807L 12.9994,3.49807L 12.9994,8.99807 Z M 5.99938,1.99809L 13.9994,1.99809L 19.9994,7.99808L 19.9994,19.9981C 19.9994,21.1021 19.1034,21.9981 17.9994,21.9981L 5.98937,21.9981C 4.88537,21.9981 3.99939,21.1021 3.99939,19.9981L 4.0094,3.99808C 4.0094,2.89407 4.89437,1.99809 5.99938,1.99809 Z M 6,20L 15,20L 18,20L 18,12L 14,16L 12,14L 6,20 Z M 8,9C 6.89543,9 6,9.89543 6,11C 6,12.1046 6.89543,13 8,13C 9.10457,13 10,12.1046 10,11C 10,9.89543 9.10457,9 8,9 Z "></path>
            </svg>
          </figure>
        </label>
      </Wrapper>
    )
  }
}

UploadBase64Image.propTypes = {
  name: PropTypes.string.isRequired,
  insertImage: PropTypes.func.isRequired
}

export default UploadBase64Image

const Wrapper = styled.div`
  .upload-base64 {
    width: 0.1px;
    height: 0.1px;
    opacity: 0;
    overflow: hidden;
    z-index: -1;
  }

  .upload-base64 + label {
    cursor: pointer;
    display: inline-block;
    overflow: hidden;
    z-index: 99;
  }

  .upload-base64 + label figure {
    width: 20px;
    height: 20px;
    background-color: #008d85;
    border-radius: 50%;
    display: flex;
    justify-content: center;
    align-items: center;
  }

  .upload-base64 + label svg {
    fill: #fff;
    width: 14px;
    height: 14px;
  }
`
