import { USER_ROLE } from '../constant/constant'

// dev only
const SUPER_ADMIN = 'SUPER_ADMIN'

// From lowest to highest
// There is not strictly levels among them, we use levels as convenience
export const getPermissionIndex = (role: string) => [
  USER_ROLE.MEMBER,
  USER_ROLE.LEADER,
  USER_ROLE.ADMIN,
  USER_ROLE.SECRETARY,
  USER_ROLE.COUNCIL,
  SUPER_ADMIN,
].indexOf(role)

export const checkPermissions = (userRole: string, role: string) => getPermissionIndex(userRole) >= getPermissionIndex(role)

export const isCouncil = (userRole: string) => userRole === USER_ROLE.COUNCIL

export const isSecretary = (userRole: string) => userRole === USER_ROLE.SECRETARY

export const isAdmin = (userRole: string) => userRole === USER_ROLE.ADMIN || userRole === SUPER_ADMIN

export const isLeader = (userRole: string) => userRole === USER_ROLE.LEADER
