<template>
  <q-page class="q-pa-md">
    <q-toolbar>
      <q-input dense debounce="400" color="primary" v-model="searchKeyword" class="q-pr-sm" hide-bottom-space>
        <template v-slot:append>
          <q-icon name="search" />
        </template>
      </q-input>

      <div class="q-gutter-xs row">
        <template
          v-for="item in filterOptions"
          :key="item.id"
        >
          <q-chip
            removable
            @remove="removeFilterCondition(item)"
            color="primary"
            text-color="white"
            style="max-width: 150px" class="truncate-chip-labels"
            :icon="item.icon"
            :label="item.filterCondition">
          </q-chip>
        </template>

        <q-btn
          v-show="filterOptions.length"
          flat round dense
          icon="playlist_remove"
          @click="removeAllFilterOptions"
        />
      </div>

      <q-space />

      <template v-if="isEditDashboard && chartDefines.length">
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
            :ref="(el) => chart.instance = el"
            @selectchanged="chartFilterOnSelected($event, chart)"
            @dataZoom="chartFilterOnZoom($event, chart)"
          />
        </div>
      </div>
    </div>

  </q-page>
</template>

<script lang="ts">


import { defineComponent, reactive, ref, onMounted, nextTick, watch } from 'vue';
import { Media } from '../../components/models';
import { getMediaLibrary, registerMediaLibChangedHandler } from 'boot/MediaLibrary';
import { makePieChartOption, makeBarChartOption, makeTimelineChartOption, makeToolboxOption, makeSunBurstOption,
  makeToolboxRemoveChart, makeTextChartOption, fillChartOptionTopNItems, fillTimeChartOption } from 'boot/chart';
import MediaTable from 'src/components/MediaTable.vue'
import * as crossfilter from 'crossfilter2'
import { use } from 'echarts/core';
import { CanvasRenderer } from 'echarts/renderers';
import { BarChart, PieChart, SunburstChart, TreemapChart } from 'echarts/charts';
import { TitleComponent, TooltipComponent, LegendComponent, GridComponent, ToolboxComponent, DataZoomComponent,
  GraphicComponent, } from 'echarts/components';
import VChart from 'vue-echarts';
import { assert, stringFormat, deepCopy, loadConfig, saveConfig } from 'boot/utils';


use([
  CanvasRenderer,
  PieChart,
  BarChart,
  SunburstChart,
  TreemapChart,
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
  icon: string;
  filterCondition: string;
  selected: Array<number>;
  instance: any;
  conf?: any,
}

interface ChartDefine {
  index: number;
  title: string;
  type: number;
  w: number;
  h: number;
  fieldSelector: ((item: Media) => string | number) | null;
  chartOption: () =>  any;
  icon: string;
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
const CHART_TYPE_MEDIA_TREE = 6;

const DATA_CHART_DEFINES: Array<ChartDefine> = [
  {
    index: 0,
    title: 'Count',
    type: CHART_TYPE_SUMMARY,
    w: 2, h: 2,
    fieldSelector: null,
    chartOption: () =>  makeTextChartOption('', ''),
    icon: 'pin',
  },
  {
    index: 0,
    title: 'Media List',
    type: CHART_TYPE_MEDIA_LIST,
    w: 5, h: 8,
    fieldSelector: null,
    chartOption: () =>  ({ _is_media_table: true }),
    icon: 'view_list',
  },
  {
    index: 0,
    title: 'Artist',
    type: CHART_TYPE_PIE,
    w: 3, h: 2,
    fieldSelector: (item: Media) => item.artist,
    chartOption: () =>  makePieChartOption('Artist'),
    icon: 'face',
  },
  {
    index: 0,
    title: 'Album',
    type: CHART_TYPE_PIE,
    w: 3, h: 2,
    fieldSelector: (item: Media) => item.album,
    chartOption: () =>  makePieChartOption('Album'),
    icon: 'album',
  },
  {
    index: 0,
    title: 'Media Tree',
    type: CHART_TYPE_MEDIA_TREE,
    w: 3, h: 4,
    fieldSelector: (item: Media) => item.url,
    chartOption: () =>  makeSunBurstOption('Media Tree'),
    icon: 'brightness_7',
  },
  {
    index: 0,
    title: 'Genre',
    type: CHART_TYPE_PIE,
    w: 2, h: 2,
    fieldSelector: (item: Media) => item.genre,
    chartOption: () =>  makePieChartOption('Genre'),
    icon: 'category',
  },
  {
    index: 0,
    title: 'Year',
    w: 3, h: 2,
    type: CHART_TYPE_TIMELINE,
    fieldSelector: dimensionFieldYear, chartOption: () =>  makeTimelineChartOption('Year Recorded', makeToolboxOption(true, { title: 'Filter Unkown Release Years' })),
    icon: 'calendar_month',
  },
  {
    index: 0,
    title: 'Rating',
    type: CHART_TYPE_RATING,
    w: 2, h: 2,
    fieldSelector: (item: Media) => item.rating,
    chartOption: () =>  makeBarChartOption('Rating', [1,2,3,4,5], makeToolboxOption(false, { title: 'Filter NOT Rated musics' })),
    icon: 'stars',
  },
  {
    index: 0,
    title: 'Music Format',
    type: CHART_TYPE_PIE,
    w: 2, h: 2,
    fieldSelector: (item: Media) => item.format,
    chartOption: () =>  makePieChartOption('Format'),
    icon: 'description',
  },
  {
    index: 0,
    title: 'Time Added',
    type: CHART_TYPE_TIMELINE,
    w: 3, h: 2,
    fieldSelector: (item: Media) => item.timeAdded * 1000,
    chartOption: () =>  makeTimelineChartOption('Time Added', makeToolboxOption(true, null)),
    icon: 'schedule',
  },
  {
    index: 0,
    title: 'Time Played',
    type: CHART_TYPE_TIMELINE,
    w: 3, h: 2,
    fieldSelector: (item: Media) => item.timePlayed * 1000,
    chartOption: () =>  makeTimelineChartOption('Time Played', makeToolboxOption(true, { title: 'Filter NOT Played musics' })),
    icon: 'play_arrow',
  },
  {
    index: 0,
    title: 'Lyrics Type',
    type: CHART_TYPE_PIE,
    w: 2, h: 2,
    fieldSelector: (item: Media) => item.lyricsFile,
    chartOption: () =>  makePieChartOption('Lyrics Type'),
    icon: 'lyrics',
  },
  {
    index: 0,
    title: 'Count Played',
    type: CHART_TYPE_PIE,
    w: 2, h: 2,
    fieldSelector: dimensionFieldCountPlayed,
    chartOption: () =>  makePieChartOption('Count Played'),
    icon: 'functions',
  },
  {
    index: 0,
    title: 'Bit Rate',
    type: CHART_TYPE_PIE,
    w: 2, h: 2,
    fieldSelector: dimensionFieldBitRate,
    chartOption: () =>  makePieChartOption('BitRate'),
    icon: '6k_plus',
  },
  {
    index: 0,
    title: 'Music Duration',
    type: CHART_TYPE_PIE,
    w: 2, h: 2,
    fieldSelector: dimensionFieldDuration,
    chartOption: () =>  makePieChartOption('Duration'),
    icon: 'hourglass_top',
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

interface MediaTree {
    name: string;
    value: number;     // 歌曲数量
    path: string,
    mediaDuration: number;  // 总共时长，单位秒
    children: Array<MediaTree>;
}

function nextPathPart(path: string) {
  let parts = path.split(/\/|\\/);
  parts = parts.slice(0, parts.length - 1);
  let i = 0;
  return function() {
    return parts[i++];
  }
}

function addMediaTree(parent: MediaTree, nextPathPart: ()=>string | null, duration: number) {
    parent.mediaDuration += duration;
    parent.value++;

    const name = nextPathPart();
    if (name == null) {
        return;
    }

    for (const child of parent.children) {
        if (child.name == name) {
            addMediaTree(child, nextPathPart, duration);
            return;
        }
    }

    // 添加
    const node = {
      name,
      value: 1,
      path: parent.path + name + getMediaLibrary().path_sep,
      mediaDuration: duration,
      children: [],
    };
    parent.children.push(node);

    addMediaTree(node, nextPathPart, duration);
}

function simplifyMediaTree(parent: MediaTree) {
  while (parent.children.length === 1) {
    parent.name = parent.children[0].name;
    parent.children = parent.children[0].children;
  }

  for (const child of parent.children) {
    simplifyMediaTree(child);
  }
}

function getSelectedMediaTreeItem(root: MediaTree, expected_idx: number) {
  let idx = 2;

  function search(parent: MediaTree) : MediaTree | null {
    for (const child of parent.children) {
      if (idx === expected_idx) {
        return child;
      }
      idx++;
      const ret = search(child);
      if (ret) {
        return ret;
      }
    }

    return null;
  }

  return search(root);  
}

function constructMediaTree(medias: Array<Media>) : MediaTree {
  const root: MediaTree = {
    name: '',
    value: 0,
    path: '',
    mediaDuration: 0,
    children: [] as Array<MediaTree>,
  };

  for (const media of medias) {
    addMediaTree(root, nextPathPart(media.url), media.duration);
  }

  simplifyMediaTree(root);
  return root;
}

const COUNT_PLAYED_LEVELS = [0, 1, 5, 20, 50, 100, 500, 1000, 5000, 10000];
const DURATION_LEVELS = [1, 2, 3, 4, 5, 6, 10, 20, 50, 100];

export default defineComponent({
  name: 'PageIndex',
  components: { VChart, MediaTable },
  setup() {
    const isEditDashboard = ref(false);
    const searchKeyword = ref('');
    const chartDefines = DATA_CHART_DEFINES.slice();
    const mediaList = ref([] as Array<Media>);
    const mediaTree = ref(null as any as MediaTree);
    const chartItems = reactive([] as Array<ChartItem>);
    const filterOptions = reactive([] as Array<ChartItem>);
    let grid: any = null;
    const mediaLibFilters = [] as Array<MediaLibDimension>;
    let filterKeyword = null as any as MediaLibDimension;
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

      filterKeyword = cf.dimension<string | number | Date>(item => (item.artist + ' ' + item.album + ' ' + item.title + ' ' + item.url + ' ' + item.genre).toLowerCase());
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

    function removeFilterCondition(chart: ChartItem) {
      // 只有将 selectedMode 切换为 series 才能删除所有选中的，之后再恢复为 multiple
      chart.echarts_option.series[0].selectedMode = 'series'
      chart.instance.setOption(chart.echarts_option);
      chart.instance.dispatchAction({
        type: 'unselect',
        seriesIndex: 0,
        dataIndex: chart.selected,
      });
      chart.echarts_option.series[0].selectedMode = 'multiple'
      chart.instance.setOption(chart.echarts_option);

      if (chart.echarts_option.dataZoom) {
        chart.instance.dispatchAction({
          type: 'dataZoom',
          dataZoomIndex: 0,
        });
      }

      removeFilter(chart);
    }

    function addFilter(chart: ChartItem) {
      const idx = filterOptions.indexOf(chart);
      if (idx === -1) {
        filterOptions.push(chart);
      }
    }

    function removeFilter(chart: ChartItem) {
      const idx = filterOptions.indexOf(chart);
      if (idx !== -1) {
        filterOptions.splice(idx, 1);
      }
    }

    function removeAllFilterOptions() {
      let tmp = filterOptions.slice(0);
      for (let chart of tmp) {
        removeFilterCondition(chart);
      }
      searchKeyword.value = '';
    }

    function chartFilterOnSelected(param: any, chart: ChartItem) {
      const dimension = mediaLibFilters[chart.data_chart_idx];
      if (!dimension) {
        return;
      }

      const option = chart.echarts_option;

      const idx = param.fromActionPayload.dataIndexInside;
      if (DATA_CHART_DEFINES[chart.data_chart_idx].type === CHART_TYPE_MEDIA_TREE && idx > 1) {
        const ret = getSelectedMediaTreeItem(mediaTree.value, idx);
        assert(ret);
        const path = ret?.path as string;
        dimension.filter(v => (v as string).startsWith(path));
        chart.filterCondition = ret?.name as string;
        addFilter(chart);
      } else if (param.selected.length > 0) {
        // 有过滤选项
        let keys = {} as any, names = [] as Array<string>;
        for (let idx of param.selected[0].dataIndex) {
          const item = option.series[0].data[idx] as any;
          if (item) {
            if (item.key === null) {
              // 选择了 Others，去掉前 N 个结果就是 other
              let others = dimension.group().top(Number.MAX_VALUE).slice(option.series[0].data.length - 1);
              for (let item of others) {
                keys[(item as any).key] = 1;
                names.push((item as any).key);
              }
            } else {
              keys[item.key] = 1;
              names.push(item.name);
            }
          }
        }

        // chart.selected = names as any;
        chart.selected = param.selected[0].dataIndex;
        chart.filterCondition = names.join(',');
        if (names.length) {
          dimension.filter(v => keys[v as any] === 1);
          addFilter(chart);
        } else {
          removeFilter(chart);
          dimension.filterAll();
        }
      } else {
        chart.filterCondition = '';
        removeFilter(chart);
        dimension.filterAll();
      }

      refreshCharts();
    }

    function chartFilterOnZoom(param: any, chart: ChartItem) {
      const dimension = mediaLibFilters[chart.data_chart_idx];
      const option = chart.echarts_option;

      if (param.startValue == null) {
        if (param.start == null) {
          if (param.batch && param.batch[0]) {
            param.startValue = param.batch[0].startValue;
            param.endValue = param.batch[0].endValue;
          }
        } else {
          const data = option.series[0].data;
          const start = data[0][0], end = data[data.length - 1][0];
          param.startValue = start + (end - start) * param.start / 100;
          param.endValue = start + (end - start) * param.end / 100;
        }
      }

      const zoom = option.dataZoom[0];
      zoom.start = param.start;
      zoom.end = param.end;

      if (param.startValue != null) {
        // 有过滤选项
        dimension.filter(v => v >= param.startValue && v <= param.endValue);
        addFilter(chart);
        chart.filterCondition = stringFormat('{0} ~ {1}', new Date(param.startValue).toISOString(), new Date(param.endValue).toISOString());
      } else {
        chart.filterCondition = '';
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
      } else if (type === CHART_TYPE_MEDIA_TREE) {
        chart.echarts_option.series[0].data = [mediaTree.value];
      } else {
        assert(0);
      }
    }

    function refreshCharts() {
      mediaList.value = cf.allFiltered();
      mediaTree.value = constructMediaTree(mediaList.value);

      for (let chart of chartItems) {
        refreshChart(chart);
      }
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

    function addChartEx(x: number, y: number, w: number, h: number, chart_index: number, conf?: any) {
      const chartDefine = DATA_CHART_DEFINES[chart_index];
      const echarts_option = chartDefine.chartOption();
      const node: ChartItem = {
        x: x, y: y, w: w, h: h, id: nextChartID(),
        echarts_option,
        data_chart_idx: chart_index,
        filterCondition: '',
        selected: [],
        icon: chartDefine.icon,
        instance: null,
      };

      echarts_option._filterNotSet = function () {
        chartFilterOnNotSet(chart_index, 0);
      };
      echarts_option._removeChart = function() {
        removeChart(node);
      };
      echarts_option._switchSunburstTreemap = function() {
        switchSunburstTreemap();
        node.instance.setOption(node.echarts_option);
      };

      if (chartDefine.type === CHART_TYPE_MEDIA_TREE && conf === 'treemap') {
        switchSunburstTreemap();
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

      function switchSunburstTreemap() {
        const s0 = node.echarts_option.series[0];
        node.conf = s0.type = s0.type === 'sunburst' ? 'treemap' : 'sunburst';
        if (s0.type === 'sunburst') {
          s0.label = {
            show: true,
            overflow: 'truncate',
            align: 'left',
          };
        } else {
          s0.label = {
            show: true,
          };
        }

        if (!isEditDashboard.value) {
          saveDashboard();
        }
      }
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
      } else if (type === CHART_TYPE_MEDIA_LIST || type === CHART_TYPE_MEDIA_TREE) {
        return 'table_chart';
      } else {
        return 'bar_chart';
      } 
    }

    const CONF_VERSION = 1;

    function loadDashboard() {
      try {
        const conf = loadConfig('media_lib_all_dashboard', CONF_VERSION, {'chartItems':[{'x':0,'y':0,'w':2,'h':2,'data_chart_idx':0},{'x':7,'y':0,'w':5,'h':7,'data_chart_idx':1},{'x':2,'y':0,'w':3,'h':4,'data_chart_idx':4,'conf':'sunburst'},{'x':5,'y':0,'w':2,'h':2,'data_chart_idx':5},{'x':0,'y':2,'w':2,'h':2,'data_chart_idx':7},{'x':5,'y':2,'w':2,'h':2,'data_chart_idx':8},{'x':0,'y':4,'w':2,'h':2,'data_chart_idx':12},{'x':2,'y':4,'w':3,'h':2,'data_chart_idx':2},{'x':5,'y':4,'w':2,'h':2,'data_chart_idx':13},{'x':3,'y':6,'w':3,'h':2,'data_chart_idx':9},{'x':0,'y':6,'w':3,'h':2,'data_chart_idx':6},{'x':6,'y':7,'w':2,'h':2,'data_chart_idx':11},{'x':8,'y':7,'w':2,'h':2,'data_chart_idx':14},{'x':0,'y':8,'w':3,'h':2,'data_chart_idx':3},{'x':3,'y':8,'w':3,'h':2,'data_chart_idx':10}],'_version':1});
        if (conf && conf.chartItems) {
          conf.chartItems.sort((a : any, b : any) => a.y === b.y ? a.x - b.y : a.y - b.y);

          if (conf.chartItems instanceof Array) {
            for (let chart of conf.chartItems) {
              addChartEx(chart.x, chart.y, chart.w, chart.h, chart.data_chart_idx, chart.conf);
            }
          }
        }
      } catch (e) {
        console.error(e);
      }
    }

    function saveDashboard() {
      const items = [] as any;

      for (let chart of chartItems) {
        items.push({
          x: chart.x,
          y: chart.y,
          w: chart.w,
          h: chart.h,
          data_chart_idx: chart.data_chart_idx,
          conf: chart.conf,
        });
      }

      saveConfig('media_lib_all_dashboard', CONF_VERSION, { chartItems: items });
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
        saveDashboard();
      }
    });

    watch(searchKeyword, (newSearchKeyword) => {
      newSearchKeyword = newSearchKeyword.toLowerCase();
      filterKeyword.filter(v => (v as string).indexOf(newSearchKeyword) != -1);
      refreshCharts();
    });

    return reactive({
      isEditDashboard,
      chartItems,
      mediaList,
      chartFilterOnSelected,
      chartFilterOnZoom,
      addChart,
      removeChart,
      removeFilterCondition,
      chartDefines,
      chartIcon,

      searchKeyword,
      filterOptions,
      removeAllFilterOptions,
    });
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
