export function getConfiguration(viewAPI: string, webgpuDevice: GPUDevice | null) {
    return new Promise((resolve) => {
        // Sets up a base configuration for VTK-wasm with WebGL2 rendering.
        if (viewAPI === "webgl") {
            const configuration = {
                // Must be defined as 'canvas' and nothing else. Auto generated glue js expects .canvas to exist.
                canvas: (function () {
                    const canvas = <HTMLCanvasElement>document.getElementById('canvas2');
                    canvas.oncontextmenu = (event) => { event.preventDefault(); };
                    canvas.onclick = () => { canvas.focus(); }; // grab focus when the render window region receives mouse clicks.
                    canvas.tabIndex = -1;
                    canvas.addEventListener(
                        "webglcontextlost",
                        function (e) {
                            alert("WebGL context lost. You will need to reload the page.");
                            e.preventDefault();
                        },
                        false
                    );
                    return canvas;
                })(),
                //Pipes std::cout and std::cerr into debug and error in dev console.
                print: (text: string): void => console.debug(text),
                printErr: (text: string): void => console.error(text),
            };
            resolve(configuration)
        } else if (viewAPI === "webgpu") {
            // Sets up a base configuration for VTK-wasm with WebGPU rendering.
            const configuration = {
                // Must be defined as 'canvas' and nothing else. Auto generated glue js expects .canvas to exist.
                canvas: (function () {
                    const canvas = <HTMLCanvasElement>document.getElementById('canvas');
                    canvas.oncontextmenu = (event) => { event.preventDefault(); };
                    canvas.onclick = () => { canvas.focus(); }; // grab focus when the render window region receives mouse clicks.
                    canvas.tabIndex = -1;
                    return canvas;
                })(),
                //Pipes std::cout and std::cerr into debug and error in dev console.
                print: (text: string): void => console.debug(text),
                printErr: (text: string): void => console.error(text),
                preRun: [function (module: any) {
                    // select WEBGPU backend
                    module.ENV.VTK_GRAPHICS_BACKEND = 'WEBGPU';
                }],
                preinitializedWebGPUDevice: <GPUDevice | null>null,
            };
            // Set the device from JS. This can be done in C++ as well.
            // See https://github.com/kainino0x/webgpu-cross-platform-demo/blob/main/main.cpp#L51
            configuration.preinitializedWebGPUDevice = webgpuDevice;
            resolve(configuration)
        }
    });
}