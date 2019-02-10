import { USER_ROLE } from '@/constant'

export const isAdmin = user => user.role === USER_ROLE.ADMIN || user.role === 'SUPER_ADMIN'

export const isLeader = user => user.role === USER_ROLE.LEADER
