<script lang="ts" setup>
import { onMounted, onUnmounted, ref } from "vue";
import { Properties } from "./Properties";
import { getConfiguration } from "@/utils/wasmConfigure"
import { hasWebGPU, getDevice } from "@/utils/wasmWebGPUInit";

import type { ConesViewerModule, ConesViewer } from "./ConesViewerFactory";
import createConesViewerModule from "./ConesViewer.js"

import GUI from "lil-gui"
import Stats from "stats.js";

const props = withDefaults(defineProps<Properties>(), {
  viewApi: "webgl",
  showControls: true,
  nx: 2,
  ny: 2,
  nz: 2,
  dx: 2,
  dy: 2,
  dz: 2,
  mapperIsStatic: true,
  mouseWheelMotionFactor: 0.15,
  animate: true,
});

var wasmModule: ConesViewerModule | null = null;
var gui: GUI | null = null;
var viewer: ConesViewer;
var animationRequestId: number = -1;
var fpsScript: HTMLScriptElement | null = null;
const supportsWebGPU = ref(false)

const options = {
  nx: props.nx,
  ny: props.ny,
  nz: props.nz,
  dx: props.dx,
  dy: props.dy,
  dz: props.dz,
  mapperIsStatic: props.mapperIsStatic,
  mouseWheelMotionFactor: props.mouseWheelMotionFactor,
  animate: props.animate,
}

function updateDatasets() {
  wasmModule._createDatasets(options.nx, options.ny, options.nz, options.dx, options.dy, options.dz);
  wasmModule._setMapperStatic(options.mapperIsStatic);
  wasmModule._resetView();
  wasmModule._render();
}

function spinABit() {
  wasmModule._azimuth(1);
  wasmModule._render();
  animationRequestId = requestAnimationFrame(spinABit)
}

function updateAnimateState() {
  if (options.animate) {
    spinABit();
  }
  else {
    console.log("cancel " + animationRequestId)
    cancelAnimationFrame(animationRequestId)
  }
}

function setupUI() {
  /// setup fps counter
  fpsScript = document.createElement("script");
  fpsScript.onload = () => {
    var stats = new Stats();
    document.body.appendChild(stats.dom);
    requestAnimationFrame(function loop() {
      stats.update();
      requestAnimationFrame(loop)
    });
  };
  fpsScript.src = "https://mrdoob.github.io/stats.js/build/stats.min.js";
  document.head.appendChild(fpsScript);
  /// show configuration parameters in a GUI.
  gui = new GUI();
  const datasetFolder = gui!.addFolder("Dataset Dimensions")
  datasetFolder.add(options, "nx", 1, 100).onChange(() => {
    updateDatasets();
  });
  datasetFolder.add(options, "ny", 1, 100).onChange(() => {
    updateDatasets();
  });
  datasetFolder.add(options, "nz", 1, 100).onChange(() => {
    updateDatasets();
  });
  datasetFolder.add(options, "dx", 1.0, 5.0).onChange(() => {
    updateDatasets();
  });
  datasetFolder.add(options, "dy", 1.0, 5.0).onChange(() => {
    updateDatasets();
  });
  datasetFolder.add(options, "dz", 1.0, 5.0).onChange(() => {
    updateDatasets();
  });
  gui!.add(options, "mapperIsStatic").onChange(() => {
    wasmModule._setMapperStatic(options.mapperIsStatic);
  });
  gui!.add(options, "mouseWheelMotionFactor", 0.0, 1.0).onChange(() => {
    wasmModule._setMouseWheelMotionFactor(options.mouseWheelMotionFactor);
  });
  gui!.add(options, "animate").onChange(() => {
    updateAnimateState();
  });
}

onMounted(async () => {
  console.debug("Mounted with properties ", props);
  let webgpuDevice: GPUDevice | null = null;
  if (props.viewApi == "webgpu") {
    supportsWebGPU.value = await hasWebGPU();
    if (!supportsWebGPU.value) {
      return;
    }
    webgpuDevice = await getDevice();
  }
  let configuration: any = await getConfiguration(props.viewApi, webgpuDevice);
  wasmModule = await createConesViewerModule(configuration);
  // updateDatasets();
  // updateAnimateState();
  // sends a resize event so that the render window fills up browser tab dimensions.
  // setTimeout(() => {
  //   window.dispatchEvent(new Event("resize"));
  // }, 0);
  if (props.showControls) {
    setupUI();
  }
});

onUnmounted(async () => {
  if (animationRequestId >= 0) {
    cancelAnimationFrame(animationRequestId);
  }
  if (fpsScript !== null) {
    document.body.removeChild(fpsScript);
  }
  if (gui !== null) {
    gui.destroy();
  }
  if (viewer !== undefined) {
    wasmModule._terminate();
  }
  wasmModule = null;
});

</script>

<template>
  <div v-show="!supportsWebGPU && viewApi == 'webgpu'">
    <h3>This application cannot run because your browser does not support WebGPU!</h3>
    <p>Your browser did not provide a GPU adapter. Known to happen on Linux!</p>
  </div>
  <canvas v-show="(supportsWebGPU && viewApi == 'webgpu') || viewApi == 'webgl'"
    :class="'ConesViewer' + viewApi + 'Canvas'" id="canvas"></canvas>
</template>

<style scoped>
canvas {
  position: absolute;
  left: 0px;
  top: 0px;
  display: block;
  border: 0px none;
  outline: none;
}
</style>