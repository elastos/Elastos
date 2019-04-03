import Base from '../Base'

import comment from './comment'

export default Base.setRouter([
    {
        path : '/:id/comment',
        router : comment,
        method : 'post'
    }
])
