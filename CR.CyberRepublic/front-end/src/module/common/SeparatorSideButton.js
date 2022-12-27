import React from 'react'
import { AtomicBlockUtils, Entity } from 'draft-js'
import { Icon } from 'antd'

class SeparatorSideButton extends React.Component {
  constructor(props) {
    super(props)
    this.onClick = this.onClick.bind(this)
  }
  onClick() {
    const entityKey = Entity.create('separator', 'IMMUTABLE', {})
    this.props.setEditorState(
      AtomicBlockUtils.insertAtomicBlock(
        this.props.getEditorState(),
        entityKey,
        '-'
      )
    )
    this.props.close()
  }
  render() {
    return (
      <button
        className="md-sb-button md-sb-img-button"
        type="button"
        title="Add a separator"
        onClick={this.onClick}
      >
        <Icon type="minus" />
      </button>
    )
  }
}

export default SeparatorSideButton
