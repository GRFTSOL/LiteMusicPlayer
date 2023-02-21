<template>
  <q-page class="q-pa-md">
    <div id="chart_artist"></div>
    <div class="row inline">
      <v-chart class="chart" :option="option_artist" autoresize
        @selectchanged="filterArtist" />
      <v-chart class="chart" :option="option_album" autoresize
        @selectchanged="filterAlbum" />
      <v-chart class="chart" :option="option_genre" autoresize
        @selectchanged="filterGenre" />
      <v-chart class="chart" :option="option_year" autoresize
        @dataZoom="filterYearNotSet" />
      <v-chart class="chart" :option="option_rating" autoresize
        @selectchanged="filterRating" />
      <v-chart class="chart" :option="option_format" autoresize
        @selectchanged="filterFormat" />
      <v-chart class="chart" :option="option_timeAdded" autoresize
        @dataZoom="filterTimeAdded" />
      <v-chart class="chart" :option="option_timePlayed" autoresize
        @dataZoom="filterTimePlayedNotSet" />
      <v-chart class="chart" :option="option_lyricsType" autoresize
        @selectchanged="filterLyricsType" />
      <v-chart class="chart" :option="option_countPlayed" autoresize
        @selectchanged="filterCountPlayed" />
      <v-chart class="chart" :option="option_bitRate" autoresize
        @selectchanged="filterBitRate" />
      <v-chart class="chart" :option="option_duration" autoresize
        @selectchanged="filterDuration" />
    </div>

    <q-table
      flat
      title="Treats"
      :rows="mediaList"
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
import { Media } from '../../components/models';
import { getMediaLibrary } from 'boot/MediaLibrary';
import { makePieChartOption, makeBarChartOption, makeTimelineChartOption, makeToolboxOption,
  fillChartOptionTopNItems, fillTimeChartOption } from 'boot/chart';
import MediaDetail from 'src/components/MediaDetail.vue'
import * as crossfilter from 'crossfilter2'
import { use } from 'echarts/core';
import { CanvasRenderer } from 'echarts/renderers';
import { BarChart, PieChart } from 'echarts/charts';
import { TitleComponent, TooltipComponent, LegendComponent, GridComponent, ToolboxComponent, DataZoomComponent, } from 'echarts/components';
import VChart from 'vue-echarts';
import { formatDuration, formatRecentDateTime } from 'boot/utils';


use([
  CanvasRenderer,
  PieChart,
  BarChart,
  TitleComponent,
  TooltipComponent,
  LegendComponent,
  GridComponent,
  ToolboxComponent,
  DataZoomComponent,
]);

function fillRatingItems(group: crossfilter.Group<any, string | number, number>, echart_options: any) {
    const items = [];
    for (let i = 1; i <= 5; i++) {
      items.push({
        key: i, name: i, value: 0
      });
    }

    for (const item of group.all()) {
      if (item.key >= 1 && item.key <= 5) {
        (items as any)[item.key] = item;
      }
    }

    echart_options.series[0].data = items;
}

function formatDurationML(n: number) {
  if (n == 0) {
    return '';
  }
  return formatDuration(n);
}

// function formatDateTime(n: number) {
//   return new Date(n * 1000).toLocaleString();
// }

function formatTimePlayed(n: number) {
  return n == 0 ? '' : formatRecentDateTime(n);
}

const MEDIA_LIB_COLUMNS = [
  { align: 'left', name: 'id', field: 'id', label: 'ID', sortable: true },
  { align: 'left', name: 'artist', field: 'artist', label: 'Artist', sortable: true },
  { align: 'left', name: 'title', field: 'title', label: 'Title', sortable: true },
  { align: 'left', name: 'album', field: 'album', label: 'Album', sortable: true },
  { align: 'left', name: 'file', field: 'file', label: 'File', sortable: true },
  { align: 'left', name: 'duration', field: 'duration', label: 'Duration', sortable: true, format: formatDurationML },
  { align: 'left', name: 'genre', field: 'genre', label: 'Genre', sortable: true },
  { align: 'left', name: 'year', field: 'year', label: 'Year', sortable: true },
  { align: 'left', name: 'rating', field: 'rating', label: 'Rating', sortable: true },
  { align: 'left', name: 'format', field: 'format', label: 'Format', sortable: true },
  { align: 'left', name: 'timeAdded', field: 'timeAdded', label: 'TimeAdded', sortable: true, format: formatRecentDateTime },
  { align: 'left', name: 'timePlayed', field: 'timePlayed', label: 'TimePlayed', sortable: true, format: formatTimePlayed },
  { align: 'left', name: 'lyricsType', field: 'lyricsType', label: 'LyricsType', sortable: true },
  { align: 'left', name: 'countPlayed', field: 'countPlayed', label: 'CountPlayed', sortable: true },
  { align: 'left', name: 'bitRate', field: 'bitRate', label: 'BitRate', sortable: true },
];

const COUNT_PLAYED_LEVELS = [0, 1, 5, 20, 50, 100, 500, 1000, 5000, 10000];
const DURATION_LEVELS = [1, 2, 3, 4, 5, 6, 10, 20, 50, 100];

export default defineComponent({
  name: 'PageIndex',
  components: { VChart, MediaDetail },
  data() {
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
      mediaList: [],
      mediaLib: getMediaLibrary(),
      showMediaDetail: false,
      curMedia: {},

      option_artist: makePieChartOption('Artist'),
      option_album: makePieChartOption('Album'),
      option_genre: makePieChartOption('Genre'),

      option_year: makeTimelineChartOption('Year Recorded', makeToolboxOption(true, { title: 'Filter Unkown Release Years' })),
      option_rating: makeBarChartOption('Rating', [1,2,3,4,5], makeToolboxOption(false, { title: 'Filter NOT Rated musics' })),
      option_format: makePieChartOption('Format'),
      option_timeAdded: makeTimelineChartOption('Time Added', makeToolboxOption(true, null)),
      option_timePlayed: makeTimelineChartOption('Time Played', makeToolboxOption(true, { title: 'Filter NOT Played musics' })),
      option_lyricsType: makePieChartOption('Lyrics Type'),
      option_countPlayed: makePieChartOption('Count Played'),
      option_bitRate: makePieChartOption('BitRate'),
      option_duration: makePieChartOption('Duration'),
    };
  },
  mounted() {
    let thiz = this;
    (this.option_year as any)._filterNotSet = function() {
      thiz.filterYearNotSet();
    };

    (this.option_rating as any)._filterNotSet = function() {
      thiz.filterRatingNotSet();
    };

    (this.option_timePlayed as any)._filterNotSet = function() {
      thiz.filterTimePlayedNotSet();
    };

    this.setupFilters();
  },
  methods: {
    setupFilters() {
      let cf = crossfilter.default(getMediaLibrary().list);
      this.$options._cfMusicFilters = {
        cf: cf,
        artist: cf.dimension(item => item.artist),
        album: cf.dimension(item => item.album),
        genre: cf.dimension(item => item.genre),
        year: cf.dimension(item => item.year == 0 ? 0 : new Date(item.year.toString()).getTime()),
        rating: cf.dimension(item => item.rating),
        format: cf.dimension(item => item.format),
        timeAdded: cf.dimension(item => item.timeAdded * 1000),
        timePlayed: cf.dimension(item => item.timePlayed * 1000),
        lyricsType: cf.dimension(item => item.lyricsType),
        countPlayed: cf.dimension(function(item) {
          let countPlayed = item.countPlayed;
          for (let i = 1; i < COUNT_PLAYED_LEVELS.length; i++) {
            if (countPlayed < COUNT_PLAYED_LEVELS[i]) {
              return COUNT_PLAYED_LEVELS[i - 1];
            }
          }
          return COUNT_PLAYED_LEVELS[COUNT_PLAYED_LEVELS.length - 1];
        }),
        bitRate: cf.dimension(function(item) {
          if (item.bitRate <= 0) {
            return 'Unkown';
          }

          let bitRate = Math.round(item.bitRate / 1000 / 32) * 32;
          return bitRate + 'k';
        }),
        duration: cf.dimension(function(item) {
          let duration = Math.round(item.duration / 1000 / 60);
          for (let i = 1; i < DURATION_LEVELS.length; i++) {
            if (duration < DURATION_LEVELS[i]) {
              return DURATION_LEVELS[i - 1];
            }
          }
          return DURATION_LEVELS[DURATION_LEVELS.length - 1];
        }),
      };

      this.refreshCharts();
    },
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
    },
    filterArtist(param: any) { this.doFilter(this.$options._cfMusicFilters.artist, this.option_artist, param); },
    filterAlbum(param: any) { this.doFilter(this.$options._cfMusicFilters.album, this.option_album, param); },
    filterGenre(param: any) { this.doFilter(this.$options._cfMusicFilters.genre, this.option_genre, param); },
    filterRating(param: any) { this.doFilter(this.$options._cfMusicFilters.rating, this.option_rating, param); },
    filterFormat(param: any) { this.doFilter(this.$options._cfMusicFilters.format, this.option_format, param); },
    filterTimeAdded(param: any) { this.doFilterByZoom(this.$options._cfMusicFilters.timeAdded, this.option_timeAdded, param); },
    filterLyricsType(param: any) { this.doFilter(this.$options._cfMusicFilters.lyricsType, this.option_lyricsType, param); },
    filterCountPlayed(param: any) { this.doFilter(this.$options._cfMusicFilters.countPlayed, this.option_countPlayed, param); },
    filterBitRate(param: any) { this.doFilter(this.$options._cfMusicFilters.bitRate, this.option_bitRate, param); },
    filterDuration(param: any) { this.doFilter(this.$options._cfMusicFilters.duration, this.option_duration, param); },

    filterYearNotSet() { this.doFilterNotSet(this.$options._cfMusicFilters.year, 0); },
    filterRatingNotSet() { this.doFilterNotSet(this.$options._cfMusicFilters.rating, 0); },
    filterTimePlayedNotSet() { this.doFilterNotSet(this.$options._cfMusicFilters.timePlayed, 0); },

    doFilterNotSet(dimension: crossfilter.Dimension<Media, number>, notSetValue: number | string) {
      if (!(dimension as any)._filteredNotSet) {
        dimension.filter(v => v === notSetValue);
        (dimension as any)._filteredNotSet = 1;
      } else {
        (dimension as any)._filteredNotSet = 0;
        dimension.filterAll();
      }

      this.refreshCharts();
    },
    /**
     * 
     * @param dimension 统计的维度
     * @param option echarts 的 options
     * @param param echarts selectchanged 回调的参数.
     */
    doFilter(dimension: crossfilter.Dimension<Media, number>, option: any, param: any) {
      if (param.selected.length > 0) {
        // 有过滤选项
        let keys = {} as any;
        for (let idx of param.selected[0].dataIndex) {
          const item = option.series[0].data[idx] as any;
          if (item.key === null) {
            // 选择了 Others，去掉前 N 个结果就是 other
            let others = dimension.group().top(Number.MAX_VALUE).slice(option.series[0].data.length - 1);
            for (let item of others) {
              keys[(item as any).key] = 1;
            }
          } else {
            // 选择了其他的
            keys[item.key] = 1;
          }
        }

        dimension.filter(v => keys[v as any] === 1);
      } else {
        dimension.filterAll();
      }

      this.refreshCharts();
    },
    doFilterByZoom(dimension: crossfilter.Dimension<Media, number>, option: any, param: any) {
      if (param.startValue == null) {
        const data = option.series[0].data;
        const start = data[0][0], end = data[data.length - 1][0];
        param.startValue = start + (end - start) * param.start / 100;
        param.endValue = start + (end - start) * param.end / 100;
      }

      const zoom = option.dataZoom[0];
      zoom.start = param.start;
      zoom.end = param.end;

      if (param.startValue != null) {
        // 有过滤选项
        dimension.filter(v => v >= param.startValue && v <= param.endValue);
      } else {
        dimension.filterAll();
      }

      this.refreshCharts();
    },
    refreshCharts() {
      let filters = this.$options._cfMusicFilters;

      fillChartOptionTopNItems(filters.artist.group(), 10, this.option_artist, false);
      fillChartOptionTopNItems(filters.album.group(), 10, this.option_album, false);
      fillChartOptionTopNItems(filters.genre.group(), 10, this.option_genre, false);
      fillTimeChartOption(filters.year.group(), this.option_year);
      fillRatingItems(filters.rating.group(), this.option_rating);
      fillChartOptionTopNItems(filters.format.group(), 10, this.option_format, false);
      fillTimeChartOption(filters.timeAdded.group(), this.option_timeAdded);
      fillTimeChartOption(filters.timePlayed.group(), this.option_timePlayed);
      fillChartOptionTopNItems(filters.lyricsType.group(), 10, this.option_lyricsType, false);
      fillChartOptionTopNItems(filters.countPlayed.group(), 10, this.option_countPlayed, false);
      fillChartOptionTopNItems(filters.bitRate.group(), 10, this.option_bitRate, false);
      fillChartOptionTopNItems(filters.duration.group(), 10, this.option_duration, false);
      this.mediaList = filters.cf.allFiltered();
    },
  },
  watch: {
    'mediaLib.list': function() {
      this.setupFilters();
    },
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

<style scoped>
.chart {
  height: 200px;
  width: 350px;
}
</style>
