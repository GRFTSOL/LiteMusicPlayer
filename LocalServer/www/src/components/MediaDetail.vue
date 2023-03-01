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
          {{ alignLeft  }}
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
            <div
              class="q-gutter-md q-pb-md row"
            >
              <div
                v-for="field in FIELDS"
                :key="field.name"
                :class="field.name == 'url' ? 'col-10' : 'col-5'"
                >
                <q-input
                  :modelValue="field.format ? field.format(curMedia[field.name]) : curMedia[field.name]"
                  @@update:modelValue="(newValue: string) => curMedia[field.name] = newValue"
                  :label="field.label" dense color="primary"
                  :readonly="!field.editable"
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

import { defineComponent, watch, ref, reactive } from 'vue';
import { deepCopy } from 'boot/utils'
import { MEDIA_LIB_COLUMNS } from 'src/components/MediaTable.vue'


const FIELDS = MEDIA_LIB_COLUMNS;

export default defineComponent({
  name: 'MediaDetail',
  props: {
    value: {
      type: Boolean,
      default: true,
    },
    media: {
      type: Object,
      default: () => ({}),
    },
    alignLeft: {
      type: Boolean,
      default: false,
    }
  },
  setup(props) {
    let curMedia = reactive(deepCopy({}, props.media));
    const tab = ref('info');

    return {
      curMedia,
      tab,
      FIELDS,
    };
  },
  watch: {
    media(newMedia) {
      this.curMedia = reactive(deepCopy({}, newMedia));
    }
  },
});

</script>
