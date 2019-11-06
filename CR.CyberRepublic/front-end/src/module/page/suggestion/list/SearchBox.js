import React, { Component } from 'react'
import styled from 'styled-components'
import { Input, Select } from 'antd'
import I18N from '@/I18N'

const SEARCH_FILTERS = {
  TITLE: 'TITLE',
  NUMBER: 'NUMBER',
  ABSTRACT: 'ABSTRACT',
  EMAIL: 'EMAIL',
  NAME: 'NAME'
}
const DEFAULT_SEARCH_FILTER = SEARCH_FILTERS.TITLE

class SearchBox extends Component {
  constructor(props) {
    super(props)
    this.state = {
      filter: DEFAULT_SEARCH_FILTER
    }
  }

  handleSearch = (search) => {
    const { filter } = this.state
    this.props.search(filter, search)
  }

  handleChange = (filter) => {
    this.setState({ filter })
  }

  render() {
    const { value } = this.props
    const SEARCH_FILTER_TEXTS = {
      TITLE: I18N.get('suggestion.search.title'),
      NUMBER: I18N.get('suggestion.search.number'),
      ABSTRACT: I18N.get('suggestion.search.abstract'),
      EMAIL: I18N.get('suggestion.search.email'),
      NAME: I18N.get('suggestion.search.name')
    }
    return (
      <Wrapper>
        <Select
          style={{ width: 160 }}
          onChange={this.handleChange}
          defaultValue={DEFAULT_SEARCH_FILTER}
        >
          {_.map(SEARCH_FILTERS, value => (
            <Select.Option key={value} value={value}>
              {SEARCH_FILTER_TEXTS[value]}
            </Select.Option>
          ))}
        </Select>
        <Input.Search
          defaultValue={value}
          onSearch={this.handleSearch}
          placeholder={I18N.get('suggestion.form.search')}
        />
      </Wrapper>
    )
  }
}

export default SearchBox

const Wrapper = styled.div`
  display: flex;
  .ant-select-selection--single {
    background: #f5f5f5;
    border-right: none;
  }
`
