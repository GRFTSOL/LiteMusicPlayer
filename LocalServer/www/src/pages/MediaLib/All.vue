<template>
  <q-page class="q-pa-md">
    <q-table
      flat
      title="Treats"
      :rows="mediaLib.list"
      :columns="columns"
      row-key="id"
      :visible-columns="visibleColumns"
      :filter="isFilterOn"
      :filter-method="filterMedias"
      :rows-per-page-options="[10, 15, 20, 50, 100]"
      @row-click="onClickRow"
    >
      <template v-slot:top="props">
        <q-btn-dropdown
            dense
            flat
            color="primary"
            label="过滤器"
          >
          <q-list>
            <q-item>
              <q-input dense debounce="400" color="primary" v-model="searchKeyword" label="搜索">
                <template v-slot:append>
                  <q-icon name="search" />
                </template>
              </q-input>
            </q-item>

            <q-item
              v-for="option in filterOptions"
              :key="option.label"
            >
              <q-item-section>
                <q-select outlined v-model="option.value"
                  :options="option.options"
                  @input="option.on = true"
                  @clear="option.on = false"
                  dense
                  :label="option.label">
                  <template v-slot:prepend>
                    <q-icon :name="option.icon" />
                  </template>
                </q-select>
              </q-item-section>
            </q-item>
          </q-list>
        </q-btn-dropdown>

        <div>
          <template
            v-for="option in filterOptions"
          >
            <q-chip
              v-if="option.on"
              :key="option.label"
              removable
              v-model="option.on"
              @remove="option.value=''"
              color="primary"
              text-color="white"
              :icon="option.icon">
              {{ option.value }}
            </q-chip>
          </template>

          <q-btn
            v-show="isFilterOn"
            flat round dense
            icon="playlist_remove"
            @click="removeAllFilterOptions"
          />
        </div>

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
                v-close-popup>
                <q-item-section>
                  <q-toggle v-model="visibleColumns" :val="column.name" :label="column.label" />
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
      </template>

    </q-table>

    <MediaDetail
      v-model="showMediaDetail"
      v-bind:media="curMedia"
    />

  </q-page>
</template>

<script lang="ts">


import { defineComponent } from 'vue';
import { exportFile } from 'quasar'
import { Media } from '../../components/models';
import { getMediaLibrary } from 'stores/MediaLibrary';
import MediaDetail from 'src/components/MediaDetail.vue'


function strPartition(s: string, sep: string) {
  let index = s.indexOf(sep);
  if (index == -1) {
    return [s, ''];
  } else {
    return [s.substring(0, index), s.substring(index + 1)];
  }
}

const MEDIA_LIB_COLUMNS = [{
      name: 'artist',
      required: !0,
      label: 'Artist',
      align: 'left',
      field: 'artist',
      sortable: !0
  }, {
      name: 'album',
      align: 'left',
      label: 'Album',
      field: 'album',
      sortable: !0
  }, {
      name: 'title',
      align: 'left',
      label: 'Title',
      field: 'title',
      sortable: !0
  },
];

export default defineComponent({
  name: 'PageIndex',
  components: {  },
  data () {
    return {
      searchKeyword: '',
      filterOn: '',
      filterOptions: [
        {
          icon: 'groups',
          label: 'Artist',
          field: 'artist',
          options: [''],
          value: '',
          on: false,
        },
        {
          icon: 'album',
          label: 'Album',
          field: 'album',
          options: [''],
          value: '',
          on: false,
        },
      ],
      filters: {
      },
      visibleColumns: ['artist', 'album', 'title'],
      columns: MEDIA_LIB_COLUMNS,
      mediaLib: getMediaLibrary(),
      showMediaDetail: false,
      curMedia: {},
    }
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
    filterMedias(rows: Array<Media>) {
      debugger;
      let a = [];
      let filters = [];

      for (let filter of this.filterOptions) {
        if (filter.on) {
          filters.push({ filed: filter.field, value: filter.value });
        }
      }

      for (let row of rows) {
        let match = true;
        for (let filter of filters) {
        let v = row[filter.filed as keyof Media];
          if (v !== filter.value) {
            match = false;
            break;
          }
        }

        if (match) {
          if (this.searchKeyword) {
            if (row.title.indexOf(this.searchKeyword) === -1 &&
                row.artist.indexOf(this.searchKeyword) === -1 &&
                row.album.indexOf(this.searchKeyword) === -1) {
              continue;
            }
          }

          a.push(row);
        }
      }

      return a;
    },
    removeAllFilterOptions() {
      for (let option of this.filterOptions) {
        option.on = false;
        option.value = '';
      }
    },
    onClickRow(_event: UIEvent, row: Media) {
      this.curMedia = row;
      this.showMediaDetail = true;
    }
  },
  computed: {
    isFilterOn() {
      for (let option of this.filterOptions) {
        if (option.on) {
          return 'true';
        }
      }
      return '';
    },
  },
});

</script>
