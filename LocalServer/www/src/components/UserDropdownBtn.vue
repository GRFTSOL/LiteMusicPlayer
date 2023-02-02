<template>
  <q-btn-dropdown 
    v-model="isUserMenuVisible"
    flat
    no-caps
    label="User Name"
    @mouseover="isInDropdownBtn = true, onUpdateVisibility(true)"
    @mouseout="isInDropdownBtn = false, onUpdateVisibility(false)"
  >
    <q-list
      @mouseover="isInDroplist = true, onUpdateVisibility(true)"
      @mouseout="isInDroplist = false, onUpdateVisibility(false)"
    >

      <q-item clickable v-close-popup>
        <q-avatar size="72px">
            <img src="images/account-avatar.png">
          </q-avatar>
      </q-item>

      <q-item clickable v-close-popup>
        <q-item-section>
          <q-item-label>修改密码</q-item-label>
        </q-item-section>
      </q-item>

      <q-item clickable v-close-popup>
        <q-item-section>
          <q-item-label>登出</q-item-label>
        </q-item-section>
      </q-item>
    </q-list>
  </q-btn-dropdown>
</template>

<script lang="ts">

import { defineComponent } from 'vue';

export default defineComponent({
  name: 'LeftNavigationBar',
  props: {
  },
  data () {
    return {
      isInDropdownBtn: false,
      isInDroplist: false,
      isUserMenuVisible: false,
      isExpertMode: true,
    }
  },
  methods: {
    onUpdateVisibility(immediately: boolean) {
      setTimeout(()=>(
        this.isUserMenuVisible = this.isInDropdownBtn || this.isInDroplist),
        immediately ? 0 : 200);
    }
  }
});

</script>

<style lang="sass" scoped>
.align-left
  margin-left: auto
</style>
