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
#include "Axes.hh"
#include "FScalars.hh"
#include "FNormals.hh"

// Description:
// Construct with origin=(0,0,0) and scale factor=1.
vtkAxes::vtkAxes()
{
  this->Origin[0] = 0.0;  
  this->Origin[1] = 0.0;  
  this->Origin[2] = 0.0;

  this->ScaleFactor = 1.0;
}

void vtkAxes::Execute()
{
  int numPts=6, numLines=3;
  vtkFloatPoints *newPts;
  vtkCellArray *newLines;
  vtkFloatScalars *newScalars;
  vtkFloatNormals *newNormals;
  float x[3], n[3];
  int ptIds[2];

  this->Initialize();

  newPts = new vtkFloatPoints(numPts);
  newLines = new vtkCellArray();
  newLines->Allocate(newLines->EstimateSize(numLines,2));
  newScalars = new vtkFloatScalars(numPts);
  newNormals = new vtkFloatNormals(numPts);
//
// Create axes
//
  n[0] = 0.0; n[1] = 1.0; n[2] = 0.0; 
  ptIds[0] = newPts->InsertNextPoint(this->Origin);
  newScalars->InsertNextScalar(0.0);
  newNormals->InsertNextNormal(n);

  x[0] = this->Origin[0] + this->ScaleFactor;
  x[1] = this->Origin[1];
  x[2] = this->Origin[2];
  ptIds[1] = newPts->InsertNextPoint(x);
  newLines->InsertNextCell(2,ptIds);
  newScalars->InsertNextScalar(0.0);
  newNormals->InsertNextNormal(n);


  n[0] = 0.0; n[1] = 0.0; n[2] = 1.0; 
  ptIds[0] = newPts->InsertNextPoint(this->Origin);
  newScalars->InsertNextScalar(0.25);
  newNormals->InsertNextNormal(n);

  x[0] = this->Origin[0];
  x[1] = this->Origin[1] + this->ScaleFactor;
  x[2] = this->Origin[2];
  ptIds[1] = newPts->InsertNextPoint(x);
  newScalars->InsertNextScalar(0.25);
  newNormals->InsertNextNormal(n);
  newLines->InsertNextCell(2,ptIds);


  n[0] = 1.0; n[1] = 0.0; n[2] = 0.0; 
  ptIds[0] = newPts->InsertNextPoint(this->Origin);
  newScalars->InsertNextScalar(0.5);
  newNormals->InsertNextNormal(n);

  x[0] = this->Origin[0];
  x[1] = this->Origin[1];
  x[2] = this->Origin[2] + this->ScaleFactor;
  ptIds[1] = newPts->InsertNextPoint(x);
  newScalars->InsertNextScalar(0.5);
  newNormals->InsertNextNormal(n);
  newLines->InsertNextCell(2,ptIds);
//
// Update self and release memory
// 
  this->SetPoints(newPts);
  newPts->Delete();

  this->PointData.SetScalars(newScalars);
  newScalars->Delete();

  this->PointData.SetNormals(newNormals);
  newNormals->Delete();

  this->SetLines(newLines);
  newLines->Delete();
}

void vtkAxes::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkPolySource::PrintSelf(os,indent);

  os << indent << "Origin: (" << this->Origin[0] << ", "
               << this->Origin[1] << ", "
               << this->Origin[2] << ")\n";

  os << indent << "Scale Factor: " << this->ScaleFactor << "\n";
}
