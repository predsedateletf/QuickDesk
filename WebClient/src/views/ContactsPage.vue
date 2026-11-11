<template>
  <div>
    <h2 class="page-title">{{ $t('contacts.title') }}</h2>

    <div v-if="!authState.isLoggedIn" class="card">
      <div class="empty-state">
        <div class="empty-icon">🔒</div>
        <p>{{ $t('contacts.loginRequired') }}</p>
        <p class="hint">{{ $t('contacts.loginHint') }}</p>
      </div>
    </div>

    <template v-else>
      <div class="card">
        <div class="card-title">{{ $t('contacts.addTitle') }}</div>
        <div class="form-row">
          <input v-model="form.name" class="input" :placeholder="$t('contacts.name')">
          <input v-model="form.device_id" class="input mono" :placeholder="$t('contacts.deviceId')">
          <input v-model="form.access_code" class="input mono" :placeholder="$t('contacts.accessCode')">
          <input v-model="form.note" class="input" :placeholder="$t('contacts.note')">
          <button class="btn btn-primary btn-sm" :disabled="saving || !form.device_id" @click="save">
            {{ editingId ? $t('contacts.save') : $t('contacts.add') }}
          </button>
          <button v-if="editingId" class="btn btn-secondary btn-sm" @click="resetForm">{{ $t('user.cancel') }}</button>
        </div>
        <p class="hint">{{ $t('contacts.hint') }}</p>
      </div>

      <div class="card">
        <div class="card-title">{{ $t('contacts.shared') }}</div>
        <div v-if="loading" class="empty-state"><p>{{ $t('contacts.loading') }}</p></div>
        <div v-else-if="contacts.length === 0" class="empty-state">
          <div class="empty-icon">📒</div>
          <p>{{ $t('contacts.empty') }}</p>
        </div>
        <div v-for="c in contacts" :key="c.id" class="device-item">
          <div class="device-info">
            <div class="device-name">{{ c.name }}</div>
            <div class="device-id">
              <span class="mono">{{ c.device_id }}</span>
              <span v-if="c.access_code" class="mono"> · {{ $t('contacts.codeShort') }} {{ c.access_code }}</span>
              <span v-if="c.note" class="muted"> — {{ c.note }}</span>
            </div>
          </div>
          <div class="device-actions">
            <button class="btn btn-primary btn-sm" @click="connectTo(c)">{{ $t('devices.connect') }}</button>
            <button v-if="c.created_by === myId" class="icon-btn" :title="$t('contacts.edit')" @click="edit(c)">✏️</button>
            <button v-if="c.created_by === myId" class="icon-btn danger" :title="$t('contacts.remove')" @click="remove(c)">🗑</button>
          </div>
        </div>
      </div>
    </template>
  </div>
</template>

<script setup>
import { ref, reactive, computed, onMounted } from 'vue'
import { useRouter } from 'vue-router'
import { useI18n } from 'vue-i18n'
import { userApi } from '../api/userApi'
import { authState } from '../store/auth'

const router = useRouter()
const { t } = useI18n()

const contacts = ref([])
const loading = ref(false)
const saving = ref(false)
const editingId = ref(null)
const form = reactive({ name: '', device_id: '', access_code: '', note: '' })
const myId = computed(() => (authState.userInfo && authState.userInfo.id) || null)

async function load() {
  loading.value = true
  const res = await userApi.fetchTeamContacts()
  if (res.ok && res.data) contacts.value = res.data.items || []
  loading.value = false
}

function resetForm() {
  editingId.value = null
  form.name = ''
  form.device_id = ''
  form.access_code = ''
  form.note = ''
}

function edit(c) {
  editingId.value = c.id
  form.name = c.name
  form.device_id = c.device_id
  form.access_code = c.access_code
  form.note = c.note
}

async function save() {
  saving.value = true
  const payload = { ...form }
  const res = editingId.value
    ? await userApi.updateTeamContact(editingId.value, payload)
    : await userApi.createTeamContact(payload)
  saving.value = false
  if (res.ok) {
    resetForm()
    await load()
  } else {
    alert(res.error || t('toast.networkError'))
  }
}

async function remove(c) {
  if (!confirm(t('contacts.confirmRemove'))) return
  const res = await userApi.deleteTeamContact(c.id)
  if (res.ok) await load()
  else alert(res.error || t('toast.networkError'))
}

function connectTo(c) {
  router.push({ path: '/remote', query: { device: c.device_id, code: c.access_code || '' } })
}

onMounted(() => { if (authState.isLoggedIn) load() })
</script>

<style scoped>
.form-row { display: flex; gap: 8px; flex-wrap: wrap; align-items: center; }
.form-row .input { flex: 1 1 160px; min-width: 140px; }
.hint { margin-top: 10px; font-size: 12px; color: var(--text-secondary, #8a94a6); }
.mono { font-family: ui-monospace, SFMono-Regular, Menlo, monospace; }
</style>
