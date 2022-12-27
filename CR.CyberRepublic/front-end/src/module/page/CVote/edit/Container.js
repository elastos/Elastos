import { createContainer, api_request } from '@/util'
import Component from './Component'

export default createContainer(Component, state => ({
  user: state.user,
}), () => ({
  async getData(id) {
    const d = await api_request({
      path: `/api/cvote/get/${id}`,
    })

    return d
  },
}))
