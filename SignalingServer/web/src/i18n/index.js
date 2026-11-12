import { createI18n } from 'vue-i18n'
import zhCN from './zh-CN.js'
import enUS from './en-US.js'
import ruRU from './ru-RU.js'

function getDefaultLocale() {
  const stored = localStorage.getItem('quickdesk_admin_locale')
  if (stored && ['zh-CN', 'en-US', 'ru-RU'].includes(stored)) return stored
  const nav = navigator.language || navigator.userLanguage || 'ru'
  // админка рассчитана на русскоязычных администраторов: по умолчанию русский
  if (nav.startsWith('zh')) return 'zh-CN'
  return 'ru-RU'
}

const i18n = createI18n({
  legacy: false,
  locale: getDefaultLocale(),
  fallbackLocale: 'en-US',
  messages: {
    'zh-CN': zhCN,
    'en-US': enUS,
    'ru-RU': ruRU
  }
})

export function setLocale(locale) {
  i18n.global.locale.value = locale
  localStorage.setItem('quickdesk_admin_locale', locale)
}

export function getLocale() {
  return i18n.global.locale.value
}

export default i18n
