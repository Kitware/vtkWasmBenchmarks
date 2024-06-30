<script setup lang='ts'>
import type { GeometryViewerModule, GeometryViewer } from "./GeometryViewerFactory";
import { onMounted, onUnmounted, ref } from 'vue'
import { maxSize } from "../../utils/ArrayBufferLimits"
import { chunkify } from "../../utils/chunkify"
import { hexToRgb } from "../../utils/hexToRgb"
import { Properties } from "./Properties";
import { getConfiguration } from '../../utils/wasmConfigure'
import { download } from "../../utils/fileDownload"
import createGeometryViewerModule from './GeometryViewer'
import { hasWebGPU, getDevice } from "../../utils/wasmWebGPUInit";
import GUI, { Controller } from 'lil-gui'

let props = withDefaults(defineProps<Properties>(), {
  viewApi: "webgl",
  url: "",
  showControls: true,
  representation: 2,
  vertexVisibility: false,
  pointSize: 1.0,
  lineWidth: 1.0,

  colorByArray: 'Solid',
  colorMapPreset: 'Warm',
  interpolateScalarsBeforeMapping: false,

  solidColor: 0xffffff,
  vertexColor: 0x80ff80,
  edgeColor: 0x000000,
  opacity: 1.0,
  edgeOpacity: 1.0,

  mouseWheelMotionFactor: 0.15,
  animate: true,
  backgroundColor1: 0xa8e6a9,
  backgroundColor2: 0x1d2671,
  highlightOnHover: 0,
  ditherGradient: true,
  orthographic: false,
});

var wasmModule: GeometryViewerModule | null = null;
var viewer: GeometryViewer;
var gui: GUI | null = null;
var animationRequestId: number = -1;
var fpsScript: HTMLScriptElement | null = null;
const supportsWebGPU = ref(false)

// Default options
const options = {
  simulateFileInput: function () { document.getElementById('vtk-input')?.click(); },
  colorArrays: ['Solid'],
  representation: props.representation,
  vertexVisibility: props.vertexVisibility,
  pointSize: props.pointSize,
  lineWidth: props.lineWidth,

  colorByArray: props.colorByArray,
  colorMapPreset: props.colorMapPreset,
  interpolateScalarsBeforeMapping: props.interpolateScalarsBeforeMapping,

  solidColor: props.solidColor,
  vertexColor: props.vertexColor,
  edgeColor: props.edgeColor,
  opacity: props.opacity,
  edgeOpacity: props.edgeOpacity,

  mouseWheelMotionFactor: props.mouseWheelMotionFactor,
  animate: props.animate,
  backgroundColor1: props.backgroundColor1,
  backgroundColor2: props.backgroundColor2,
  highlightOnHover: props.highlightOnHover,
  ditherGradient: props.ditherGradient,
  orthographic: props.orthographic,
};

function spinABit() {
  viewer.azimuth(1);
  viewer.render();
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

var colorByArraysController: Controller;

async function loadFile(file: File) {
  if (wasmModule === null || wasmModule === undefined) {
    return;
  }
  if (viewer === null || viewer === undefined) {
    return;
  }
  let chunks = chunkify(file, /*chunkSize=*/((num: bigint) => (num < -9007199254740992n || num > 9007199254740992n) ? NaN : Number(num))(maxSize))
  let offset = 0;
  let ptr = wasmModule._malloc(file.size);
  for (let i = 0; i < chunks.length; ++i) {
    let chunk = chunks[i];
    let data = new Uint8Array(await chunk.arrayBuffer());
    wasmModule.HEAPU8.set(data, ptr + offset);
    offset += data.byteLength;
  }
  viewer.loadDataFileFromMemory(file.name, ptr, file.size);
  wasmModule._free(ptr);

  if (gui !== null) {
    var colorArrays = ['Solid', ...viewer.getPointDataArrays().split(';'), ...viewer.getCellDataArrays().split(';')];
    colorByArraysController = colorByArraysController.options(colorArrays.filter((el) => { return el.length > 0; }));
    colorByArraysController.onChange(() => {
      viewer.setColorMapPreset(options.colorMapPreset);
      viewer.setColorByArray(options.colorByArray);
      viewer.render();
    });
    viewer.setColorMapPreset(options.colorMapPreset);
    viewer.setColorByArray(options.colorByArray);
    viewer.render();
  } else {
    // set color map for the default file.
    viewer.setColorMapPreset(options.colorMapPreset);
    viewer.setColorByArray(options.colorByArray);
    viewer.render();
  }
  viewer.resetView();
  viewer.render();
}

async function onFilesChanged() {
  viewer.removeAllActors();
  colorByArraysController.setValue('Solid');
  let inputEl = document.getElementById('vtk-input') as HTMLInputElement;
  let files = inputEl.files as FileList;
  await loadFile(files[0]);
}

function setupUI() {
  /// setup fps counter
  fpsScript = document.createElement('script');
  fpsScript.onload = () => {
    var stats = new Stats();
    document.body.appendChild(stats.dom);
    requestAnimationFrame(function loop() {
      stats.update();
      requestAnimationFrame(loop)
    });
  };
  fpsScript.src = 'https://mrdoob.github.io/stats.js/build/stats.min.js';
  document.head.appendChild(fpsScript);
  /// show configuration options in a GUI.
  gui = new GUI();
  gui.add(options, 'simulateFileInput').name('Choose file');
  const meshFolder = gui.addFolder('Mesh');
  meshFolder?.add(options, 'representation', { Points: 0, Wireframe: 1, Surface: 2, SurfaceWithEdges: 3 }).onChange(() => {
    viewer.setRepresentation(options.representation);
    viewer.render();
  });
  meshFolder?.add(options, 'vertexVisibility').onChange(() => {
    viewer.setVertexVisibility(options.vertexVisibility);
    viewer.render();
  });
  meshFolder?.add(options, 'pointSize', 0.1, 10.0, 0.01).onChange(() => {
    viewer.setPointSize(options.pointSize);
    viewer.render();
  });
  meshFolder?.add(options, 'lineWidth', 0.1, 5.0, 0.01).onChange(() => {
    viewer.setLineWidth(options.lineWidth);
    viewer.render();
  });
  const colorFolder = gui.addFolder('Color');
  const scalarMapFolder = colorFolder.addFolder('Scalar Mapping');
  colorByArraysController = scalarMapFolder.add(options, 'colorByArray', options.colorArrays);
  scalarMapFolder.add(options, 'colorMapPreset', viewer.getColorMapPresets().split(';')).onChange(() => {
    viewer.setColorMapPreset(options.colorMapPreset);
    viewer.render();
  });
  scalarMapFolder?.add(options, 'interpolateScalarsBeforeMapping').onChange(() => {
    viewer.setInterpolateScalarsBeforeMapping(options.interpolateScalarsBeforeMapping);
    viewer.render();
  });
  colorFolder.addColor(options, 'solidColor').onChange(() => {
    const rgb = hexToRgb(options.solidColor);
    viewer.setColor(rgb[0], rgb[1], rgb[2]);
    viewer.render();
  });
  colorFolder.addColor(options, 'vertexColor').onChange(() => {
    const rgb = hexToRgb(options.vertexColor);
    viewer.setVertexColor(rgb[0], rgb[1], rgb[2]);
    viewer.render();
  });
  colorFolder.addColor(options, 'edgeColor').onChange(() => {
    const rgb = hexToRgb(options.edgeColor);
    viewer.setEdgeColor(rgb[0], rgb[1], rgb[2]);
    viewer.render();
  });
  colorFolder.add(options, 'opacity', 0.0, 1.0, 0.01).onChange(() => {
    viewer.setOpacity(options.opacity);
    viewer.render();
  })
  colorFolder.add(options, 'edgeOpacity', 0.0, 1.0, 0.01).onChange(() => {
    viewer.setEdgeOpacity(options.edgeOpacity);
    viewer.render();
  })
  const viewFolder = gui.addFolder('View');
  viewFolder.addColor(options, 'backgroundColor1').onChange(() => {
    const rgb = hexToRgb(options.backgroundColor1);
    viewer.setBackgroundColor1(rgb[0], rgb[1], rgb[2]);
    viewer.render();
  });
  viewFolder.addColor(options, 'backgroundColor2').onChange(() => {
    const rgb = hexToRgb(options.backgroundColor2);
    viewer.setBackgroundColor2(rgb[0], rgb[1], rgb[2]);
    viewer.render();
  });
  viewFolder.add(options, 'mouseWheelMotionFactor', 0.15, 1.0, 0.001).onChange(() => {
    viewer.setMouseWheelMotionFactor(options.mouseWheelMotionFactor);
  })
  viewFolder?.add(options, 'highlightOnHover', { None: 0, Points: 1, Cells: 2 }).onChange(() => {
    if (options.highlightOnHover == 0) {
      viewer.setHighlightOnHover(false, false);
    }
    else if (options.highlightOnHover == 1) {
      viewer.setHighlightOnHover(true, /*snapToPoint*/true);
    }
    else if (options.highlightOnHover == 2) {
      viewer.setHighlightOnHover(true, /*snapToPoint*/false);
    }
    viewer.render();
  });
  viewFolder.add(options, 'ditherGradient').onChange(() => {
    viewer.setDitherGradient(options.ditherGradient);
    viewer.render();
  });
  viewFolder.add(options, 'orthographic').onChange(() => {
    viewer.setUseOrthographicProjection(options.orthographic);
    viewer.render();
  });
  viewFolder.add(options, 'mouseWheelMotionFactor', 0.0, 1.0).onChange(() => {
    viewer.setMouseWheelMotionFactor(options.mouseWheelMotionFactor);
  });
  viewFolder.add(options, 'animate').onChange(() => {
    updateAnimateState();
  });
  viewFolder.add(
    {
      ResetView: () => {
        viewer.resetView();
        viewer.render();
      }
    }, 'ResetView');
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
  wasmModule = await createGeometryViewerModule(configuration);
  viewer = new wasmModule!.GeometryViewer();
  viewer.initialize();
  viewer.resetView();
  viewer.render();
  // starts processing events on browser main thread.
  viewer.start();
  /// connect drop events
  const dropDestination = document.getElementById('canvas2') as HTMLElement;
  dropDestination!.addEventListener('dragover', (e: DragEvent) => {
    // prevent default to allow drop
    e.preventDefault();
  }, false);

  dropDestination!.addEventListener('dragenter', () => {
    dropDestination!.classList.add('drag-active');
  });

  dropDestination!.addEventListener('dragleave', () => {
    dropDestination!.classList.remove('drag-active');
  });
  dropDestination!.addEventListener('drop', (e: DragEvent) => {
    e.preventDefault();
    dropDestination!.classList.remove('drag-active');
    const dataTransfer = e.dataTransfer as DataTransfer;
    loadFile(dataTransfer.files[0]);
  });

  if (props.showControls) {
    setupUI();
  } else {
    // white background when we're showing ourselves with other apps.
    // ideally this is exposed via a property.
    viewer.setBackgroundColor1(255, 255, 255);
    viewer.setBackgroundColor2(255, 255, 255);
  }
  // load the default file.
  if (props.url.length) {
    let { blob, filename } = await download(props.url);
    loadFile(new File([blob], filename));
  }
  updateAnimateState();
})

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
    viewer.delete();
  }
  wasmModule = null;
})

</script>

<template>
  <div v-if="!supportsWebGPU && viewApi == 'webgpu'">
    <h3>This application cannot run because your browser does not support WebGPU!</h3>
    <p>Your browser did not provide a GPU adapter. Known to happen on Linux!</p>
  </div>
  <input type='file' id='vtk-input' v-on:change="onFilesChanged" accept='.obj, .ply, .vtk, .vtp, .vtu' required>

  <div style="position: absolute; left: 0; top: 0; width: 100vw; height: 100vh;">
    <div class='canvas_container'>
      <canvas v-if="(supportsWebGPU && viewApi == 'webgpu') || viewApi == 'webgl'"
        :class="'GeometryViewer' + viewApi + 'Canvas'" id="canvas2"></canvas>
    </div>
    <div class='tooltip'
      style="-moz-user-select: none; -webkit-user-select: none; -ms-user-select:none; user-select:none;-o-user-select:none;"
      unselectable="on" onselectstart="return false;" onmousedown="return false;" oncontextmenu="event.preventDefault()">
    </div>
  </div>
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

.tooltip {
  visibility: hidden;
  background-color: black;
  color: #fff;
  text-align: center;
  border-radius: 6px;
  padding: 5px;

  /* Position the tooltip */
  position: absolute;
  opacity: 0.7;
  z-index: 1;
}

input {
  display: none;
}
</style>