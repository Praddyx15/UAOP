// The six-role RBAC model (SECURITY.md §3). Role→permission matrix itself
// lives in docs/context/ (versioned there per that doc) — this module only
// carries the role identifiers so auth code has one source of truth to
// import instead of stringly-typed role checks scattered around.

export const ROLES = Object.freeze({
  PLATFORM_ADMIN: 'PLATFORM_ADMIN',
  ORG_ADMIN: 'ORG_ADMIN',
  FLEET_MANAGER: 'FLEET_MANAGER',
  PILOT: 'PILOT',
  OBSERVER: 'OBSERVER',
  MAINTENANCE: 'MAINTENANCE',
});

export function isValidRole(role) {
  return Object.values(ROLES).includes(role);
}
