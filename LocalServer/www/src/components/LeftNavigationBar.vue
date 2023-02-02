<template>
  <q-drawer
    v-bind:value="value"
    v-on:input="$emit('input', $event)"
    show-if-above
    :mini="!value || isMiniState"
    @click.capture="drawerClick"
    :width="200"
    :breakpoint="500"
    bordered
    content-class="bg-grey-3"
  >
    <q-scroll-area class="fit">
      <q-list>
        <q-item
          class="player-primary-color"
        >
          <q-item-section avatar>
            <img
              src="/images/logo.png"
              style='height: 35px; margin-left: auto;'
            />
          </q-item-section>
          <q-item-section>
            DH Player
          </q-item-section>
        </q-item>

        <template
          v-for="menu in menus"
        >
          <q-item
            clickable
            v-ripple
            :key="menu.title"
            v-if="!menu.children"
            :active="currentRoutePath === menu.link || currentRouteParentPath === menu.link"
            :to="menu.link" exact
            active-class="player-primary-color"
          >
            <q-item-section avatar>
              <q-icon :name="menu.icon" />
            </q-item-section>

            <q-item-section>{{ menu.title }}</q-item-section>
          </q-item>

          <q-expansion-item
            expand-separator
            v-else
            :key="menu.title"
            :icon="menu.icon"
            :label="menu.title"
            :caption="menu.caption"
            active-class="player-primary-color"
          >
            <q-item
              clickable
              v-ripple
              v-for="submenu in menu.children"
              :key="submenu.title"
              :to="submenu.link" exact
              :active="currentRoutePath === submenu.link || currentRouteParentPath === submenu.link"
              active-class="player-primary-color"
              :header-inset-level="1"
              :content-inset-level="2"
              style="padding-left: 40px"
            >
              <q-item-section avatar>
                <q-icon :name="submenu.icon" />
              </q-item-section>

              <q-item-section>{{ submenu.title }}</q-item-section>
            </q-item>

          </q-expansion-item>
        </template>
      </q-list>
    </q-scroll-area>

    <!--
      in this case, we use a button (can be anything)
      so that user can switch back
      to mini-mode
    -->
    <div class="q-mini-drawer-hide absolute" style="top: 58px; right: -17px">
      <q-btn
        dense
        round
        unelevated
        color="player-primary"
        icon="chevron_left"
        @click="isMiniState = true"
      />
    </div>
  </q-drawer>
</template>

<script lang="ts">

import { defineComponent } from 'vue';

export default defineComponent({
  name: 'LeftNavigationBar',
  props: {
    value: {
      type: Boolean,
      default: true,
    },
    
    menus: {
      type: Array,
      required: true,
      default() {
        return [ {tile: 'x', link: '/'}]
      },
      props: {
        title: {
          type: String,
        },
        link: {
          type: String,
        }
      }
    },
  },
  computed: {
    currentRoutePath() {
        return this.$route.path;
    },
    currentRouteParentPath() {
      let a = this.$route.path.split('/');
      a.pop();
      a.pop();
      a.push('')
      console.log(a.join('/'));
      return a.join('/');
    }
  },
  data () {
    return {
      //isVisible: true,
      isMiniState: false
    }
  },
  methods: {
    drawerClick (e: UIEvent) {
      if (this.isMiniState) {
        this.isMiniState = false
        e.stopPropagation()
      }
    },
  }
});

</script>

<style lang="sass" scoped>
.align-left
  margin-left: auto
</style>
