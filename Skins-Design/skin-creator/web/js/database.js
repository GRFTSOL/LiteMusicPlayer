
let appDataBase = Vue.createApp({
    el: '#app-database',
    data: function () {
        return {
            tableSchema: '',
            tableSampleData: '',
            dbTables: [
            ],
            dbTablePagination: {
                rowsPerPage: 10,
            },
            dbTableSelected: [],
            dbTablesColumns: [
                { name: 'database', align: 'left', label: '数据库', field: 'database', sortable: true },
                { name: 'table', align: 'left', label: '表', field: 'table', sortable: true },
            ],
        }
    },
    methods: {
        deleteTable(item) {
            var self = this;
            fetch(composeUrl('/query/', {}), {
                method: 'POST',
                cache: 'no-cache',
                body: stringFormat("DROP table {0}", item.table),
            })
            .then(stream => stream.text())
            .then(function (data) {
                self.refresh();
            })
            .catch(error => (console.error(error), this.isFetching = false));
        },
        clearTable(item) {
        },
        queryTable(item) {
            this.queryTableData(item);
            this.queryTableSchema(item);
        },
        queryTableSchema(item) {
            var self = this;
            fetch(stringFormat('/api/db/{0}/{1}/', item.database, item.table))
            .then(stream => stream.json())
            .then(function (data) {
                if (data.result == 'OK') {
                    item.schema = data.data;
                } else {
                    item.schema = data.result || data.message;
                }

                if (self.dbTableSelected.length && self.dbTableSelected[0] === item) {
                    self.tableSchema = item.schema;
                }
            })
            .catch(error => (console.error(error), this.isFetching = false));
        },
        queryTableData(item) {
            var self = this;
            fetch(composeUrl('/query/', {}), {
                method: 'POST',
                cache: 'no-cache',
                body: stringFormat("WHERE table = '{0}' | HEAD 10", item.table),
            })
            .then(stream => stream.text())
            .then(function (data) {
                item.sampleData = data;

                if (self.dbTableSelected.length && self.dbTableSelected[0] === item) {
                    self.tableSampleData = item.sampleData;
                }
            })
            .catch(error => (console.error(error), this.isFetching = false));
        },
        refresh() {
            var self = this;
            fetch(composeUrl('/api/tables/', {}))
            .then(stream => stream.json())
            .then(function (data) {
                if (data.result != 'OK') {
                    console.error('Invalid tables result: ' + JSON.stringify(data));
                    return;
                }

                let tables = [];
                let i = 0;
                for (let db of data.data) {
                    for (let table of db.tables) {
                        let info = {};
                        info.database = db.name;
                        info.table = table.name;
                        info.id = ++i;
                        tables.push(info);
                    }
                }

                self.dbTables = tables;
            })
            .catch(error => (console.error(error), this.isFetching = false));
        },
        onTableClicked(item) {
            this.tableSchema = item.schema || '';
            this.tableSampleData = item.sampleData || '';
            this.dbTableSelected = [item];

            if (!item.schema) {
                this.queryTableSchema(item);
            }

            if (!item.sampleData) {
                this.queryTableData(item);
            }
        },
    },
    mounted() {
        this.refresh();
    },
});

appDataBase.use(Quasar)
appDataBase.mount('#app-database')
