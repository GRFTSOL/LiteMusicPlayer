<template>
  <q-dialog
    v-bind:value="value"
    v-on:input="$emit('input', $event)"
    :position="alignLeft? 'left' : 'right'"
    maximized
    full-height
  >
    <q-card style="width: 850px">
      <q-toolbar
        class="bg-grey-3 text-dark"
      >
        <q-toolbar-title>
          Media Information
        </q-toolbar-title>
      </q-toolbar>

      <div class="q-pa-md">

        <q-tabs
          v-model="tab"
          dense
          no-border
          active-color="player-primary"
          indicator-color="player-primary"
          align="left"
          no-caps
        >
          <q-tab name="info" label="基本信息" />
          <q-tab name="artwork" label="Artwork" />
          <q-tab name="lyrics" label="Lyrics" />
        </q-tabs>

        <q-separator />

        <q-tab-panels v-model="tab" animated>

          <q-tab-panel name="info" class="q-pa-md">
            <div class="row q-gutter-md q-pb-md">
              <div class="col">
                <q-input
                  dense
                  color="primary"
                  v-model:value="media.artist"
                  label="Artist"
                />
              </div>
            </div>

            <div class="row q-gutter-md q-pb-md">
              <div class="col">
                <q-input
                  dense
                  color="primary"
                  v-model="media.title"
                  label="Title"
                />
              </div>
            </div>
          </q-tab-panel>

          <q-tab-panel name="artwork">
          </q-tab-panel>

          <q-tab-panel name="lyrics">
          </q-tab-panel>
        </q-tab-panels>

      </div>

    </q-card>
  </q-dialog>
</template>

<script lang="ts">

import { defineComponent } from 'vue';

export default defineComponent({
  name: 'MediaDetail',
  props: {
    value: {
      type: Boolean,
      default: true,
    },
    mediaProp: {
      type: Object,
      default: () => ({}),
    },
    alignLeft: {
      type: Boolean,
      default: false,
    }
  },
  beforeUpdate() {
    this.media = this.mediaProp;
  },
  data () {
    return {
      media: {},
      tab: 'info',
    }
  },

  methods: {
    
  }
});

</script>
