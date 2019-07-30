import React from 'react'
import { addNewBlock, ImageSideButton, Block } from 'medium-draft'

class ImageButton extends ImageSideButton {
  onChange(e) {
    const file = e.target.files[0]
    if (file.type.indexOf('image/') === 0) {
      const src = URL.createObjectURL(file)
      this.props.setEditorState(
        addNewBlock(this.props.getEditorState(), Block.IMAGE, {
          src
        })
      )
    }
    this.props.close()
  }
}

export default ImageButton
