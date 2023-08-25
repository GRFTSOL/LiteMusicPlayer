<template>
  <q-layout view="lHh Lpr lFf">

    <q-header>
      <q-toolbar
        class="bg-grey-3 text-dark"
      >

      <div style="width: 550px" class="row items-center no-wrap">
        <div class="q-space">
          <div class="row">
            <div class="q-pr-sm text-weight-bold">{{ playerData.cur_media?.title }}</div>
            <div class="text-grey">{{ playerData.cur_media?.artist }}</div>
          </div>
          <q-slider v-model="position" :min="0" :max="playerData.cur_media.duration"
            label color="green" :label-value="formatDuration(position)"
            thumb-size="0px"
            />
        </div>

        <q-btn flat round icon="fast_rewind" @click="prev" />
        <q-btn flat size="xl" dense round :icon="playing_status" @click="play_pause" />
        <q-btn flat size="md" round icon="fast_forward" @click="next" />

        <div>
          <div class="row">
            <q-btn size="sm" flat round
              :color="playerData.settings.shuffle ? 'dark' : 'grey-6'"
              icon="shuffle"
              @click="toggleShuffle"
            />
            <q-space />
            <q-btn size="sm" flat round style="margin"
              :color="playerData.settings.loop == 'off' ? 'grey-6' : 'dark'"
              :icon="loopModeIcon"
              @click="toggleLoop"
            />
          </div>
          <div class="row inline">
            <q-btn size="sm" flat round :icon="playerData.settings.mute ? 'volume_off' : 'volume_up'" @click="volume_mute" />
            <div style="padding-top: 4px; padding-right: 8px;">
              <q-slider
                v-model="volume" :min="0" :max="100" label color="green"
                style="width: 80px" :label-value="'Volume: ' + volume + '%'" dense
                thumb-size="5px" class="vertical-middle"
              />
            </div>
          </div>
        </div>
      </div>

        <!-- <q-btn
          flat
          dense
          round
          icon="menu"
          aria-label="Menu"
          @click="leftNavBarOpen = !leftNavBarOpen"
        /> -->
        <template
          v-for="menu in navBarData.menus"
          :key="menu.title"
        >
          <q-btn-dropdown 
            flat
            no-caps
            push
            dense
            :label="menu.title"
            :icon="menu.icon"
          >
            <q-list>
              <q-item clickable v-close-popup
                v-for="child in menu.children"
                :key="child.link"
                :to="child.link" 
              >
                <q-item-section avatar>
                  <q-avatar :icon="child.icon" flat />
                </q-item-section>
                <q-item-section>
                  <q-item-label>{{ child.title }}</q-item-label>
                </q-item-section>
              </q-item>
            </q-list>
          </q-btn-dropdown>
        </template>

        <q-toolbar-title>
        </q-toolbar-title>

        <div>
          <q-btn
            flat
            dense
            round
            :color="notification?.color"
            :icon="notification?.icon"
            :aria-label="notification?.tooltip"
          >
            <q-tooltip>{{ notification?.tooltip }}</q-tooltip>
          </q-btn>
          <UserDropdownBtn />
        </div>

        </q-toolbar>
    </q-header>

    <!-- <LeftNavigationBar
      v-model="leftNavBarOpen"
      v-bind="navBarData"
    /> -->

    <q-page-container>
      <router-view />
    </q-page-container>
  </q-layout>
</template>

<script lang="ts">

import { defineComponent, ref } from 'vue';
// import LeftNavigationBar from 'src/components/LeftNavigationBar.vue';
import UserDropdownBtn from 'src/components/UserDropdownBtn.vue';
import { network, STATUS_CONNECTING, STATUS_DISCONNECTED, STATUS_INITED } from 'src/boot/web_socket_client';
import { assert } from 'src/boot/utils';
import * as wsc from 'boot/web_socket_client';
import { playerData } from 'boot/PlayerData';
import { formatDuration } from 'boot/utils';


const navBarData = {
  isVisible: true,
  isMiniState: false,
  menus: [
    {
      title: '媒体库',
      icon: 'library_music',
      children: [
        {
          title: '所有歌曲',
          icon: 'format_align_justify',
          link: '/media_lib/all/',
        },
        {
          title: '歌手',
          icon: 'face',
          link: '/media_lib/artist/',
        },
        {
          title: '专辑',
          icon: 'album',
          link: '/media_lib/album/',
        },
        {
          title: '播放列表',
          icon: 'queue_music',
          link: '/media_lib/playlist/',
        },
        {
          title: '目录',
          icon: 'folder_open',
          link: '/media_lib/directory/',
        },
        {
          title: 'Genre',
          icon: 'category',
          link: '/media_lib/genre/',
        },
        {
          title: '语言',
          icon: 'language',
          link: '/media_lib/language/',
        },
      ],
    },
    {
      title: '歌词',
      icon: 'lyrics',
      children: [
        {
          title: '本地歌词',
          icon: 'list',
          link: '/lyrics/local/',
        },
        {
          title: '我制作的歌词',
          icon: 'badge',
          link: '/lyrics/mine/',
        },
      ],
    },
    {
      title: '特效歌词',
      icon: 'generating_tokens',
      children: [
        {
          title: '本地特效歌词',
          icon: 'list',
          link: '/sf_lyrics/local/',
        },
        {
          title: '我制作的特效歌词',
          icon: 'badge',
          link: '/sf_lyrics/mine/',
        },
      ],
    },
    {
      title: '统计报表',
      icon: 'analytics',
      children: [
        {
          title: '媒体库',
          icon: 'library_music',
          link: '/report/media_lib/',
        },
        {
          title: '播放器',
          icon: 'play_circle',
          link: '/report/player/',
        },
        {
          title: '远程连接',
          icon: 'remove_red_eye',
          link: '/report/query/',
        },
      ],
    },
    {
      title: '设置',
      icon: 'settings',
      link: '/system/settings/',
    },
  ],
}


export default defineComponent({
  name: 'MainLayout',
  components: { UserDropdownBtn },
  
  data() {
    return {
      playerData: playerData,
      navBarData: ref(navBarData),
      network: network,
    };
  },
  methods: {
    prev() {
      wsc.sendPlayerCommand('prev');
    },
    next() {
      wsc.sendPlayerCommand('next');
    },
    play_pause() {
      wsc.sendPlayerCommand('play_pause');
    },
    volume_mute() {
      wsc.sendPlayerCommand('settings.mute', !this.playerData.settings.mute);
    },
    formatDuration : formatDuration,
    toggleShuffle() {
      wsc.sendPlayerCommand('settings.shuffle', !this.playerData.settings.shuffle);
    },
    toggleLoop() {
      function nextLoopMode(value: string) {
        switch (value) {
          case 'all': return 'off';
          case 'one': return 'all';
          case 'off': return 'one';
          default: assert(0); return '';
        }
      }

      wsc.sendPlayerCommand('settings.loop', nextLoopMode(this.playerData.settings.loop));
    },
  },
  computed: {
    playing_status() {
      return this.playerData?.status == 'playing' ? 'pause_circle' : 'play_circle';
    },
    loopModeIcon() {
      let icon = '';
      switch (this.playerData.settings.loop) {
        case 'all': icon = 'repeat'; break;
        case 'one': icon = 'repeat_one'; break;
        case 'off': icon = 'repeat'; break;
        default: assert(0);
      }

      return icon;
    },
    volume: {
      get() {
        return this.playerData.settings.volume;
      },
      set(value: number) {
        this.playerData.setVolume(value);
      }
    },
    position: {
      get() {
        return this.playerData.position;
      },
      set(value: number) {
        this.playerData.seek(value);
      }
    },
    notification() {
      switch (this.network.status) {
        case STATUS_CONNECTING:
          return { icon: 'pending', color: 'warning', tooltip: this.network.description };
        case STATUS_DISCONNECTED:
          return { icon: 'error', color: 'negative', tooltip: this.network.description };
        case STATUS_INITED:
          return { icon: 'cloud_done', color: 'positive', tooltip:this.network.description };
        default:
          assert(0);
          return { icon: 'error', color: 'negative', tooltip: this.network.description };
      }
    },
  }
});

</script>
