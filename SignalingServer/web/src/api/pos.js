// /v1/pos/* — постоянные пароли кассовых станций.
// Авторизация — та же сессия админ-кабинета (Bearer), отдельного ключа нет.
import { authJson } from './auth.js'

const BASE = '/v1/pos/devices'

export function listPosDevices() {
  return authJson(BASE)
}

export function createPosDevice(data) {
  return authJson(BASE, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(data)
  })
}

export function updatePosDevice(deviceId, data) {
  return authJson(`${BASE}/${encodeURIComponent(deviceId)}`, {
    method: 'PATCH',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(data)
  })
}

export function deletePosDevice(deviceId) {
  return authJson(`${BASE}/${encodeURIComponent(deviceId)}`, { method: 'DELETE' })
}
