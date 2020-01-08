//
// This source code is property of the Computer Graphics and Visualization 
// chair of the TU Dresden. Do not distribute in modified or unmodified form! 
// Copyright (C) 2018 CGV TU Dresden - All Rights Reserved
//

#include <vtkAutoInit.h>
VTK_MODULE_INIT( vtkRenderingOpenGL2 );
VTK_MODULE_INIT( vtkInteractionStyle );
VTK_MODULE_INIT( vtkRenderingFreeType );

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkDEMReader.h>
#include <vtkContourFilter.h>
#include <vtkImageGradientMagnitude.h>
#include <vtkWarpScalar.h>
#include <vtkDataSetMapper.h>

#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkRendererCollection.h>

#include <vtkColorTransferFunction.h>
#include <vtkScalarBarActor.h>
#include <vtkTextRenderer.h>
#include <vtkPolyDataNormals.h>
#include <vtkDataSetAttributes.h>
#include <vtkWarpScalar.h>
#include <vtkNamedColors.h>
#include <vtkLookupTable.h>

// standard includes
#include <vector>
#include <algorithm>


// ----- utility functions -----
void setGradientBackground( vtkSmartPointer<vtkRenderer> renderer )
{
	renderer->GradientBackgroundOn();
	renderer->SetBackground( 0.5, 0.5, 0.5 );
	renderer->SetBackground2( 0.9, 0.9, 0.9 );
}
// ----- end of utility functions -----


vtkSmartPointer<vtkRenderWindow> createRenderWindowFromMultipleMappers( std::vector<vtkSmartPointer<vtkMapper>> mappers )
{
	// create renderer and window
	vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
	vtkSmartPointer<vtkRenderWindow> window = vtkSmartPointer<vtkRenderWindow>::New();
	vtkSmartPointer<vtkActor> warpActor = vtkSmartPointer<vtkActor>::New();
	vtkSmartPointer<vtkActor> contourActor = vtkSmartPointer<vtkActor>::New();

	// let's have a fancy background for the canvas
	setGradientBackground( renderer );

	// for every mapper we create an actor and add it to the renderer
	warpActor->GetProperty()->SetColor(1, 1, 1);
	contourActor->GetProperty()->SetColor(1, 1, 1);

	// connect everything
	warpActor->SetMapper(mappers[1]);
	contourActor->SetMapper(mappers[0]);
	renderer->AddActor(warpActor);
	renderer->AddActor(contourActor);
	window->AddRenderer(renderer);

	// set window size and return
	window->SetSize( 800, 800 );
	return window;
}

void doRenderingAndInteraction( vtkSmartPointer<vtkRenderWindow> window )
{
    // create interactor and connect a window
	vtkSmartPointer<vtkRenderWindowInteractor> interactor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	interactor->SetRenderWindow( window );
	// set an interaction style
	interactor->SetInteractorStyle( vtkInteractorStyleTrackballCamera::New() );

    // execute render/interaction loop
    interactor->Initialize();
    interactor->Start();

    // close the window when finished
    window->Finalize();
}

int main(int argc, char * argv[])
{
    // -- begin of basic visualization network definition --

	// 1. create source
	vtkSmartPointer<vtkDEMReader> source = vtkSmartPointer<vtkDEMReader>::New();
	source->SetFileName( "../data/SainteHelens.dem" );
	
	// 2. create filters
	// a) warped filter
	vtkSmartPointer<vtkWarpScalar> warpFilter = vtkSmartPointer<vtkWarpScalar>::New();
	// set scale factor to 2
	warpFilter->SetScaleFactor(2);
	// use source as filter input
	warpFilter->SetInputConnection(source->GetOutputPort());

    // b) contour filter
	vtkSmartPointer<vtkContourFilter> contourFilter = vtkSmartPointer<vtkContourFilter>::New();
	// use source as filter input
	contourFilter->SetInputConnection(warpFilter->GetOutputPort());
	// set the height value at which the contour line shall be drawn
	
	//*********** Possibly use for loop instead of using directly setting them
	/*for (int i = 0; i < 19; i++) {
		int height = 790;
		contourFilter->SetValue(i, height);
		height = height + 90;
	}*/
	contourFilter->SetValue(0, 700);
	contourFilter->SetValue(1, 790);
	contourFilter->SetValue(2, 880);
	contourFilter->SetValue(3, 970);
	contourFilter->SetValue(4, 1060);
	contourFilter->SetValue(5, 1150);
	contourFilter->SetValue(6, 1240);
	contourFilter->SetValue(7, 1330);
	contourFilter->SetValue(8, 1420);
	contourFilter->SetValue(9, 1510);
	contourFilter->SetValue(10, 1600);
	contourFilter->SetValue(11, 1790);
	contourFilter->SetValue(12, 1880);
	contourFilter->SetValue(13, 1970);
	contourFilter->SetValue(14, 2060);
	contourFilter->SetValue(15, 2150);
	contourFilter->SetValue(16, 2240);
	contourFilter->SetValue(17, 2330);
	contourFilter->SetValue(18, 2420);
	contourFilter->SetValue(19, 2510);

	// 3.  create mappers
	// a) contour mapper, show the regions where the data has a specific value
	vtkSmartPointer<vtkDataSetMapper> contourMapper = vtkSmartPointer<vtkDataSetMapper>::New();
	// connect to the contour filter output (the pipeline is source->contourFilter->contourMapper->...)
	contourMapper->SetInputConnection( contourFilter->GetOutputPort() );
	// avoid z-buffer fighting with small polygon shift
	contourMapper->SetResolveCoincidentTopologyToPolygonOffset();
	// don't use the scalar value to color the line, see fallback coloring via actor in createRenderWindowFromMapper
	contourMapper->ScalarVisibilityOff();

	// b) warp mapper
	vtkSmartPointer<vtkDataSetMapper> warpMapper = vtkSmartPointer<vtkDataSetMapper>::New();
	// connect to the gradient filter output (the pipeline is source->warpFilter->warpMapper->...)
	warpMapper->SetInputConnection(warpFilter->GetOutputPort());
	// set the range of the gradient magnitudes in order to correctly map the colors
	warpMapper->SetScalarRange(0, 2);

    // -- end of basic visualization network definition --
	std::vector<vtkSmartPointer<vtkMapper>> combinedMapper;
	combinedMapper.push_back(warpMapper);
	combinedMapper.push_back(contourMapper);

	vtkSmartPointer<vtkScalarBarActor> scalarBar =
		vtkSmartPointer<vtkScalarBarActor>::New();

	scalarBar->SetLookupTable(warpMapper->GetLookupTable());
	scalarBar->SetTitle("Elevation");
	scalarBar->SetNumberOfLabels(10);

	// Create a lookup table to share between the mapper and the scalarbar
	vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction =
		vtkSmartPointer<vtkColorTransferFunction>::New();
	colorTransferFunction->AddRGBPoint(682, 0, 0, 0);
	colorTransferFunction->AddRGBPoint(1302, 1, 0, 0);
	colorTransferFunction->AddRGBPoint(1922, 1, 1, 0);
	colorTransferFunction->AddRGBPoint(2543, 1, 1, 1);

	vtkSmartPointer<vtkLookupTable> hueLut =
		vtkSmartPointer<vtkLookupTable>::New();
	hueLut->SetTableRange(682, 2543);
	hueLut->SetHueRange(0, 1);
	hueLut->SetSaturationRange(1, 1);
	hueLut->SetValueRange(1, 1);
	hueLut->Build();

	warpMapper->SetLookupTable(colorTransferFunction);
	scalarBar->SetLookupTable(colorTransferFunction);

    // 4. create actors, renderers, render windows 	
	vtkSmartPointer<vtkRenderWindow> combinedWindow = createRenderWindowFromMultipleMappers(combinedMapper);
	combinedWindow->GetRenderers()->GetFirstRenderer()->AddActor2D(scalarBar);

    // 5. successively show each window and allow user interaction (until it is closed)
	doRenderingAndInteraction( combinedWindow);

    return 0;
}