import {createContainer} from '@/util'
import Component from './Component'

export default createContainer(Component, (state, p) => {
  return {
    id: p.match.params.id
  }
}, () => {
  return {}
})
