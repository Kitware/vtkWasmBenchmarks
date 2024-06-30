#pragma once

#include <memory>

#include "config.h"
class ConesViewer {
public:
  ConesViewer();
  ~ConesViewer();

  void Initialize();
  void Render();
  void ResetView();
  void SetSize(int width, int height);
  int Run();

  int CreateDatasets(int nx, int ny, int nz, double dx, double dy, double dz);
  void SetMapperStatic(bool value);

  void Azimuth(double value);
  void SetMouseWheelMotionFactor(float sensitivity);

private:
  class Internal;
  std::unique_ptr<Internal> P;
};

#ifdef __EMSCRIPTEN__
#include <emscripten/bind.h>
EMSCRIPTEN_BINDINGS(ConesViewerJSBindings) {
  emscripten::class_<ConesViewer>("ConesViewer")
      .constructor<>()
      .function("initialize", &ConesViewer::Initialize)
      .function("render", &ConesViewer::Render)
      .function("resetView", &ConesViewer::ResetView)
      .function("setSize", &ConesViewer::SetSize)
      .function("run", &ConesViewer::Run)
      .function("createDatasets", &ConesViewer::CreateDatasets)
      .function("setMapperStatic", &ConesViewer::SetMapperStatic)
      .function("azimuth", &ConesViewer::Azimuth)
      .function("setMouseWheelMotionFactor",
                &ConesViewer::SetMouseWheelMotionFactor);
}
#endif
