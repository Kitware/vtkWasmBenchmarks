#include "ConesViewer.h"

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
#if 1 //VTK_VERSION_NUMBER >= 9303023080ULL
#include <vtkWebAssemblyOpenGLRenderWindow.h>
#endif

#include <iostream>

class ConesViewer::Internal {
public:
  vtkNew<vtkRenderWindowInteractor> Interactor;
  vtkNew<vtkRenderWindow> Window;
  vtkNew<vtkRenderer> Renderer;
};

//------------------------------------------------------------------------------
ConesViewer::ConesViewer() {
  this->P.reset(new Internal());
  vtkRenderWindowInteractor::InteractorManagesTheEventLoop = false;
  std::cout << __func__ << std::endl;
  auto* p = vtkWebAssemblyOpenGLRenderWindow::SafeDownCast(this->P->Window);
  if (p) {
    p->SetCanvasId("#canvas2");
  }
  this->P->Window->SetWindowName(__func__);
}

//------------------------------------------------------------------------------
ConesViewer::~ConesViewer() {
  std::cout << __func__ << std::endl;
  this->P->Interactor->TerminateApp();
}

//------------------------------------------------------------------------------
int ConesViewer::CreateDatasets(int nx, int ny, int nz, double dx, double dy,
                                double dz) {
  std::cout << __func__ << '(' << nx << ',' << ny << ',' << nz << ',' << dx
            << ',' << dy << ',' << dz << ')' << std::endl;

  // clear previous actors.
  this->P->Renderer->RemoveAllViewProps();

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
        this->P->Renderer->AddActor(actor);
        x += dx;
      }
      x = 0.0;
      y += dy;
    }
    y = 0.0;
    z += dz;
  }
  std::cout << "Created " << nx * ny * nz << " cones" << std::endl;
  return this->P->Renderer->GetViewProps()->GetNumberOfItems();
}

//------------------------------------------------------------------------------
void ConesViewer::SetMapperStatic(bool value) {
  std::cout << __func__ << '(' << value << ')' << std::endl;
  for (const auto &viewProp : vtk::Range(this->P->Renderer->GetViewProps())) {
    if (auto actor = static_cast<vtkActor *>(viewProp)) {
      actor->GetMapper()->SetStatic(value);
    }
  }
}

//------------------------------------------------------------------------------
void ConesViewer::Azimuth(double value) {
  this->P->Renderer->GetActiveCamera()->Azimuth(value);
  this->P->Renderer->ResetCameraClippingRange();
}

//------------------------------------------------------------------------------
void ConesViewer::Initialize() {
  std::cout << __func__ << std::endl;
  // create the default renderer
  this->P->Window->AddRenderer(this->P->Renderer);
  this->P->Window->SetInteractor(this->P->Interactor);
  // set the current style to TrackBallCamera. Default is joystick
  if (auto iStyle = vtkInteractorStyle::SafeDownCast(
          this->P->Interactor->GetInteractorStyle())) {
    if (auto switchStyle = vtkInteractorStyleSwitch::SafeDownCast(iStyle)) {
      switchStyle->SetCurrentStyleToTrackballCamera();
    }
  }
}

//------------------------------------------------------------------------------
void ConesViewer::Render() {
  this->P->Window->Render();
}

//------------------------------------------------------------------------------
void ConesViewer::ResetView() {
  std::cout << __func__ << std::endl;
  auto ren = this->P->Window->GetRenderers()->GetFirstRenderer();
  if (ren != nullptr) {
    ren->ResetCamera();
  }
}

//------------------------------------------------------------------------------
void ConesViewer::SetSize(int width, int height)
{
  std::cout << __func__ << width << ',' << height << std::endl;
  this->P->Interactor->UpdateSize(width, height);
}

//------------------------------------------------------------------------------
int ConesViewer::Run() {
  std::cout << __func__ << std::endl;

  this->P->Interactor->UpdateSize(300, 300);
  this->P->Renderer->GetActiveCamera()->Elevation(30.0);
  this->P->Renderer->GetActiveCamera()->Azimuth(-40.0);
  this->P->Renderer->GetActiveCamera()->Zoom(3.0);
  this->P->Renderer->GetActiveCamera()->Roll(10.0);
  this->P->Renderer->SetBackground(1.0, 1.0, 1.0);
  this->P->Renderer->ResetCamera();
  std::cout << this->P->Window->GetClassName() << std::endl;
  this->P->Window->Render();
  this->P->Interactor->Start();
#if VTK_VERSION_NUMBER >= 93030230809ULL
  vtkWebAssemblyOpenGLRenderWindow* renderWindow = vtkWebAssemblyOpenGLRenderWindow::SafeDownCast(this->P->Window.GetPointer());
  if (renderWindow) {
    std::cout << renderWindow->GetCanvasId() << std::endl;
  }
#endif
  return 0;
}

//------------------------------------------------------------------------------
void ConesViewer::SetMouseWheelMotionFactor(float sensitivity) {
  std::cout << __func__ << "(" << sensitivity << ")" << std::endl;
  if (auto iStyle = vtkInteractorStyle::SafeDownCast(
          this->P->Interactor->GetInteractorStyle())) {
    if (auto switchStyle = vtkInteractorStyleSwitch::SafeDownCast(iStyle)) {
      switchStyle->GetCurrentStyle()->SetMouseWheelMotionFactor(sensitivity);
    } else {
      iStyle->SetMouseWheelMotionFactor(sensitivity);
    }
  }
}
