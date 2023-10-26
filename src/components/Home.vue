<script lang="ts" setup>
import { ref } from 'vue';

function objectToQueryString(obj: any): string {
  const keys = Object.keys(obj);
  const keyValuePairs = keys.map(key => {
    return encodeURIComponent(key) + '=' + encodeURIComponent(obj[key]);
  });
  return keyValuePairs.join('&');
}

const benchmarks = ref([
  {
    name: "ConesViewer",
    previewQuery: {
      viewApi: 'webgl',
      showControls: 'false',
    },
    mainQuery: {
      viewApi: 'webgl',
      showControls: 'true',
    },
    viewApi: "webgl",
    id: 0,
  },
  // {
  //   name: "ConesViewer",
  //   previewQuery: {
  //     viewApi: 'webgpu',
  //     showControls: 'false',
  //   },
  //   mainQuery: {
  //     viewApi: 'webgpu',
  //     showControls: 'true',
  //   },
  //   viewApi: "webgpu",
  //   id: 1,
  // },
  // {
  //   name: "GeometryViewer",
  //   previewQuery: {
  //     viewApi: 'webgl',
  //     url: 'https://data.kitware.com/api/v1/file/6285865d4acac99f429d979b/download',
  //     showControls: 'false',
  //     colorByArray: 'Normals',
  //     colorMapPreset: 'Warm',
  //     backgroundColor1: '0xffffff', // white background in preview
  //     backgroundColor2: '0xffffff', // white background in preview
  //   },
  //   mainQuery: {
  //     viewApi: 'webgl',
  //     url: 'https://data.kitware.com/api/v1/file/6285865d4acac99f429d979b/download',
  //     showControls: 'true',
  //     colorByArray: 'Normals',
  //     colorMapPreset: 'Warm',
  //   },
  //   viewApi: "webgl",
  //   id: 2,
  // },
  // {
  //   name: "GeometryViewer",
  //   previewQuery: {
  //     viewApi: 'webgpu',
  //     url: 'https://data.kitware.com/api/v1/file/6285865d4acac99f429d979b/download',
  //     showControls: 'false',
  //     colorByArray: 'Normals',
  //     colorMapPreset: 'Warm',
  //     backgroundColor1: '0xffffff', // white background in preview
  //     backgroundColor2: '0xffffff', // white background in preview
  //   },
  //   mainQuery: {
  //     viewApi: 'webgpu',
  //     url: 'https://data.kitware.com/api/v1/file/6285865d4acac99f429d979b/download',
  //     showControls: 'true',
  //     colorByArray: 'Normals',
  //     colorMapPreset: 'Warm',
  //   },
  //   viewApi: "webgpu",
  //   id: 3,
  // },
])

</script>

<template>
  <v-container class="fill-height">
    <v-responsive class="text-center fill-height">
      <h1 class="text-body-2 font-weight-light mb-n1">VTK WebAssembly Benchmarks</h1>

      <div class="py-14" />

      <v-row class="d-flex align-center justify-center">

        <v-col v-for="benchmark in benchmarks" cols="auto" :key="benchmark.id">
          <v-card hide-details rounded :elevation="9" class="ma-0 pa-0">
            <iframe scrolling="no" :src="'#/' + benchmark.name + '?' + objectToQueryString(benchmark.previewQuery)">
            </iframe>
            <v-card-actions>
              <v-btn :to="{ name: benchmark.name, query: benchmark.mainQuery }" min-width="164" rel="noopener noreferrer"
                target="_blank" variant="text">
                <v-sheet>
                  {{ benchmark.name }} (vtk.wasm {{ benchmark.viewApi }})
                </v-sheet>
                <v-icon icon="mdi-open-in-new" />
              </v-btn>
            </v-card-actions>
          </v-card>
        </v-col>

      </v-row>
    </v-responsive>
  </v-container>
</template>

<style scoped>
iframe {
  height: 300px;
  width: 300px;
  border: none;
}
</style>