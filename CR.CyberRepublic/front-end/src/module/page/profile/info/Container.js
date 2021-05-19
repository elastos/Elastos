import { createContainer } from '@/util'
import Component from './Component'

export default createContainer(Component, (state) => {
  return {
    user: state.user
  }
})
