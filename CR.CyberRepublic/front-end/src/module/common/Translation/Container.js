import {
  createContainer,
} from '@/util'
import Component from './Component'
import GoogleService from '@/service/GoogleService'

const mapState = () => ({
})

const mapDispatch = () => {
  const googleService = new GoogleService()
  return {
    async gTranslate(param) {
      const res = await googleService.translate(param)
      return res
    },
  }
}

export default createContainer(Component, mapState, mapDispatch)
