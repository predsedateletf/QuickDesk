<template>
  <div class="books-page">
    <div class="page-header">
      <h2>{{ t('books.title') }}</h2>
      <div class="header-actions">
        <el-input v-model="searchQuery" :placeholder="t('books.searchPlaceholder')" clearable
                  style="width: 280px" :prefix-icon="Search" @input="onSearchInput" />
        <el-button :icon="Refresh" size="small" :loading="loading" @click="load">
          {{ t('common.refresh') }}
        </el-button>
        <el-button type="primary" :icon="Plus" size="small" @click="openBook">
          {{ t('books.addBook') }}
        </el-button>
      </div>
    </div>

    <el-alert type="info" :closable="false" show-icon class="hint">
      {{ t('books.hint') }}
    </el-alert>

    <el-card v-if="searchQuery.trim().length >= 2" shadow="never" style="margin-top: 16px">
      <template #header>
        <span>{{ t('books.searchResults', { n: searchResults.length }) }}</span>
      </template>
      <el-table :data="searchResults" size="small" v-loading="searching" @row-click="openSearchResult">
        <el-table-column prop="book_name" :label="t('books.book')" min-width="150" />
        <el-table-column prop="group_path" :label="t('books.group')" min-width="150">
          <template #default="{ row }">{{ row.group_path || '—' }}</template>
        </el-table-column>
        <el-table-column prop="name" :label="t('books.name')" min-width="160" />
        <el-table-column prop="device_id" :label="t('books.deviceId')" width="140">
          <template #default="{ row }"><span class="mono">{{ row.device_id }}</span></template>
        </el-table-column>
        <el-table-column prop="note" :label="t('books.note')" min-width="140">
          <template #default="{ row }">{{ row.note || '—' }}</template>
        </el-table-column>
        <el-table-column :label="t('books.hasPassword')" width="110">
          <template #default="{ row }">
            <el-tag :type="row.has_password ? 'success' : 'info'" size="small">
              {{ row.has_password ? t('books.passwordSet') : t('books.noPassword') }}
            </el-tag>
          </template>
        </el-table-column>
      </el-table>
      <el-empty v-if="!searching && searchResults.length === 0" :description="t('books.nothingFound')" />
    </el-card>

    <el-card v-show="searchQuery.trim().length < 2" shadow="never" style="margin-top: 16px">
      <el-table :data="books" v-loading="loading" size="small" highlight-current-row
                @current-change="selectBook">
        <el-table-column prop="name" :label="t('books.book')" min-width="200" />
        <el-table-column prop="contacts" :label="t('books.contacts')" width="120" />
        <el-table-column :label="t('books.access')" min-width="220">
          <template #default="{ row }">
            <el-tag v-for="a in row.access" :key="a.user_id" size="small" class="tag">
              {{ a.username || a.user_id }}
            </el-tag>
            <span v-if="!row.access.length" class="muted">{{ t('books.noAccess') }}</span>
          </template>
        </el-table-column>
        <el-table-column :label="t('common.operation')" width="180" fixed="right">
          <template #default="{ row }">
            <el-button link type="primary" size="small" @click.stop="selectBook(row)">
              {{ t('books.open') }}
            </el-button>
            <el-button link type="danger" size="small" @click.stop="removeBook(row)">
              {{ t('common.delete') }}
            </el-button>
          </template>
        </el-table-column>
      </el-table>
      <el-empty v-if="!loading && books.length === 0" :description="t('books.noBooks')" />
    </el-card>

    <template v-if="current">
      <el-card shadow="never" style="margin-top: 16px">
        <template #header>
          <div class="card-header">
            <span>{{ current.name }} — {{ t('books.structure') }}</span>
            <div class="header-actions">
              <el-button size="small" :icon="Plus" @click="openGroup(false)">
                {{ t('books.addGroup') }}
              </el-button>
              <el-button size="small" :icon="Plus" :disabled="!selectedNode || selectedNode.type !== 'group'"
                         @click="openGroup(true)">
                {{ t('books.addSubgroup') }}
              </el-button>
              <el-button size="small" type="primary" :icon="Plus" @click="openContact">
                {{ t('books.addContact') }}
              </el-button>
            </div>
          </div>
        </template>

        <el-tree ref="treeRef" :data="treeData" node-key="key" :props="{ label: 'label', children: 'children' }"
                 default-expand-all highlight-current :expand-on-click-node="false"
                 @node-click="onNodeClick">
          <template #default="{ data }">
            <div class="tree-row">
              <span :class="data.type === 'group' ? 'group-label' : 'entry-label'">
                {{ data.type === 'group' ? '📁' : '🖥' }} {{ data.label }}
              </span>
              <span v-if="data.type === 'contact'" class="muted mono">{{ data.device_id }}</span>
              <span v-if="data.type === 'group'" class="muted">{{ t('books.entriesCount', { n: countEntries(data) }) }}</span>
            </div>
          </template>
        </el-tree>

        <el-empty v-if="treeData.length === 0" :description="t('books.noContacts')" />
      </el-card>

      <el-card shadow="never" style="margin-top: 16px">
        <template #header>
          <div class="card-header">
            <span>{{ t('books.access') }}</span>
            <el-button size="small" type="primary" :icon="User" @click="openGrant">
              {{ t('books.grant') }}
            </el-button>
          </div>
        </template>
        <div class="access-list">
          <el-tag v-for="a in current.access" :key="a.user_id" closable size="default" class="tag"
                  @close="revoke(a)">
            {{ a.username || a.user_id }}
          </el-tag>
          <span v-if="!current.access.length" class="muted">{{ t('books.noAccess') }}</span>
        </div>
      </el-card>
    </template>

    <el-dialog v-model="bookDialog" :title="t('books.addBook')" width="420px">
      <el-input v-model="bookName" :placeholder="t('books.bookPlaceholder')" />
      <template #footer>
        <el-button @click="bookDialog = false">{{ t('common.cancel') }}</el-button>
        <el-button type="primary" :loading="saving" @click="submitBook">{{ t('common.create') }}</el-button>
      </template>
    </el-dialog>

    <el-dialog v-model="groupDialog" :title="groupParent ? t('books.addSubgroup') : t('books.addGroup')" width="420px">
      <el-input v-model="groupName" :placeholder="t('books.groupPlaceholder')"
                @keyup.enter="saveGroup" />
      <p class="muted">{{ t('books.groupHint') }}</p>
      <template #footer>
        <el-button @click="groupDialog = false">{{ t('common.cancel') }}</el-button>
        <el-button type="primary" :loading="saving" @click="saveGroup">{{ t('common.create') }}</el-button>
      </template>
    </el-dialog>

    <el-dialog v-model="contactDialog" :title="t('books.addContact')" width="520px">
      <el-form label-width="150px">
        <el-form-item :label="t('books.fromRegister')">
          <el-select v-model="pickedRegister" clearable :placeholder="t('books.pickRegister')"
                     style="width: 100%" @change="applyRegister">
            <el-option v-for="r in registers" :key="r.device_id"
                       :label="`${r.name || t('books.register')} — ${r.device_id}`"
                       :value="r.device_id" />
          </el-select>
        </el-form-item>
        <el-form-item :label="t('books.name')">
          <el-input v-model="contactForm.name" :placeholder="t('books.namePlaceholder')" />
        </el-form-item>
        <el-form-item :label="t('books.deviceId')">
          <el-input v-model="contactForm.device_id" class="mono" />
        </el-form-item>
        <el-form-item :label="t('books.password')">
          <el-input v-model="contactForm.access_code" class="mono" :placeholder="t('books.passwordPlaceholder')" />
        </el-form-item>
        <el-form-item :label="t('books.note')">
          <el-input v-model="contactForm.note" />
        </el-form-item>
      </el-form>
      <p class="muted">{{ t('books.passwordHint') }}</p>
      <template #footer>
        <el-button @click="contactDialog = false">{{ t('common.cancel') }}</el-button>
        <el-button type="primary" :loading="saving" @click="createContact">{{ t('common.create') }}</el-button>
      </template>
    </el-dialog>

    <el-dialog v-model="grantDialog" :title="t('books.grant')" width="520px">
      <el-select v-model="grantUser" filterable :placeholder="t('books.pickUser')" style="width: 100%">
        <el-option v-for="u in users" :key="u.id" :label="`${u.username} — ${u.id}`" :value="u.id" />
      </el-select>
      <template #footer>
        <el-button @click="grantDialog = false">{{ t('common.cancel') }}</el-button>
        <el-button type="primary" :loading="saving" :disabled="!grantUser" @click="grant">
          {{ t('books.grant') }}
        </el-button>
      </template>
    </el-dialog>

    <el-drawer v-model="cardOpen" :title="cardTitle" size="420px" destroy-on-close>
      <el-form v-if="card.type === 'group'" label-position="top">
        <el-form-item :label="t('books.name')">
          <el-input v-model="card.name" @keyup.enter="saveCard" />
        </el-form-item>
        <el-form-item :label="t('books.parentGroup')">
          <el-select v-model="card.parent_id" clearable style="width: 100%"
                     :placeholder="t('books.groupNone')">
            <el-option v-for="g in flatGroups" :key="g.id" :label="g.path" :value="g.id"
                       :disabled="g.id === card.id" />
          </el-select>
        </el-form-item>
      </el-form>

      <el-form v-else-if="card.type === 'contact'" label-position="top">
        <el-form-item :label="t('books.name')">
          <el-input v-model="card.name" />
        </el-form-item>
        <el-form-item :label="t('books.deviceId')">
          <el-input v-model="card.device_id" class="mono" />
        </el-form-item>
        <el-form-item :label="t('books.password')">
          <el-input v-model="card.access_code" class="mono" type="password" show-password
                    :placeholder="card.has_password ? t('books.passwordKeep') : t('books.passwordPlaceholder')" />
        </el-form-item>
        <el-form-item :label="t('books.group')">
          <el-select v-model="card.group_id" clearable style="width: 100%"
                     :placeholder="t('books.groupNone')">
            <el-option v-for="g in flatGroups" :key="g.id" :label="g.path" :value="g.id" />
          </el-select>
        </el-form-item>
        <el-form-item :label="t('books.note')">
          <el-input v-model="card.note" />
        </el-form-item>
      </el-form>

      <template #footer>
        <div class="drawer-footer">
          <el-button type="danger" plain @click="deleteCard">{{ t('common.delete') }}</el-button>
          <span class="spacer" />
          <el-button @click="cardOpen = false">{{ t('common.cancel') }}</el-button>
          <el-button type="primary" :loading="saving" @click="saveCard">{{ t('common.save') }}</el-button>
        </div>
      </template>
    </el-drawer>
  </div>
</template>

<script setup>
import { ref, reactive, computed, onMounted } from 'vue'
import { useI18n } from 'vue-i18n'
import { ElMessage, ElMessageBox } from 'element-plus'
import { Refresh, Plus, User, Search } from '@element-plus/icons-vue'
import {
  listBooks, listBookContacts, listBookGroups, searchContacts, createBook, deleteBook,
  addBookContact, updateBookContact, deleteBookContact, createGroup,
  updateGroup, deleteGroup, grantBookAccess, revokeBookAccess
} from '../api/books.js'
import { getUsers } from '../api/users.js'
import { listPosDevices } from '../api/pos.js'

const { t } = useI18n()
const books = ref([])
const current = ref(null)
const loading = ref(false)
const saving = ref(false)
const bookDialog = ref(false)
const contactDialog = ref(false)
const grantDialog = ref(false)
const bookName = ref('')
const registers = ref([])
const users = ref([])
const pickedRegister = ref('')
const grantUser = ref(null)
const contactForm = reactive({ name: '', device_id: '', access_code: '', note: '' })
const treeRef = ref(null)
const treeData = ref([])
const selectedNode = ref(null)
const groupDialog = ref(false)
const groupName = ref('')
const groupParent = ref(null)
const cardOpen = ref(false)
const card = reactive({ type: '', id: 0, name: '', device_id: '', access_code: '', note: '',
                        group_id: null, parent_id: null, has_password: false })
const flatGroups = ref([])

const searchQuery = ref('')
const searchResults = ref([])
const searching = ref(false)
let searchTimer = null

const cardTitle = computed(() => card.type === 'group' ? t('books.groupCard') : t('books.entryCard'))

async function load() {
  loading.value = true
  try {
    const data = await listBooks()
    books.value = data?.items || []
    if (current.value) {
      current.value = books.value.find(b => b.id === current.value.id) || null
    }
    if (current.value) await loadContacts()
  } catch (error) {
    ElMessage.error(error.message || t('books.loadFailed'))
  } finally {
    loading.value = false
  }
}

async function loadContacts() {
  if (!current.value) return
  try {
    const [groups, contacts] = await Promise.all([
      listBookGroups(current.value.id),
      listBookContacts(current.value.id)
    ])
    current.value.contacts_list = contacts
    treeData.value = buildTree(groups, contacts)
    rebuildFlatGroups(treeData.value, current.value.name)
  } catch (error) {
    ElMessage.error(error.message || t('books.loadFailed'))
  }
}

// Группы приходят деревом; записи развешиваем по их группам,
// записи без группы — в корень.
function buildTree(groups, contacts) {
  const counters = { contacts: 0, groups: 0 }
  const walk = nodes => nodes.map(node => {
    counters.groups += 1
    return {
      key: 'g' + node.id,
      id: node.id,
      type: 'group',
      label: node.name,
      parent_id: node.parent_id || null,
      children: [
        ...walk(node.children || []),
        ...contacts
          .filter(c => c.group_id === node.id)
          .map(c => contactNode(c))
      ]
    }
  })
  const contactNode = c => {
    counters.contacts += 1
    return {
      key: 'c' + c.id,
      id: c.id,
      type: 'contact',
      label: c.name,
      device_id: c.device_id,
      note: c.note,
      group_id: c.group_id || null,
      has_password: !!c.access_code
    }
  }
  const tree = walk(groups)
  contacts.filter(c => !c.group_id).forEach(c => tree.push(contactNode(c)))
  return tree
}

function onSearchInput() {
  if (searchTimer) clearTimeout(searchTimer)
  const query = searchQuery.value.trim()
  if (query.length < 2) {
    searchResults.value = []
    return
  }
  searchTimer = setTimeout(async () => {
    searching.value = true
    try {
      searchResults.value = await searchContacts(query)
    } catch (error) {
      ElMessage.error(error.message || t('books.loadFailed'))
      searchResults.value = []
    } finally {
      searching.value = false
    }
  }, 300)
}

// из результатов поиска открываем книгу и карточку найденной записи
async function openSearchResult(row) {
  const book = books.value.find(b => b.id === row.book_id)
  if (book) {
    current.value = book
    await loadContacts()
  }
  openCard({
    type: 'contact',
    id: row.id,
    label: row.name,
    device_id: row.device_id,
    note: row.note,
    group_id: row.group_id || null,
    has_password: row.has_password
  })
}

function onNodeClick(data) {
  selectedNode.value = data
  openCard(data)
}

function openCard(node) {
  Object.assign(card, {
    type: node.type,
    id: node.id,
    name: node.label,
    device_id: node.device_id || '',
    access_code: '',
    note: node.note || '',
    group_id: node.type === 'contact' ? (node.group_id || null) : null,
    parent_id: node.type === 'group' ? (node.parent_id || null) : null,
    has_password: !!node.has_password
  })
  cardOpen.value = true
}

// плоский список групп с путями — для выбора родителя и группы записи
function rebuildFlatGroups(nodes, prefix) {
  const result = []
  const walk = (list, path) => list.forEach(node => {
    if (node.type !== 'group') return
    const full = path ? path + ' / ' + node.label : node.label
    result.push({ id: node.id, path: full })
    walk(node.children || [], full)
  })
  walk(nodes, prefix || '')
  flatGroups.value = result
}

function countEntries(node) {
  let total = 0
  const walk = list => list.forEach(item => {
    if (item.type === 'contact') total += 1
    else walk(item.children || [])
  })
  walk(node.children || [])
  return total
}

async function saveCard() {
  if (!card.name.trim()) {
    ElMessage.warning(t('books.nameRequired'))
    return
  }
  saving.value = true
  try {
    if (card.type === 'group') {
      await updateGroup(card.id, { name: card.name.trim(), parent_id: card.parent_id })
    } else {
      const payload = {
        name: card.name.trim(),
        device_id: card.device_id.trim(),
        note: card.note,
        group_id: card.group_id
      }
      if (card.access_code.trim()) payload.access_code = card.access_code.trim()
      await updateBookContact(current.value.id, card.id, payload)
    }
    cardOpen.value = false
    ElMessage.success(t('books.saved'))
    await loadContacts()
  } catch (error) {
    ElMessage.error(error.message || t('books.saveFailed'))
  } finally {
    saving.value = false
  }
}

async function deleteCard() {
  const title = card.type === 'group'
    ? t('books.deleteGroupConfirm', { name: card.name })
    : t('books.deleteContactConfirm', { name: card.name })
  try {
    await ElMessageBox.confirm(title, t('common.tip'), { type: 'warning' })
  } catch { return }
  try {
    if (card.type === 'group') await deleteGroup(card.id)
    else await deleteBookContact(current.value.id, card.id)
    cardOpen.value = false
    ElMessage.success(t('books.deleted'))
    await loadContacts()
  } catch (error) {
    ElMessage.error(error.message || t('books.saveFailed'))
  }
}

function selectBook(row) {
  current.value = row || null
  if (row) loadContacts()
}

function openBook() {
  bookName.value = ''
  bookDialog.value = true
}

async function submitBook() {
  if (!bookName.value.trim()) return
  saving.value = true
  try {
    await createBook(bookName.value.trim())
    bookDialog.value = false
    ElMessage.success(t('books.created'))
    await load()
  } catch (error) {
    ElMessage.error(error.message || t('books.saveFailed'))
  } finally {
    saving.value = false
  }
}

async function removeBook(row) {
  try {
    await ElMessageBox.confirm(t('books.deleteBookConfirm', { name: row.name }), t('common.tip'), { type: 'warning' })
  } catch { return }
  try {
    await deleteBook(row.id)
    if (current.value?.id === row.id) current.value = null
    ElMessage.success(t('books.deleted'))
    await load()
  } catch (error) {
    ElMessage.error(error.message || t('books.saveFailed'))
  }
}

async function openContact() {
  contactForm.name = ''
  contactForm.device_id = ''
  contactForm.access_code = ''
  contactForm.note = ''
  pickedRegister.value = ''
  contactDialog.value = true
  try {
    const data = await listPosDevices()
    registers.value = data?.items || []
  } catch {
    registers.value = []
  }
}

function applyRegister(deviceId) {
  const found = registers.value.find(r => r.device_id === deviceId)
  if (!found) return
  contactForm.device_id = found.device_id
  contactForm.name = found.name || t('books.register')
  contactForm.access_code = found.access_code || ''
}

async function createContact() {
  if (!contactForm.device_id.trim()) {
    ElMessage.warning(t('books.deviceIdRequired'))
    return
  }
  saving.value = true
  try {
    const payload = { ...contactForm }
    if (selectedNode.value && selectedNode.value.type === 'group') {
      payload.group_id = selectedNode.value.id
    }
    await addBookContact(current.value.id, payload)
    contactDialog.value = false
    ElMessage.success(t('books.contactAdded'))
    await load()
  } catch (error) {
    ElMessage.error(error.message || t('books.saveFailed'))
  } finally {
    saving.value = false
  }
}

function openGroup(asChild) {
  groupName.value = ''
  groupParent.value = asChild && selectedNode.value && selectedNode.value.type === 'group'
    ? selectedNode.value.id
    : null
  groupDialog.value = true
}

async function saveGroup() {
  if (!groupName.value.trim()) return
  saving.value = true
  try {
    await createGroup(current.value.id, { name: groupName.value.trim(), parent_id: groupParent.value })
    groupDialog.value = false
    ElMessage.success(t('books.groupCreated'))
    await loadContacts()
  } catch (error) {
    ElMessage.error(error.message || t('books.saveFailed'))
  } finally {
    saving.value = false
  }
}

async function renameGroup(node) {
  try {
    const { value } = await ElMessageBox.prompt(t('books.renamePrompt'), t('books.renameGroup'), {
      inputValue: node.label,
      inputValidator: v => (v && v.trim().length > 0) || t('books.nameRequired')
    })
    await updateGroup(node.id, { name: value.trim() })
    ElMessage.success(t('books.saved'))
    await loadContacts()
  } catch (error) {
    if (error !== 'cancel' && error !== 'close') ElMessage.error(error?.message || t('books.saveFailed'))
  }
}

async function removeGroup(node) {
  try {
    await ElMessageBox.confirm(t('books.deleteGroupConfirm', { name: node.label }), t('common.tip'), { type: 'warning' })
  } catch { return }
  try {
    await deleteGroup(node.id)
    ElMessage.success(t('books.deleted'))
    await loadContacts()
  } catch (error) {
    ElMessage.error(error.message || t('books.saveFailed'))
  }
}

async function removeContact(node) {
  try {
    await ElMessageBox.confirm(t('books.deleteContactConfirm', { name: node.label }), t('common.tip'), { type: 'warning' })
  } catch { return }
  try {
    await deleteBookContact(current.value.id, node.id)
    ElMessage.success(t('books.deleted'))
    await load()
  } catch (error) {
    ElMessage.error(error.message || t('books.saveFailed'))
  }
}

async function openGrant() {
  grantUser.value = null
  grantDialog.value = true
  if (!users.value.length) {
    try {
      const data = await getUsers({ limit: 100 })
      users.value = data?.items || data?.users || []
    } catch {
      users.value = []
    }
  }
}

async function grant() {
  const user = users.value.find(u => u.id === grantUser.value)
  if (!user) return
  saving.value = true
  try {
    await grantBookAccess(current.value.id, user)
    grantDialog.value = false
    ElMessage.success(t('books.granted'))
    await load()
  } catch (error) {
    ElMessage.error(error.message || t('books.saveFailed'))
  } finally {
    saving.value = false
  }
}

async function revoke(entry) {
  try {
    await revokeBookAccess(current.value.id, entry.user_id)
    ElMessage.success(t('books.revoked'))
    await load()
  } catch (error) {
    ElMessage.error(error.message || t('books.saveFailed'))
  }
}

onMounted(load)
</script>

<style scoped>
.page-header { display: flex; align-items: center; justify-content: space-between; }
.header-actions { display: flex; gap: 8px; }
.card-header { display: flex; align-items: center; justify-content: space-between; }
.hint { margin-top: 12px; }
.mono { font-family: ui-monospace, SFMono-Regular, Menlo, monospace; }
.muted { color: #909399; }
.tag { margin: 0 6px 6px 0; }
.tree-row { display: flex; align-items: center; gap: 10px; width: 100%; cursor: pointer; }
.group-label { font-weight: 600; }
.entry-label { color: inherit; }
.drawer-footer { display: flex; align-items: center; gap: 8px; }
.drawer-footer .spacer { flex: 1; }
.access-list { display: flex; flex-wrap: wrap; }
</style>
