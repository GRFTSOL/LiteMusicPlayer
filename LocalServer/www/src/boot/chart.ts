
import { Group } from 'crossfilter2';
import { deepCopy } from './utils';


export function makeTextChartOption(title: string, subTitle: string, extraOptions?: any) {
    const opt = {
        graphic: {
            elements: [
                {
                    type: 'text',
                    left: 'center',
                    top: 'center',
                    style: {
                        text: title,
                        fontSize: 60,
                        fontWeight: 'bold',
                    },
                },
                {
                    type: 'text',
                    left: 'center',
                    top: '70%',
                    style: {
                        text: subTitle,
                        fontSize: 20,
                    },
                }
            ]
        }
    };

    if (extraOptions) {
        deepCopy(opt, extraOptions);
    }

    return opt;
}

export function makePieChartOption(title: string, extraOptions?: any) {
    const opt = {
        title: {
            text: title,
            left: 'center',
        },
        tooltip: {
            trigger: 'item',
            formatter: '{b} : {c} ({d}%)',
            confine: true,
        },
        // legend: {
        //   orient: 'vertical',
        //   left: 'left',
        //   data: [''],
        // },
        series: [
            {
                name: title,
                type: 'pie',
                radius: '55%',
                center: ['50%', '60%'],
                data: [],
                selectedMode: 'multiple',
                emphasis: {
                    itemStyle: {
                        shadowBlur: 10,
                        shadowOffsetX: 0,
                        shadowColor: 'rgba(0, 0, 0, 0.5)',
                    },
                },
            },
        ],
    };

    if (extraOptions) {
        deepCopy(opt, extraOptions);
    }

    return opt;
}

export function xAxisOption(isSortByXaxis?: boolean, _fixedXaxis?: boolean, fixedXaxisKeys?: Array<string | number>) {
    return {
        _isSortByXaxis: isSortByXaxis,
        _fixedXaxis: _fixedXaxis,
        _fixedXaxisKeys: fixedXaxisKeys,
    };
}

export function makeBarChartOption(title: string, fixedXaxisData?: Array<string | number>, extraOptions?: any) {
    const opt = {
        _fixedXaxis: fixedXaxisData,
        title: {
            text: title,
            left: 'center',
        },
        xAxis: {
            type: 'category',
            data: fixedXaxisData,
            axisLabel: {
                rotate: 45,
            }
        },
        yAxis: {
            type: 'value'
        },
        series: [
            {
                data: [],
                type: 'bar',
                selectedMode: 'multiple',
                emphasis: {
                    itemStyle: {
                        shadowBlur: 10,
                        shadowOffsetX: 0,
                        shadowColor: 'rgba(0, 0, 0, 0.5)',
                    },
                },
            }
        ]
    };

    if (extraOptions) {
        deepCopy(opt, extraOptions);
    }

    return opt;
}

export function makeToolboxRemoveChart() {
    return {
        toolbox: {
            show: true,
            feature: {
                myRemoveChart: {
                    show: true,
                    title: 'Remove',
                    icon: 'path://M249,753L207,711L438,480L207,249L249,207L480,438L711,207L753,249L522,480L753,711L711,753L480,522L249,753Z',
                    onclick: function (e: any) {
                        if (e.option._removeChart) {
                            e.option._removeChart();
                        } else {
                            console.warn('option._removeChart is not set.');
                        }
                    }
                }
            }
        }
    };
}

export function makeToolboxOption(dataZoom: boolean, filterNotSet: { title: string } | null) {
    const toolbox = {
        toolbox: {
            show: true,
            feature: {
            }
        }
    };

    if (dataZoom) {
        (toolbox.toolbox.feature as any).dataZoom = {
            yAxisIndex: 'none'
        };
    }

    if (filterNotSet) {
        (toolbox.toolbox.feature as any).myFilterNotSet = {
            show: true,
            title: filterNotSet.title,
            icon: 'path://M440,800Q423,800 411.5,788.5Q400,777 400,760L400,520L161,215Q147,198 157,179Q167,160 188,160L772,160Q793,160 803,179Q813,198 799,215L560,520L560,760Q560,777 548.5,788.5Q537,800 520,800L440,800ZM480,524L720,220L240,220L480,524ZM480,524L480,524L480,524L480,524Z',
            onclick: function (e: any) {
                if (e.option._filterNotSet) {
                    e.option._filterNotSet();
                } else {
                    console.warn('option._filterNotSet is not set.');
                }
            }
        };
    }

    return toolbox;
}

export function makeTimelineChartOption(title: string, extraOptions?: any) {
    const opt = {
        tooltip: {
            trigger: 'axis',
            position: function (pt: any) {
                return [pt[0], '10%'];
            }
        },
        title: {
            left: 'center',
            text: title
        },
        xAxis: {
            type: 'time',
            boundaryGap: false,
            axisLabel: {
                rotate: 20,
            }
        },
        yAxis: {
            type: 'value',
            boundaryGap: [0, '100%']
        },
        dataZoom: [
            {
                start: 0,
                end: 100
            }
        ],
        series: [
            {
                name: title,
                type: 'bar',
                smooth: true,
                symbol: 'none',
                areaStyle: {},
                data: [[0, 0]]
            }
        ]
    };

    if (extraOptions) {
        deepCopy(opt, extraOptions);
    }

    return opt;
}

export function topNItems(group: Group<any, string, number>, N: number, noOthers?: boolean | null) {
    const items = [];
    for (const item of group.top(N)) {
        if (item.value > 0) {
            items.push({
                key: item.key,
                name: item.key === '' ? 'Unknown' : item.key,
                value: item.value
            });
        }
    }

    if (!noOthers) {
        const totalCount = group.all().reduce((partialSum, item) => partialSum + item.value, 0);
        const count = items.reduce((partialSum, item) => partialSum + item.value, 0);
        if (totalCount - count > 0) {
            items.push({
                key: null,
                name: 'Others',
                value: totalCount - count,
            });
        }
    }

    return items;
}

function kvItemsToXaxisSeriesData(items: Array<any>, isSortByKey?: boolean) {
    const xAxisData: Array<string> = [], seriesData: Array<number> = [];

    items = items.splice(0);
    if (isSortByKey) {
        items.sort((a, b) => a.value - b.value);
    }

    for (const item of items) {
        xAxisData.push(item.name);
        seriesData.push(item.value);
    }

    return [xAxisData, items];
}

function kvItemsToFixedXaxis(items: Array<any>, xAxis: Array<string | number>) {
    const data = [];

    for (let i = 0; i < xAxis.length; i++) {
        const key = xAxis[i];
        let value = { key: key, name: key, value: 0 };
        for (const item of items) {
            if (item.key == key) {
                value = item;
                break;
            }
        }

        data[i] = value;
    }

    return data;
}

/**
 * 从 groupt 中筛选出 top N 的值，并填入 echart_options, 会根据 echarts 的 type 来填入对应的值.
 * 
 * @param group The crossfilter's group
 * @param N Top N
 * @param echart_options The options of echarts
 * @param noOthers Show others or NOT?
 * @returns 
 */
export function fillChartOptionTopNItems(group: Group<any, string, number>, N: number, echart_options: any, noOthers?: boolean | null) {
    const items = topNItems(group, N, noOthers);

    for (const serial of echart_options.series) {
        if (serial.type == 'pie') {
            serial.data = items;
        } else if (serial.type == 'bar') {
            if (echart_options._fixedXaxis) {
                const xAxisData = echart_options._fixedXaxisKeys || echart_options.xAxis.data;
                serial.data = kvItemsToFixedXaxis(items, xAxisData);
            } else {
                [echart_options.xAxis.data, serial.data] = kvItemsToXaxisSeriesData(items, echart_options.xAxis.data);
            }
        }
    }

    return items;
}

export function fillTimeChartOption(group: Group<any, string | number, number>, echart_options: any) {
    const items = group.all();

    (items as Array<any>).sort((a, b) => a.key - b.key);

    const data = [];
    for (const item of items) {
        if (item.key != 0) {
            data.push([item.key, item.value]);
        }
    }

    if (data.length > 0) {
        const start = data[0][0];
        const end = data[data.length - 1][0];

        if (echart_options._minTime == null) {
            echart_options._minTime = start;
        } else if (echart_options._minTime < start) {
            data.splice(0, 0, [echart_options._minTime, 0]);
        } else {
            echart_options._minTime = start;
        }

        if (echart_options._maxTime == null) {
            echart_options._maxTime = end;
        } else if (echart_options._maxTime > end) {
            data.push([echart_options._maxTime, 0]);
        } else {
            echart_options._maxTime = end;
        }
    } else {
        if (echart_options._minTime != null) {
            data.splice(0, 0, [echart_options._minTime, 0]);
        }

        if (echart_options._maxTime != null) {
            data.push([echart_options._maxTime, 0]);
        }
    }

    for (const serial of echart_options.series) {
        serial.data = data;
    }
}