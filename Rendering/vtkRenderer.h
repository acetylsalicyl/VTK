/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


Copyright (c) 1993-1995 Ken Martin, Will Schroeder, Bill Lorensen.

This software is copyrighted by Ken Martin, Will Schroeder and Bill Lorensen.
The following terms apply to all files associated with the software unless
explicitly disclaimed in individual files. This copyright specifically does
not apply to the related textbook "The Visualization Toolkit" ISBN
013199837-4 published by Prentice Hall which is covered by its own copyright.

The authors hereby grant permission to use, copy, and distribute this
software and its documentation for any purpose, provided that existing
copyright notices are retained in all copies and that this notice is included
verbatim in any distributions. Additionally, the authors grant permission to
modify this software and its documentation for any purpose, provided that
such modifications are not distributed without the explicit consent of the
authors and that existing copyright notices are retained in all copies. Some
of the algorithms implemented by this software are patented, observe all
applicable patent law.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.


=========================================================================*/
// .NAME vtkRenderer - abstract specification for renderers
// .SECTION Description
// vtkRenderer provides an abstract specification for renderers. A renderer
// is an object that controls the rendering process for objects. Rendering
// is the process of converting geometry, a specification for lights, and 
// a camera view into an image. vtkRenderer also performs coordinate 
// transformation between world coordinates, view coordinates (the computer
// graphics rendering coordinate system), and display coordinates (the 
// actual screen coordinates on the display device).

#ifndef __vtkRenderer_hh
#define __vtkRenderer_hh

#include "vtkObject.hh"
#include "vtkMatrix4x4.hh"
#include "vtkLightCollection.hh"
#include "vtkCamera.hh"
#include "vtkActorCollection.hh"
#include "vtkGeometryPrimitive.hh"

class vtkRenderWindow;
class vtkVolumeRenderer;

class vtkRenderer : public vtkObject
{
public:
  vtkRenderer();
  ~vtkRenderer();
  char *GetClassName() {return "vtkRenderer";};
  void PrintSelf(ostream& os, vtkIndent indent);

  void AddLights(vtkLight *);
  void AddActors(vtkActor *);
  void RemoveLights(vtkLight *);
  void RemoveActors(vtkActor *);
  vtkLightCollection *GetLights();
  vtkActorCollection *GetActors();
  void SetActiveCamera(vtkCamera *);
  vtkCamera *GetActiveCamera();
  void SetVolumeRenderer(vtkVolumeRenderer *);
  vtkVolumeRenderer *GetVolumeRenderer();

  // Description:
  // Set the background color of the rendering screen using an rgb color
  // specification.
  vtkSetVector3Macro(Background,float);
  vtkGetVectorMacro(Background,float,3);

  // Description:
  // Set the aspect ratio of the rendered image.
  vtkSetVector2Macro(Aspect,float);
  vtkGetVectorMacro(Aspect,float,2);

  // Description:
  // Set the level of ambient lighting.
  vtkSetVector3Macro(Ambient,float);
  vtkGetVectorMacro(Ambient,float,3);

  // Description:
  // Turn on/off whether objects are lit from behind with another light.
  // If backlighting is on, for every light that is created, a second 
  // opposing light is created to backlight the object.
  vtkSetMacro(BackLight,int);
  vtkGetMacro(BackLight,int);
  vtkBooleanMacro(BackLight,int);

  // Description:
  // Create an image.
  virtual void Render() = 0;

  // Description:
  // Get a device specific geometry representation.
  virtual vtkGeometryPrimitive *GetPrimitive(char *) = 0;
  
  // Description:
  // Ask all actors to build and draw themselves.
  virtual int UpdateActors(void) = 0;

  // Description:
  // Ask the camera to load its view matrix.
  virtual int UpdateCameras(void) = 0;

  // Description:
  // Ask all lights to load themselves into rendering pipeline.
  virtual int UpdateLights(void) = 0;

  void DoCameras();
  void DoLights();
  void DoActors();
  void ResetCamera();
  void ResetCamera(float bounds[6]);

  void SetRenderWindow(vtkRenderWindow *);
  vtkRenderWindow *GetRenderWindow() {return RenderWindow;};
  
  // Description:
  // Specify a point location in display (or screen) coordinates.
  vtkSetVector3Macro(DisplayPoint,float);
  vtkGetVectorMacro(DisplayPoint,float,3);

  // Description:
  // Specify a point location in view coordinates.
  vtkSetVector3Macro(ViewPoint,float);
  vtkGetVectorMacro(ViewPoint,float,3);

  // Description:
  // Specify a point location in world coordinates.
  vtkSetVector4Macro(WorldPoint,float);
  vtkGetVectorMacro(WorldPoint,float,4);

  // Description:
  // Specify the area for the renderer to draw in the rendering window. 
  // Coordinates are expressed as (xmin,ymin,xmax,ymax) where each
  // coordinate is 0 <= coordinate <= 1.0.
  vtkSetVector4Macro(Viewport,float);
  vtkGetVectorMacro(Viewport,float,4);

  virtual float *GetCenter();

  virtual void DisplayToView(); // these get modified in subclasses
  virtual void ViewToDisplay(); // to handle stereo rendering
  virtual int  IsInViewport(int x,int y); 
  void WorldToView();
  void ViewToWorld();
  void DisplayToWorld();
  void WorldToDisplay();

  void SetStartRenderMethod(void (*f)(void *), void *arg);
  void SetEndRenderMethod(void (*f)(void *), void *arg);
  void SetStartRenderMethodArgDelete(void (*f)(void *));
  void SetEndRenderMethodArgDelete(void (*f)(void *));

protected:
  vtkVolumeRenderer *VolumeRenderer;
  vtkCamera *ActiveCamera;
  vtkLight  *CreatedLight;
  vtkLightCollection Lights;
  vtkActorCollection Actors;
  float Ambient[3];  
  float Background[3];  
  int BackLight;
  vtkRenderWindow *RenderWindow;
  float DisplayPoint[3];
  float ViewPoint[3];
  float WorldPoint[4];
  float Viewport[4];
  float Aspect[2];
  float Center[2];
  int   SelfCreatedCamera;
  int   SelfCreatedLight;
  
  void (*StartRenderMethod)(void *);
  void (*StartRenderMethodArgDelete)(void *);
  void *StartRenderMethodArg;
  void (*EndRenderMethod)(void *);
  void (*EndRenderMethodArgDelete)(void *);
  void *EndRenderMethodArg;
};

// Description:
// Get the list of lights for this renderer.
inline vtkLightCollection *vtkRenderer::GetLights() {return &(this->Lights);};

// Description:
// Get the list of actors for this renderer.
inline vtkActorCollection *vtkRenderer::GetActors() {return &(this->Actors);};

// Description:
// Convert display (or screen) coordinates to world coordinates.
inline void vtkRenderer::DisplayToWorld() {DisplayToView(); ViewToWorld();};

// Description:
// Convert world point coordinates to display (or screen) coordinates.
inline void vtkRenderer::WorldToDisplay() {WorldToView(); ViewToDisplay();};

#endif
