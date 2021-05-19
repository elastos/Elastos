import React from 'react'
import {Tag, Input, Tooltip, Icon} from 'antd'
import BaseComponent from '@/model/BaseComponent'

export default class extends BaseComponent {
  ord_states() {
    return {
      tags: [],
      inputVisable: false,
      inputValue: ''
    }
  }

  handleInputChange(e) {
    this.setState({inputValue: e.target.value})
  }

  handleInputConfirm() {
    const state = this.state
    const inputValue = state.inputValue
    let tags = state.tags
    if (inputValue && tags.indexOf(inputValue) === -1) {
      tags = [...tags, inputValue]
    }
    console.log(tags)
    this.setState({
      tags,
      inputVisible: false,
      inputValue: '',
    })
  }

  handleClose(removeTag) {
    const tags = this.state.tags.filter(tag => tag !== removeTag)
    console.log(tags)
    this.setState({tags})
  }

  showInput() {
    this.setState({inputVisible: true}, () => this.input.focus())
  }

  saveInputRef(input) {
    this.input = input
  }

  ord_render() {
    const {tags, inputVisible, inputValue} = this.state
    return (
      <div>
        {tags.map((tag, index) => {
          const isLongTag = tag.length > 20
          const tagElem = (
            <Tag key={tag} closable={true} afterClose={() => this.handleClose(tag)}>
              {isLongTag ? `${tag.slice(0, 20)}...` : tag}
            </Tag>
          )
          return isLongTag ? <Tooltip title={tag} key={tag}>{tagElem}</Tooltip> : tagElem
        })}
        {inputVisible && (
          <Input
            ref={this.saveInputRef.bind(this)}
            type="text"
            size="small"
            style={{width: 78}}
            value={inputValue}
            onChange={this.handleInputChange.bind(this)}
            onBlur={this.handleInputConfirm.bind(this)}
            onPressEnter={this.handleInputConfirm.bind(this)}
          />
        )}
        {!inputVisible && (
          <Tag
            onClick={this.showInput.bind(this)}
            style={{background: '#fff', borderStyle: 'dashed'}}
          >
            <Icon type="plus"/>
            {' '}
New Tag
          </Tag>
        )}
      </div>
    )
  }

  getValue() {
    return this.state.tags
  }
}
