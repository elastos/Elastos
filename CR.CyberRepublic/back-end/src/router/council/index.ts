import Base from '../Base'

import term from './term'
import council_list from './council_list'
import council_information from './council_information'
import scheduled_tasks from './scheduled_tasks'
import council_secretariat from './council_secretariat'

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
        path: '/information/:did?/:id?',
        router: council_information,
        method: 'get'
    },
    {
        path: '/scheduled_tasks',
        router: scheduled_tasks,
        method: 'get'
    },
    {
        path: '/council_secretariat',
        router: council_secretariat,
        method: 'get'
    }
])
