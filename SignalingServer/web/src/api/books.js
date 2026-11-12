// /v1/admin/books* — адресные книги и доступы (админ-кабинет).
import { authJson } from './auth.js'

const BASE = '/v1/admin/books'

export function listBooks() {
  return authJson(BASE)
}

export async function listBookContacts(bookId) {
  const data = await authJson(`${BASE}/${bookId}/contacts`)
  return data?.items || []
}

export async function listBookGroups(bookId) {
  const data = await authJson(`${BASE}/${bookId}/groups`)
  return data?.items || []
}

export function createGroup(bookId, data) {
  return authJson(`${BASE}/${bookId}/groups`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(data)
  })
}

export function updateGroup(groupId, data) {
  return authJson(`/v1/admin/groups/${groupId}`, {
    method: 'PATCH',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(data)
  })
}

export function deleteGroup(groupId) {
  return authJson(`/v1/admin/groups/${groupId}`, { method: 'DELETE' })
}

export function updateBookContact(bookId, contactId, data) {
  return authJson(`${BASE}/${bookId}/contacts/${contactId}`, {
    method: 'PATCH',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(data)
  })
}

export async function searchContacts(query) {
  const data = await authJson('/v1/admin/search?q=' + encodeURIComponent(query))
  return data?.items || []
}

export function createBook(name) {
  return authJson(BASE, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ name })
  })
}

export function deleteBook(bookId) {
  return authJson(`${BASE}/${bookId}`, { method: 'DELETE' })
}

export function addBookContact(bookId, data) {
  return authJson(`${BASE}/${bookId}/contacts`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(data)
  })
}

export function deleteBookContact(bookId, contactId) {
  return authJson(`${BASE}/${bookId}/contacts/${contactId}`, { method: 'DELETE' })
}

export function grantBookAccess(bookId, user) {
  return authJson(`${BASE}/${bookId}/access`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ user_id: String(user.id), username: user.username })
  })
}

export function revokeBookAccess(bookId, userId) {
  return authJson(`${BASE}/${bookId}/access/${encodeURIComponent(userId)}`, { method: 'DELETE' })
}
