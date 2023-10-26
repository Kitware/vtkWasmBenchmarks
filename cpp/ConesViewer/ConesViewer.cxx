#include <vtkActor.h>
#include <vtkCallbackCommand.h>
#include <vtkCamera.h>
#include <vtkCellData.h>
#include <vtkCollectionRange.h>
#include <vtkConeSource.h>
#include <vtkInteractorStyleSwitch.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkMinimalStandardRandomSequence.h>
#include <vtkNew.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRange.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>

#include <emscripten/emscripten.h>
#include <iostream>

static vtkSmartPointer<vtkRenderWindowInteractor> Interactor;
static vtkSmartPointer<vtkRenderWindow> Window;
static vtkSmartPointer<vtkRenderer> Renderer;

#ifdef __cplusplus
#define EXTERN extern "C"
#else
#define EXTERN
#endif

//------------------------------------------------------------------------------
EXTERN EMSCRIPTEN_KEEPALIVE void initialize() {
  std::cout << __func__ << std::endl;
  ::Interactor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
  ::Window = vtkSmartPointer<vtkRenderWindow>::New();
  ::Renderer = vtkSmartPointer<vtkRenderer>::New();
  // create the default renderer
  ::Window->SetWindowName("Cones Viewer");
  ::Window->AddRenderer(::Renderer);
  ::Window->SetInteractor(::Interactor);
  // set the current style to TrackBallCamera. Default is joystick
  if (auto iStyle = vtkInteractorStyle::SafeDownCast(
          ::Interactor->GetInteractorStyle())) {
    if (auto switchStyle = vtkInteractorStyleSwitch::SafeDownCast(iStyle)) {
      switchStyle->SetCurrentStyleToTrackballCamera();
    }
  }
  ::Window->Render();
  ::Interactor->UpdateSize(300, 300);
  ::Renderer->GetActiveCamera()->Elevation(30.0);
  ::Renderer->GetActiveCamera()->Azimuth(-40.0);
  ::Renderer->GetActiveCamera()->Zoom(3.0);
  ::Renderer->GetActiveCamera()->Roll(10.0);
  ::Renderer->SetBackground(1.0, 1.0, 1.0);
  ::Renderer->ResetCamera();
}

//------------------------------------------------------------------------------
EXTERN EMSCRIPTEN_KEEPALIVE void terminate() {
  std::cout << __func__ << std::endl;
  ::Interactor->TerminateApp();
}

//------------------------------------------------------------------------------
EXTERN EMSCRIPTEN_KEEPALIVE int createDatasets(int nx, int ny, int nz, double dx,
                                        double dy, double dz) {
  std::cout << __func__ << '(' << nx << ',' << ny << ',' << nz << ',' << dx
            << ',' << dy << ',' << dz << ')' << std::endl;

  // clear previous actors.
  ::Renderer->RemoveAllViewProps();

  // Used for randomized cell colors.
  vtkNew<vtkMinimalStandardRandomSequence> seq;

  // spacings between cones in all three directions.
  double x = 0.0, y = 0.0, z = 0.0;
  for (int k = 0; k < nz; ++k) {
    for (int j = 0; j < ny; ++j) {
      for (int i = 0; i < nx; ++i) {
        // create a cone
        vtkNew<vtkConeSource> coneSrc;
        coneSrc->SetResolution(10);
        coneSrc->SetCenter(x, y, z);
        coneSrc->Update();
        vtkPolyData *cone = coneSrc->GetOutput();

        // generate random colors for each face of the cone.
        vtkNew<vtkUnsignedCharArray> colors;
        colors->SetNumberOfComponents(4);
        seq->SetSeed(k * ny * nx + j * nx + i);
        for (vtkIdType cellId = 0; cellId < cone->GetNumberOfPolys();
             ++cellId) {
          double red = seq->GetNextRangeValue(0, 255.);
          double green = seq->GetNextRangeValue(0, 255.);
          double blue = seq->GetNextRangeValue(0, 255.);
          colors->InsertNextTuple4(red, green, blue, 255);
        }
        cone->GetCellData()->SetScalars(colors);

        vtkNew<vtkPolyDataMapper> mapper;
        mapper->SetInputData(cone);

        vtkNew<vtkActor> actor;
        actor->SetMapper(mapper);
        actor->GetProperty()->SetEdgeVisibility(1);
        actor->GetProperty()->SetEdgeColor(1.0, 1.0, 1.0);
        mapper->Update();
        actor->SetOrigin(x, y, z);
        actor->RotateZ(i * j);
        ::Renderer->AddActor(actor);
        x += dx;
      }
      x = 0.0;
      y += dy;
    }
    y = 0.0;
    z += dz;
  }
  std::cout << "Created " << nx * ny * nz << " cones" << std::endl;
  return ::Renderer->GetViewProps()->GetNumberOfItems();
}

//------------------------------------------------------------------------------
EXTERN EMSCRIPTEN_KEEPALIVE void setMapperStatic(bool value) {
  std::cout << __func__ << '(' << value << ')' << std::endl;
  for (const auto &viewProp : vtk::Range(::Renderer->GetViewProps())) {
    if (auto actor = static_cast<vtkActor *>(viewProp)) {
      actor->GetMapper()->SetStatic(value);
    }
  }
}

//------------------------------------------------------------------------------
EXTERN EMSCRIPTEN_KEEPALIVE void azimuth(double value) {
  ::Renderer->GetActiveCamera()->Azimuth(value);
  ::Renderer->ResetCameraClippingRange();
}

//------------------------------------------------------------------------------
EXTERN EMSCRIPTEN_KEEPALIVE void render() { ::Window->Render(); }

//------------------------------------------------------------------------------
EXTERN EMSCRIPTEN_KEEPALIVE void resetView() {
  std::cout << __func__ << std::endl;
  auto ren = ::Window->GetRenderers()->GetFirstRenderer();
  if (ren != nullptr) {
    ren->ResetCamera();
  }
}

//------------------------------------------------------------------------------
EXTERN EMSCRIPTEN_KEEPALIVE void setMouseWheelMotionFactor(float sensitivity) {
  std::cout << __func__ << "(" << sensitivity << ")" << std::endl;
  if (auto iStyle = vtkInteractorStyle::SafeDownCast(
          ::Interactor->GetInteractorStyle())) {
    if (auto switchStyle = vtkInteractorStyleSwitch::SafeDownCast(iStyle)) {
      switchStyle->GetCurrentStyle()->SetMouseWheelMotionFactor(sensitivity);
    } else {
      iStyle->SetMouseWheelMotionFactor(sensitivity);
    }
  }
}

int main(int argc, char *argv[]) {
  std::cout << "main" << std::endl;
  initialize();
  Interactor->Start();
}