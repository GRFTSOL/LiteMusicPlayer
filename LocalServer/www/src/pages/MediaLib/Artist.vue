<template>
  <q-page class="row items-center justify-evenly">
    {{ $route.path }}
  </q-page>
</template>

<script lang="ts">

import { defineComponent } from 'vue';

export default defineComponent({
  name: 'PageIndex',
  components: {  },
  setup() {
    return { };
  }
});

</script>
