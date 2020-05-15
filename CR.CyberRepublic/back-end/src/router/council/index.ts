import Base from '../Base'

import term from './term'
import council_list from './council_list'
import council_information from './council_information'

export default Base.setRouter([
    {
        path: '/term',
        router: term,
        method: 'get'
    },
    {
        path: '/list/:id',
        router: council_list,
        method: 'get'
    },
    {
        path: '/information/:did/:id?',
        router: council_information,
        method: 'get'
    }
])
