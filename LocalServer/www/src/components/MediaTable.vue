<template>
  <q-table
    flat
    title="Treats"
    :rows="mediaList"
    :columns="columns"
    row-key="id"
    :visible-columns="visibleColumns"
    :rows-per-page-options="[10, 15, 20, 50, 100]"
    @row-click="onClickRow"
    ref="el_table"
  >
    <template v-slot:top="props">
      
      <q-space />

      <div>
        <q-btn-dropdown
          dense
          flat
          color="primary"
          label="显示列"
        >
          <q-list>
            <q-item
              v-for="column in columns"
              :key="column.name"
              dense
              v-close-popup>
              <q-item-section>
                <q-toggle v-model="visibleColumns" :val="column.name" :label="column.label"  dense/>
              </q-item-section>
            </q-item>
          </q-list>
        </q-btn-dropdown>
      </div>

      <q-btn
        flat dense
        label="导出"
        @click="exportTable"
      />
      
      <q-btn
        flat round dense
        icon="settings"
      />

      <q-btn
        flat round dense
        :icon="props.inFullscreen ? 'fullscreen_exit' : 'fullscreen'"
        @click="props.toggleFullscreen"
      />

      <q-btn
        v-if="canRemoveTable"
        flat round dense
        icon="clear"
        @click="$emit('remove_table')"
      />
    </template>

  </q-table>

  <MediaDetail
    v-model="showMediaDetail"
    v-bind:media="curMedia"
    :alignLeft="mediaDetailAlignLeft"
  />

</template>

<script lang="ts">

import { defineComponent, reactive } from 'vue';
import { formatDurationML, formatTimePlayed, formatRecentDateTime, formatBitRate, loadConfig, saveConfig } from 'boot/utils';
import { Media } from 'components/models';
import MediaDetail from 'src/components/MediaDetail.vue'


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
  name: 'MediaTable',
  components: { MediaDetail },
  props: {
    value: {
      type: Boolean,
      default: true,
    },
    mediaList: {
      type: Array<Media>,
      default: () => ([]),
    },
    canRemoveTable: {
      type: Boolean,
      default: false,
    },
    saveStateName: {
      type: String,
      default: '',
    },
  },
  emits: ['remove_table'],
  setup(props) {
    const data = reactive({
      media: {},
      el_table: null,
      visibleColumns: ['artist', 'album', 'title'],
      columns: MEDIA_LIB_COLUMNS,
      showMediaDetail: false,
      curMedia: {},
      mediaDetailAlignLeft: false,

      onClickRow,
      saveVisibleColumns,
    });

    function onClickRow(_event: Event, row: Media) {
      data.curMedia = row;
      data.showMediaDetail = true;
      const el = (data.el_table as any).$el;
      data.mediaDetailAlignLeft = (el.getBoundingClientRect().x + el.clientWidth / 2) >= window.innerWidth / 2;
    }

    const CONF_VERSION = 1;
    const saveStateName = 'mediaTable_' + props.saveStateName;
    const conf = loadConfig(saveStateName, CONF_VERSION);
    if (conf) {
      data.visibleColumns = conf.visibleColumns;
    }

    function saveVisibleColumns(newVisibleColumns: any) {
      saveConfig(saveStateName, CONF_VERSION, { visibleColumns: newVisibleColumns });
    }

    // Won't work, why???
    // watch(data.visibleColumns, (newVisibleColumns) => saveVisibleColumns(newVisibleColumns), { deep: true });

    return data;
  },
  methods: {
    exportTable () {
      // let a = [];
      // a.push(this.columns.map(col => col.label).join(','));

      // for (let row of this.mediaLib.list) {
      //   let b = [] as Array<string>;
      //   for (let col of this.columns) {
      //     b.push(row[col.field as keyof Media]);
      //   }
      //   a.push(b.join(','));
      // }
      // const content = a.join('\n');

      // const status = exportFile(
      //   'table-export.csv',
      //   content,
      //   'text/csv'
      // )

      // if (status !== true) {
      //   this.$q.notify({
      //     message: 'Browser denied file download...',
      //     color: 'negative',
      //     icon: 'warning'
      //   })
      // }
    },
  },
  watch: {
      visibleColumns: {
        handler: function(newVisibleColumns) {
          this.saveVisibleColumns(newVisibleColumns);
        },
        deep: true,
      },
    }
});

</script>
