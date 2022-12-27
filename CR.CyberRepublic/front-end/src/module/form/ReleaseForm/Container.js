import {
  createContainer,
} from '@/util'
import Component from './Component'
import ReleaseService from '@/service/ReleaseService'

const mapState = state => ({
})

const mapDispatch = () => {
  const service = new ReleaseService()
  return {
    async create(param) {
      return service.create(param)
    },
    async update(param) {
      return service.update(param)
    },
  }
}

export default createContainer(Component, mapState, mapDispatch)
