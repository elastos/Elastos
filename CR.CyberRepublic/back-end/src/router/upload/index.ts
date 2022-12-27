import Base from '../Base'

import file from './file'


export default Base.setRouter([
    {
        path : '/file',
        router : file,
        method : 'post'
    }
])