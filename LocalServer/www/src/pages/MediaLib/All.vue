<template>
  <q-page class="q-pa-md">
    <q-toolbar>
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

      <template v-if="isEditDashboard">
      <q-btn-dropdown
        dense
        flat
        icon="add_circle"
        color="primary"
        label="添加图表"
      >
        <q-list dense>
          <q-item
            v-for="chartDef in chartDefines"
            :key="chartDef.index"
            @click="addChart(chartDef)"
            clickable v-close-popup
          >
            <q-item-section avatar>
              <q-icon :name="chartIcon(chartDef.type)"/>
            </q-item-section>
            <q-item-section>
              <q-item-label>{{ chartDef.title }}</q-item-label>
            </q-item-section>
          </q-item>
        </q-list>
      </q-btn-dropdown>
      </template>

      <q-btn flat round dense
        :icon="isEditDashboard ? 'check' : 'edit_square'"
        @click="isEditDashboard=!isEditDashboard"
      />
    </q-toolbar>

    <div class="grid-stack">
      <div v-for="chart in chartItems"
        class="grid-stack-item" :gs-x="chart.x" :gs-y="chart.y" :gs-w="chart.w" :gs-h="chart.h"
        :key="chart.id" :gs-id="chart.id" :id="chart.id"
        >
        <div class="grid-stack-item-content">
          <MediaTable
            v-if="chart.echarts_option._is_media_table"
            :mediaList="mediaList"
            :canRemoveTable="chart.echarts_option.toolbox?.feature?.myRemoveChart?.show"
            saveStateName="MediaLibAll"
            @remove_table="removeChart(chart)"
          />
          <v-chart class="chart" :option="chart.echarts_option" autoresize
            v-else
            @selectchanged="chartFilterOnSelected($event, chart)"
            @dataZoom="chartFilterOnZoom($event, chart)" />
          
        </div>
      </div>
    </div>

  </q-page>
</template>

<script lang="ts">


import { defineComponent, reactive, ref, onMounted, nextTick, watch } from 'vue';
import { Media } from '../../components/models';
import { getMediaLibrary, registerMediaLibChangedHandler } from 'boot/MediaLibrary';
import { makePieChartOption, makeBarChartOption, makeTimelineChartOption, makeToolboxOption,
  makeToolboxRemoveChart, makeTextChartOption, fillChartOptionTopNItems, fillTimeChartOption } from 'boot/chart';
import MediaTable from 'src/components/MediaTable.vue'
import * as crossfilter from 'crossfilter2'
import { use } from 'echarts/core';
import { CanvasRenderer } from 'echarts/renderers';
import { BarChart, PieChart } from 'echarts/charts';
import { TitleComponent, TooltipComponent, LegendComponent, GridComponent, ToolboxComponent, DataZoomComponent,
  GraphicComponent, } from 'echarts/components';
import VChart from 'vue-echarts';
import { assert, stringFormat, deepCopy, loadConfig, saveConfig } from 'boot/utils';


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
  GraphicComponent,
]);

interface ChartItem {
  x: number;
  y: number;
  w: number;
  h: number;
  id: string;
  echarts_option: any;
  data_chart_idx: number; // index of DATA_CHART_DEFINES
}

interface ChartDefine {
  index: number;
  title: string;
  type: number;
  w: number;
  h: number;
  fieldSelector: ((item: Media) => string | number) | null;
  chartOption: () =>  any;
}

type MediaLibDimension = crossfilter.Dimension<Media, string | number | Date>;

function dimensionFieldYear(item: Media) {
  return item.year == 0 ? 0 : new Date(item.year.toString()).getTime();
}

function dimensionFieldCountPlayed(item: Media) {
  let countPlayed = item.countPlayed;
  for (let i = 1; i < COUNT_PLAYED_LEVELS.length; i++) {
    if (countPlayed < COUNT_PLAYED_LEVELS[i]) {
      return COUNT_PLAYED_LEVELS[i - 1];
    }
  }
  return COUNT_PLAYED_LEVELS[COUNT_PLAYED_LEVELS.length - 1];
}

function dimensionFieldBitRate(item: Media) {
  if (item.bitRate <= 0) {
    return 'Unkown';
  }

  let bitRate = Math.round(item.bitRate / 1000 / 32) * 32;
  return bitRate + 'k';
}

function dimensionFieldDuration(item: Media) {
  let duration = Math.round(item.duration / 1000 / 60);
  for (let i = 1; i < DURATION_LEVELS.length; i++) {
    if (duration < DURATION_LEVELS[i]) {
      return DURATION_LEVELS[i - 1];
    }
  }
  return DURATION_LEVELS[DURATION_LEVELS.length - 1];
}

const CHART_TYPE_PIE = 1;
const CHART_TYPE_TIMELINE = 2;
const CHART_TYPE_RATING = 3;
const CHART_TYPE_SUMMARY = 4;
const CHART_TYPE_MEDIA_LIST = 5;

const DATA_CHART_DEFINES: Array<ChartDefine> = [
  {
    index: 0,
    title: 'Count',
    type: CHART_TYPE_SUMMARY,
    w: 2, h: 2,
    fieldSelector: null,
    chartOption: () =>  makeTextChartOption('', ''),
  },
  {
    index: 0,
    title: 'Media List',
    type: CHART_TYPE_MEDIA_LIST,
    w: 5, h: 8,
    fieldSelector: null,
    chartOption: () =>  ({ _is_media_table: true }),
  },
  {
    index: 0,
    title: 'Artist',
    type: CHART_TYPE_PIE,
    w: 3, h: 2,
    fieldSelector: (item: Media) => item.artist,
    chartOption: () =>  makePieChartOption('Artist'),
  },
  {
    index: 0,
    title: 'Album',
    type: CHART_TYPE_PIE,
    w: 3, h: 2,
    fieldSelector: (item: Media) => item.album,
    chartOption: () =>  makePieChartOption('Album'),
  },
  {
    index: 0,
    title: 'Genre',
    type: CHART_TYPE_PIE,
    w: 2, h: 2,
    fieldSelector: (item: Media) => item.genre,
    chartOption: () =>  makePieChartOption('Genre'),
  },
  {
    index: 0,
    title: 'Year',
    w: 3, h: 2,
    type: CHART_TYPE_TIMELINE,
    fieldSelector: dimensionFieldYear, chartOption: () =>  makeTimelineChartOption('Year Recorded', makeToolboxOption(true, { title: 'Filter Unkown Release Years' })),
  },
  {
    index: 0,
    title: 'Rating',
    type: CHART_TYPE_RATING,
    w: 2, h: 2,
    fieldSelector: (item: Media) => item.rating,
    chartOption: () =>  makeBarChartOption('Rating', [1,2,3,4,5], makeToolboxOption(false, { title: 'Filter NOT Rated musics' })),
  },
  {
    index: 0,
    title: 'Music Format',
    type: CHART_TYPE_PIE,
    w: 2, h: 2,
    fieldSelector: (item: Media) => item.format,
    chartOption: () =>  makePieChartOption('Format'),
  },
  {
    index: 0,
    title: 'Time Added',
    type: CHART_TYPE_TIMELINE,
    w: 3, h: 2,
    fieldSelector: (item: Media) => item.timeAdded * 1000,
    chartOption: () =>  makeTimelineChartOption('Time Added', makeToolboxOption(true, null)),
  },
  {
    index: 0,
    title: 'Time Played',
    type: CHART_TYPE_TIMELINE,
    w: 3, h: 2,
    fieldSelector: (item: Media) => item.timePlayed * 1000,
    chartOption: () =>  makeTimelineChartOption('Time Played', makeToolboxOption(true, { title: 'Filter NOT Played musics' })),
  },
  {
    index: 0,
    title: 'Lyrics Type',
    type: CHART_TYPE_PIE,
    w: 2, h: 2,
    fieldSelector: (item: Media) => item.lyricsFile,
    chartOption: () =>  makePieChartOption('Lyrics Type'),
  },
  {
    index: 0,
    title: 'Count Played',
    type: CHART_TYPE_PIE,
    w: 2, h: 2,
    fieldSelector: dimensionFieldCountPlayed,
    chartOption: () =>  makePieChartOption('Count Played'),
  },
  {
    index: 0,
    title: 'Bit Rate',
    type: CHART_TYPE_PIE,
    w: 2, h: 2,
    fieldSelector: dimensionFieldBitRate,
    chartOption: () =>  makePieChartOption('BitRate'),
  },
  {
    index: 0,
    title: 'Music Duration',
    type: CHART_TYPE_PIE,
    w: 2, h: 2,
    fieldSelector: dimensionFieldDuration,
    chartOption: () =>  makePieChartOption('Duration'),
  },
];

// https://github.com/gridstack/gridstack.js/tree/master/doc
const GridStack = (window as any).GridStack, MAX_COLUMN_COUNT = 12;
let chartIDNext = 0;
function nextChartID() {
  return 'chart_' + chartIDNext++;
}

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

function formatDurationText(n: number) {
    n /= 1000;
    let minutes = n / 60;
    let hours = minutes / 60;
    let days = hours / 24;
    if (days >= 1) {
      days = Math.floor(days);
      hours = Math.floor(hours) % 24;
      if (hours >= 1) {
        return stringFormat('{0} day(s) {1} hour(s)', days, hours);
      } else {
        return stringFormat('{0} day(s)', days);
      }
    } else {
      hours = Math.floor(hours);
      minutes = Math.floor(minutes) % 60;
      if (hours >= 1) {
        return stringFormat('{0} hour(s) {1} minute(s)', hours, minutes);
      } else {
        return stringFormat('{0} minute(s)', minutes);
      }
    }
}

// function formatDateTime(n: number) {
//   return new Date(n * 1000).toLocaleString();
// }

const COUNT_PLAYED_LEVELS = [0, 1, 5, 20, 50, 100, 500, 1000, 5000, 10000];
const DURATION_LEVELS = [1, 2, 3, 4, 5, 6, 10, 20, 50, 100];

export default defineComponent({
  name: 'PageIndex',
  components: { VChart, MediaTable },
  setup() {
    const isEditDashboard = ref(false);
    const chartDefines = DATA_CHART_DEFINES.slice();
    const mediaList = ref([] as Array<Media>);
    const chartItems = reactive([] as Array<ChartItem>);
    let grid: any = null;
    const mediaLibFilters = [] as Array<MediaLibDimension>;
    let cf = null as any as crossfilter.Crossfilter<Media>;
    initMediaLibFilters();
    refreshCharts();

    registerMediaLibChangedHandler(function() {
      initMediaLibFilters();
      refreshCharts();
    })

    onMounted(() => {
      grid = GridStack.init({
        float: false,
        cellHeight: 100,
        minRow: 1,
        oneColumnSize: 400,
        column: MAX_COLUMN_COUNT,
      });

      grid.on('change', onGridChanged);

      loadDashboard();
      grid.disable();
    });

    function initMediaLibFilters() {
      cf = crossfilter.default(getMediaLibrary().list)
      mediaLibFilters.splice(0, mediaLibFilters.length);
      let i = 0;
      for (let item of DATA_CHART_DEFINES) {
        item.index = i++;
        if (item.fieldSelector) {
          mediaLibFilters.push(cf.dimension<string | number | Date>(item.fieldSelector));
        } else {
          mediaLibFilters.push(null as any as MediaLibDimension);
        }
      }
    }

    function onGridChanged(_event: Event, changeItems: Array<any>) {
      changeItems.forEach(item => {
        var widget = chartItems.find(a => a.id == item.id) as ChartItem;
        assert(widget);
        widget.x = item.x;
        widget.y = item.y;
        widget.w = item.w;
        widget.h = item.h;
      });
    }

    function chartFilterOnSelected(param: any, chart: ChartItem) {
      const dimension = mediaLibFilters[chart.data_chart_idx];
      const option = chart.echarts_option;

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

      refreshCharts();
    }

    function chartFilterOnZoom(param: any, chart: ChartItem) {
      const dimension = mediaLibFilters[chart.data_chart_idx];
      const option = chart.echarts_option;

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

      refreshCharts();
    }

    function chartFilterOnNotSet(chart_idx: number, notSetValue: number | string) {
      const dimension = mediaLibFilters[chart_idx];
      if (!(dimension as any)._filteredNotSet) {
        dimension.filter(v => v === notSetValue);
        (dimension as any)._filteredNotSet = 1;
      } else {
        (dimension as any)._filteredNotSet = 0;
        dimension.filterAll();
      }

      refreshCharts();
    }

    function fillChartSummary(echarts_option: any) {
      const items = cf.allFiltered();
      let duration = 0;
      items.forEach(item => { duration += item.duration });
      echarts_option.graphic.elements[0].style.text = items.length;
      echarts_option.graphic.elements[1].style.text = formatDurationText(duration);
    }

    function refreshChart(chart: ChartItem) {
      const type = DATA_CHART_DEFINES[chart.data_chart_idx].type;
      const filter = mediaLibFilters[chart.data_chart_idx];
      if (type === CHART_TYPE_PIE) {
        fillChartOptionTopNItems(filter.group(), 10, chart.echarts_option, false);
      } else if (type === CHART_TYPE_TIMELINE) {
        fillTimeChartOption(filter.group(), chart.echarts_option);
      } else if (type === CHART_TYPE_RATING) {
        fillRatingItems(filter.group(), chart.echarts_option);
      } else if (type === CHART_TYPE_SUMMARY) {
        fillChartSummary(chart.echarts_option);
      } else if (type === CHART_TYPE_MEDIA_LIST) {
      } else {
        assert(0);
      }
    }

    function refreshCharts() {
      for (let chart of chartItems) {
        refreshChart(chart);
      }

      mediaList.value = cf.allFiltered();
    }

    function removeChartDefine(chart: ChartItem) {
      const index = chartDefines.findIndex(item => item.index === chart.data_chart_idx);
        if (index !== -1) {
          // 删除已经存在的图表类型
          chartDefines.splice(index, 1);
        }
    }

    function recoverChartDefine(chart: ChartItem) {
      // 按照顺序将图表定义添加回去
      for (let i = 0; i < chartDefines.length; i++) {
        if (chartDefines[i].index > chart.data_chart_idx) {
          chartDefines.splice(i, 0, DATA_CHART_DEFINES[chart.data_chart_idx]);
          return;
        }
      }

      chartDefines.push(DATA_CHART_DEFINES[chart.data_chart_idx]);
    }

    // function updatechartDefines() {
    //   for (let chart of chartItems) {
    //     removeChartDefine(chart);
    //   }
    // }

    function addChart(chart: ChartDefine) {
      function findEmptySpace(w: number, h: number) {
        for (let y = 0; y < 50; y++) {
          for (let x = 0; x < MAX_COLUMN_COUNT; x++) {
            if (grid.isAreaEmpty(x, y, w, h)) {
              return [x, y];
            }
          }
        }
        return [0, 0];
      }

      let [x, y] = findEmptySpace(chart.w, chart.h);

      addChartEx(x, y, chart.w, chart.h, chart.index);
    }

    function addChartEx(x: number, y: number, w: number, h: number, chart_index: number) {
      const echarts_option = DATA_CHART_DEFINES[chart_index].chartOption();
      const node: ChartItem = {
        x: x, y: y, w: w, h: h, id: nextChartID(),
        echarts_option, data_chart_idx: chart_index
      };

      echarts_option._filterNotSet = function () {
        chartFilterOnNotSet(chart_index, 0);
      }
      echarts_option._removeChart = function() {
        removeChart(node);
      }
      if (isEditDashboard.value) {
        deepCopy(node.echarts_option, makeToolboxRemoveChart());
      }

      removeChartDefine(node);

      chartItems.push(node);
      refreshChart(node);

      nextTick(()=>{
        grid.makeWidget(node.id);
      });
    }

    function removeChart(item: ChartItem) {
      var index = chartItems.findIndex(w => w.id == item.id);
      chartItems.splice(index, 1);
      const selector = `#${item.id}`;
      grid.removeWidget(selector, false, false);

      recoverChartDefine(item);
    }

    function chartIcon(type: number) {
      if (type === CHART_TYPE_PIE) {
        return 'pie_chart';
      } else if (type === CHART_TYPE_SUMMARY) {
        return 'text_fields';
      } else if (type === CHART_TYPE_MEDIA_LIST) {
        return 'table_chart';
      } else {
        return 'bar_chart';
      } 
    }

    const CONF_VERSION = 1;

    function loadDashboard() {
      const conf = loadConfig('media_lib_all_dashboard', CONF_VERSION);
      if (conf) {
        conf.chartItems.sort((a : any, b : any) => a.y === b.y ? a.x - b.y : a.y - b.y);

        if (conf.chartItems instanceof Array) {
          for (let chart of conf.chartItems) {
            addChartEx(chart.x, chart.y, chart.w, chart.h, chart.data_chart_idx);
          }
        }
      }
    }

    function saveDashboard(items : Array<ChartItem>) {
      const chartItems = [] as any;

      for (let chart of items) {
        chartItems.push({
          x: chart.x,
          y: chart.y,
          w: chart.w,
          h: chart.h,
          data_chart_idx: chart.data_chart_idx,
        });
      }

      saveConfig('media_lib_all_dashboard', CONF_VERSION, { chartItems });
    }

    watch(isEditDashboard, (newIsEditDashboard) => {
      grid.enableMove(newIsEditDashboard);
      grid.enableResize(newIsEditDashboard);

      for (let chart of chartItems) {
        if (newIsEditDashboard) {
          deepCopy(chart.echarts_option, makeToolboxRemoveChart());
        } else {
          if (chart.echarts_option.toolbox?.feature?.myRemoveChart) {
            chart.echarts_option.toolbox.feature.myRemoveChart.show = false;
          }
        }
      }

      if (!newIsEditDashboard) {
        // Save configuration.
        saveDashboard(chartItems);
      }
    });

    return reactive({
      isEditDashboard: isEditDashboard,
      chartItems,
      mediaList,
      chartFilterOnSelected,
      chartFilterOnZoom,
      addChart,
      removeChart,
      chartDefines,
      chartIcon,

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
    });
  },
  methods: {
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
  height: 100%;
  width: 100%;
}

.grid-stack {
  /* background: #FAFAD2; */
  /* width: 1000px; */
}

.grid-stack-item-content {
  /* color: #2c3e50; */
  /* text-align: center; */
  /* background-color: #18bc9c; */
}

.grid-stack-item-removing {
  opacity: 0.5;
}

</style>
