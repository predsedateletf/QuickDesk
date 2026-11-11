import { createApp } from 'vue'
import { createI18n } from 'vue-i18n'
import App from './App.vue'
import router from './router'
import { zh } from './i18n/zh'
import { en } from './i18n/en'
import { ru } from './i18n/ru'

const savedLang = localStorage.getItem('quickdesk_lang')
const browserLang = navigator.language.startsWith('zh') ? 'zh-CN'
  : navigator.language.startsWith('ru') ? 'ru-RU' : 'en-US'
const locale = ['zh-CN', 'en-US', 'ru-RU'].includes(savedLang) ? savedLang : browserLang

const i18n = createI18n({
  legacy: false,
  locale,
  fallbackLocale: 'en-US',
  messages: { 'zh-CN': zh, 'en-US': en, 'ru-RU': ru },
})

const app = createApp(App)
app.use(router)
app.use(i18n)
app.mount('#app')
