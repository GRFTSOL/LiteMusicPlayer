<template>
  <q-layout view="lHh Lpr lFf">

    <q-header>
      
      <q-toolbar
        class="bg-grey-3 text-dark"
      >
        <q-btn
          flat
          dense
          round
          icon="menu"
          aria-label="Menu"
          @click="leftNavBarOpen = !leftNavBarOpen"
        />

        <q-toolbar-title>
          媒体库
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

      <LeftNavigationBar
        v-model="leftNavBarOpen"
        v-bind="navBarData"
      />

    <q-page-container>
      <router-view />
    </q-page-container>
  </q-layout>
</template>

<script lang="ts">

import { defineComponent, ref } from 'vue';
import LeftNavigationBar from 'src/components/LeftNavigationBar.vue';
import UserDropdownBtn from 'src/components/UserDropdownBtn.vue';
import { network, STATUS_CONNECTING, STATUS_DISCONNECTED, STATUS_INITED } from 'src/boot/web_socket_client';
import { assert } from 'src/boot/utils';

const navBarData = {
  isVisible: true,
  isMiniState: false,
  menus: [
    {
      title: '播放器',
      icon: 'play_circle',
      children: [
        {
          title: '远程控制',
          icon: 'settings_remote',
          link: '/player/remote/',
        },
        {
          title: '网页内',
          icon: 'live_tv',
          link: '/player/web/',
        },
      ],
    },
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
  components: { LeftNavigationBar, UserDropdownBtn },
  
  data() {
    const leftNavBarOpen = ref(false);

    return {
      leftNavBarOpen,
      navBarData: ref(navBarData),
      network: network,
    }
  },
  methods: {
  },
  computed: {
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
