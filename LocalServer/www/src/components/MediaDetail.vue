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
                <q-rating
                  v-if="field.name === 'rating'"
                  v-model="curMedia[field.name]"
                  size="2.5em"
                  color="grey"
                  color-selected="red-7"
                />
                <q-input
                  v-else
                  :modelValue="field.format ? field.format(curMedia[field.name]) : curMedia[field.name]"
                  @update:modelValue="(newValue: string | number | null) => curMedia[field.name] = newValue"
                  :label="field.label" dense color="primary"
                  :readonly="!field.editable"
                />
              </div>
            </div>
            <div class="row justify-end">
              <div class="col-4 q-gutter-md q-pb-md ">
                <q-btn :disabled="!isBasicInfoModified" color="primary" label="Save" @click="save" v-close-popup/>
                <q-btn label="Close" v-close-popup/>
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

import { defineComponent, ref, reactive, computed } from 'vue';
import { formatDurationML, formatTimePlayed, formatRecentDateTime, formatBitRate, deepCopy } from 'boot/utils'
import { saveMediaBasicInfo } from 'boot/MediaLibrary'


export const MEDIA_LIB_COLUMNS = [
  { name: 'id', field: 'id', label: 'ID', sortable: true, align: 'left', editable: false },
  { name: 'artist', field: 'artist', label: 'Artist', sortable: true, align: 'left', editable: true },
  { name: 'album', field: 'album', label: 'Album', sortable: true, align: 'left', editable: true },
  { name: 'title', field: 'title', label: 'Title', sortable: true, align: 'left', editable: true },
  { name: 'url', field: 'url', label: 'File', sortable: true, align: 'left', editable: false },
  { name: 'duration', field: 'duration', label: 'Duration', sortable: true, format: formatDurationML, align: 'left', editable: false },
  { name: 'genre', field: 'genre', label: 'Genre', sortable: true, align: 'left', editable: true },
  { name: 'year', field: 'year', label: 'Year', sortable: true, align: 'left', editable: true },
  { name: 'rating', field: 'rating', label: 'Rating', sortable: true, align: 'left', editable: true },
  { name: 'format', field: 'format', label: 'Format', sortable: true, align: 'left', editable: false },
  { name: 'timeAdded', field: 'timeAdded', label: 'Time Added', sortable: true, format: formatRecentDateTime, align: 'left', editable: false },
  { name: 'timePlayed', field: 'timePlayed', label: 'Time Played', sortable: true, format: formatTimePlayed, align: 'left', editable: false },
  { name: 'lyricsFile', field: 'lyricsFile', label: 'Lyrics File', sortable: true, align: 'left', editable: false },
  { name: 'countPlayed', field: 'countPlayed', label: 'Count Played', sortable: true, align: 'left', editable: false },
  { name: 'bitRate', field: 'bitRate', label: 'Bit Rate', sortable: true, align: 'left', format: formatBitRate, editable: false },
];

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

    function save() {
      saveMediaBasicInfo(curMedia);
    }

    const isBasicInfoModified = computed(function() {
      for (let field of MEDIA_LIB_COLUMNS) {
        if (props.media[field.name] !== curMedia[field.name]) {
          return true;
        }
      }

      return false;
    });

    return {
      curMedia,
      tab,
      FIELDS: MEDIA_LIB_COLUMNS,
      save,
      isBasicInfoModified,
    };
  },
  watch: {
    media(newMedia) {
      deepCopy(this.curMedia, newMedia);
    }
  },
});

</script>
