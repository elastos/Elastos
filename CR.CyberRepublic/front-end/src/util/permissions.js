import { USER_ROLE } from '@/constant'

// dev only
const SUPER_ADMIN = 'SUPER_ADMIN'

// From lowest to highest
// There is not strictly levels among them, we use levels as convenience
export const getPermissionIndex = role => [
  USER_ROLE.MEMBER,
  USER_ROLE.LEADER,
  USER_ROLE.ADMIN,
  USER_ROLE.SECRETARY,
  USER_ROLE.COUNCIL,
  SUPER_ADMIN,
].indexOf(role)

export const checkPermissions = (userRole, role) => getPermissionIndex(userRole) >= getPermissionIndex(role)

export const isCouncil = userRole => userRole === USER_ROLE.COUNCIL

export const isSecretary = userRole => userRole === USER_ROLE.SECRETARY

export const isAdmin = userRole => userRole === USER_ROLE.ADMIN || userRole === SUPER_ADMIN

export const isLeader = userRole => userRole === USER_ROLE.LEADER
