<template>
  <q-page class="q-pa-md">
    <div>
      <div>
        <q-card-section class="row items-center no-wrap">
          <q-btn flat round icon="fast_rewind" @click="prev" />
          <q-btn flat size="xl" dense round :icon="playing_status" @click="play_pause" />
          <q-btn flat round icon="fast_forward" @click="next" />

          <q-btn flat round :icon="playerData.settings.mute ? 'volume_off' : 'volume_up'" @click="volume_mute" />
          <q-slider v-model="volume" :min="0" :max="100" label color="green" style="width: 100px" :label-value="volume + '%'"/>

          <q-space />

          <div>
            <div class="text-weight-bold">{{ playerData.cur_media?.artist }}</div>
            <div class="text-grey">{{ playerData.cur_media?.title }}</div>
          </div>
        </q-card-section>

        <q-slider v-model="position" :min="0" :max="playerData.cur_media.duration" label color="green" :label-value="formatDuration(position)"/>
      </div>

      <div>
        <router-view />
      </div>

    </div>
  </q-page>
</template>

<script lang="ts">

import { defineComponent } from 'vue';
import * as wsc from 'boot/web_socket_client';
import { playerData } from 'stores/PlayerData';

function paddingZero(n: number, width: number) {
  let str = String(Math.floor(n));
  while (str.length < width) {
    str = '0' + str;
  }
  return str;
}

function formatDuration(n: number) {
  n /= 1000;
  let str = ':' + paddingZero(n % 60, 2); // Seconds
  n /= 60;
  if (n >= 1) {
    str = paddingZero(n % 60, 2) + str; // Minutes
    n /= 60;
    if (n >= 1) {
      str = paddingZero(n, 0) + ':' + str; // Hours
    } else {
      if (str[0] == '0') {
        str = str.slice(1);
      }
    }
  } else {
    // Minutes: 0
    str = '0' + str;
  }
  return str;
}

export default defineComponent({
  name: 'RemoteCtrlPlayer',
  components: {  },
  data() {
    return {
      playerData: playerData,
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
  },
  computed: {
    playing_status() {
      return this.playerData?.status == 'playing' ? 'pause_circle' : 'play_circle';
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
    }
  }
});

</script>
