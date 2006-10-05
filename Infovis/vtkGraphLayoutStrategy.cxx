/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/*----------------------------------------------------------------------------
 Copyright (c) Sandia Corporation
 See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.
----------------------------------------------------------------------------*/
#include "vtkGraphLayoutStrategy.h"

#include "vtkAbstractGraph.h"

vtkCxxRevisionMacro(vtkGraphLayoutStrategy, "$Revision$");

void vtkGraphLayoutStrategy::SetGraph(vtkAbstractGraph *graph)
{
  // This method is a cut and paste of vtkCxxSetObjectMacro
  // except for the call to Initialize in the middle :)
  if (graph != this->Graph)
    {
    vtkAbstractGraph *tmp = this->Graph;
    this->Graph = graph;
    if (this->Graph != NULL)
      {
      this->Graph->Register(this);
      this->Initialize();
      }
    if (tmp != NULL)
      {
      tmp->UnRegister(this);
      }
    this->Modified();
    }
}

vtkGraphLayoutStrategy::vtkGraphLayoutStrategy()
{
  this->Graph = NULL;
}

vtkGraphLayoutStrategy::~vtkGraphLayoutStrategy()
{
  // Unregister vtk objects that were passed in
  this->SetGraph(NULL);
}

void vtkGraphLayoutStrategy::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
