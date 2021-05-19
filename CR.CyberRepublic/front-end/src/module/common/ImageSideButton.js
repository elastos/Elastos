import React from 'react'
import { addNewBlock, ImageSideButton, Block } from 'medium-draft'
import { message } from 'antd'
import I18N from '@/I18N'
import { logger } from '@/util'

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

class ImageButton extends ImageSideButton {
  onChange(e) {
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

    image.onload = () => {
      // send it to canvas to compress and convert format
      const resized = resizeImage(image)

      // release the blob url
      URL.revokeObjectURL(blobURL)

      try {
        this.props.setEditorState(
          addNewBlock(this.props.getEditorState(), Block.IMAGE, {
            src: resized
          })
        )
      } catch (error) {
        logger.error(error)
      }
    }
    this.props.close()
  }
}

export default ImageButton
