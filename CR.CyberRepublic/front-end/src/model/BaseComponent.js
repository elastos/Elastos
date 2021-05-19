import React from 'react'
import _ from 'lodash'

export default class extends React.Component {
  constructor(p) {
    super(p)

    this.state = _.extend({}, this.ord_states())

    this.$p = this.ord_props()
    this.$f = this.ord_methods()

    this.ord_init()
  }

  // could be override
  ord_init() {
  }

  ord_render() {
    return null
  }

  ord_props() {
    return {}
  }

  ord_states() {
    return {}
  }

  ord_methods() {
    return {}
  }

  ord_loading(f = false) {
    this.setState({loading: f})
  }

  render() {
    return this.ord_render(this.$p)
  }
}
