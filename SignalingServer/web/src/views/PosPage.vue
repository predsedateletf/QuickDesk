<template>
  <div class="pos-page">
    <div class="page-header">
      <h2>{{ t('pos.title') }}</h2>
      <div class="header-actions">
        <el-button :icon="Refresh" size="small" :loading="loading" @click="load">
          {{ t('common.refresh') }}
        </el-button>
        <el-button type="primary" :icon="Plus" size="small" @click="openCreate">
          {{ t('pos.addDevice') }}
        </el-button>
      </div>
    </div>

    <el-alert type="info" :closable="false" show-icon class="hint">
      {{ t('pos.hint') }}
    </el-alert>

    <el-card shadow="never" style="margin-top: 16px">
      <el-table :data="items" v-loading="loading" size="small" stripe>
        <el-table-column prop="name" :label="t('pos.name')" min-width="160">
          <template #default="{ row }">
            <el-input v-if="row._edit" v-model="row.name" size="small" />
            <span v-else>{{ row.name || '—' }}</span>
          </template>
        </el-table-column>

        <el-table-column prop="device_id" :label="t('pos.deviceId')" min-width="140">
          <template #default="{ row }">
            <span class="mono">{{ row.device_id }}</span>
          </template>
        </el-table-column>

        <el-table-column :label="t('pos.password')" width="220">
          <template #default="{ row }">
            <el-input v-if="row._edit" v-model="row.access_code" class="mono" size="small" />
            <span v-else class="mono password">{{ row._show ? row.access_code : '••••••••' }}</span>
            <el-button v-if="!row._edit" link size="small" @click="row._show = !row._show">
              {{ row._show ? t('pos.hide') : t('pos.show') }}
            </el-button>
          </template>
        </el-table-column>

        <el-table-column :label="t('pos.enabled')" width="110">
          <template #default="{ row }">
            <el-switch v-model="row.enabled" size="small" @change="toggle(row)" />
          </template>
        </el-table-column>

        <el-table-column :label="t('pos.lastPoll')" width="200">
          <template #default="{ row }">
            <span :class="row.last_poll_at ? 'ok' : 'muted'">
              {{ row.last_poll_at || t('pos.never') }}
            </span>
          </template>
        </el-table-column>

        <el-table-column :label="t('common.operation')" width="230" fixed="right">
          <template #default="{ row }">
            <template v-if="row._edit">
              <el-button type="primary" size="small" :loading="row._saving" @click="save(row)">
                {{ t('common.save') }}
              </el-button>
              <el-button size="small" @click="row._edit = false">{{ t('common.cancel') }}</el-button>
            </template>
            <template v-else>
              <el-button link type="primary" size="small" @click="row._edit = true">
                {{ t('common.edit') }}
              </el-button>
              <el-button link type="warning" size="small" @click="rotate(row)">
                {{ t('pos.rotate') }}
              </el-button>
              <el-button link type="danger" size="small" @click="remove(row)">
                {{ t('common.delete') }}
              </el-button>
            </template>
          </template>
        </el-table-column>
      </el-table>

      <el-empty v-if="!loading && items.length === 0" :description="t('pos.empty')" />
    </el-card>

    <el-dialog v-model="dialog" :title="t('pos.addDevice')" width="460px">
      <el-form label-width="140px">
        <el-form-item :label="t('pos.deviceId')">
          <el-input v-model="form.device_id" class="mono" :placeholder="t('pos.deviceIdPlaceholder')" />
        </el-form-item>
        <el-form-item :label="t('pos.name')">
          <el-input v-model="form.name" :placeholder="t('pos.namePlaceholder')" />
        </el-form-item>
        <el-form-item :label="t('pos.password')">
          <el-input v-model="form.access_code" class="mono" :placeholder="t('pos.passwordPlaceholder')" />
        </el-form-item>
      </el-form>
      <template #footer>
        <el-button @click="dialog = false">{{ t('common.cancel') }}</el-button>
        <el-button type="primary" :loading="saving" @click="create">{{ t('common.create') }}</el-button>
      </template>
    </el-dialog>

    <el-dialog v-model="tokenDialog" :title="t('pos.tokenTitle')" width="520px">
      <p class="muted">{{ t('pos.tokenHint') }}</p>
      <el-input v-model="lastToken" readonly class="mono" />
      <template #footer>
        <el-button type="primary" @click="tokenDialog = false">{{ t('common.confirm') }}</el-button>
      </template>
    </el-dialog>
  </div>
</template>

<script setup>
import { ref, reactive, onMounted } from 'vue'
import { useI18n } from 'vue-i18n'
import { ElMessage, ElMessageBox } from 'element-plus'
import { Refresh, Plus } from '@element-plus/icons-vue'
import { listPosDevices, createPosDevice, updatePosDevice, deletePosDevice } from '../api/pos.js'

const { t } = useI18n()
const items = ref([])
const loading = ref(false)
const saving = ref(false)
const dialog = ref(false)
const tokenDialog = ref(false)
const lastToken = ref('')
const form = reactive({ device_id: '', name: '', access_code: '' })

async function load() {
  loading.value = true
  try {
    const data = await listPosDevices()
    items.value = (data?.items || []).map(item => ({ ...item, _edit: false, _show: false, _saving: false }))
  } catch (error) {
    ElMessage.error(error.message || t('pos.loadFailed'))
  } finally {
    loading.value = false
  }
}

function openCreate() {
  form.device_id = ''
  form.name = ''
  form.access_code = ''
  dialog.value = true
}

async function create() {
  if (!form.device_id.trim()) {
    ElMessage.warning(t('pos.deviceIdRequired'))
    return
  }
  saving.value = true
  try {
    const data = await createPosDevice({
      device_id: form.device_id.trim(),
      name: form.name.trim(),
      access_code: form.access_code.trim()
    })
    dialog.value = false
    lastToken.value = data?.poll_token || ''
    if (lastToken.value) tokenDialog.value = true
    ElMessage.success(t('pos.created'))
    await load()
  } catch (error) {
    ElMessage.error(error.message || t('pos.saveFailed'))
  } finally {
    saving.value = false
  }
}

async function save(row) {
  row._saving = true
  try {
    await updatePosDevice(row.device_id, { name: row.name, access_code: row.access_code })
    row._edit = false
    ElMessage.success(t('pos.saved'))
    await load()
  } catch (error) {
    ElMessage.error(error.message || t('pos.saveFailed'))
  } finally {
    row._saving = false
  }
}

async function rotate(row) {
  try {
    await ElMessageBox.confirm(t('pos.rotateConfirm', { id: row.device_id }), t('common.tip'), { type: 'warning' })
  } catch {
    return
  }
  const alphabet = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789'
  const bytes = new Uint8Array(9)
  crypto.getRandomValues(bytes)
  const code = Array.from(bytes).map(byte => alphabet[byte % alphabet.length]).join('')
  try {
    await updatePosDevice(row.device_id, { access_code: code })
    ElMessage.success(t('pos.rotated', { code }))
    await load()
  } catch (error) {
    ElMessage.error(error.message || t('pos.saveFailed'))
  }
}

async function toggle(row) {
  try {
    await updatePosDevice(row.device_id, { enabled: row.enabled })
    ElMessage.success(t('pos.saved'))
  } catch (error) {
    row.enabled = !row.enabled
    ElMessage.error(error.message || t('pos.saveFailed'))
  }
}

async function remove(row) {
  try {
    await ElMessageBox.confirm(t('pos.deleteConfirm', { id: row.device_id }), t('common.tip'), { type: 'warning' })
  } catch {
    return
  }
  try {
    await deletePosDevice(row.device_id)
    ElMessage.success(t('pos.deleted'))
    await load()
  } catch (error) {
    ElMessage.error(error.message || t('pos.saveFailed'))
  }
}

onMounted(load)
</script>

<style scoped>
.page-header { display: flex; align-items: center; justify-content: space-between; }
.header-actions { display: flex; gap: 8px; }
.hint { margin-top: 12px; }
.mono { font-family: ui-monospace, SFMono-Regular, Menlo, monospace; }
.password { letter-spacing: 1px; }
.ok { color: #67c23a; }
.muted { color: #909399; }
</style>
