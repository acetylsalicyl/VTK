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
#include "Streamer.hh"
#include "vtkMath.hh"

vtkStreamArray::vtkStreamArray()
{
  this->MaxId = -1; 
  this->Array = new vtkStreamPoint[1000];
  this->Size = 1000;
  this->Extend = 5000;
  this->Direction = INTEGRATE_FORWARD;
}

vtkStreamPoint *vtkStreamArray::Resize(int sz)
{
  vtkStreamPoint *newArray;
  int newSize;

  if (sz >= this->Size) newSize = this->Size + 
    this->Extend*(((sz-this->Size)/this->Extend)+1);
  else newSize = sz;

  newArray = new vtkStreamPoint[newSize];

  memcpy(newArray, this->Array,
         (sz < this->Size ? sz : this->Size) * sizeof(vtkStreamPoint));

  this->Size = newSize;
  delete [] this->Array;
  this->Array = newArray;

  return this->Array;
}


vtkStreamer::vtkStreamer()
{
  this->StartFrom = START_FROM_POSITION;

  this->StartCell = 0;
  this->StartSubId = 0;
  this->StartPCoords[0] = this->StartPCoords[1] = this->StartPCoords[2] = 0.5;
  this->StartPosition[0] = this->StartPosition[1] = this->StartPosition[2] = 0.0;
  this->Source = NULL;
  this->Streamers = NULL;
  this->MaximumPropagationTime = 100.0;
  this->IntegrationDirection = INTEGRATE_FORWARD;
  this->IntegrationStepLength = 0.2;
  this->Vorticity = 0;
  this->TerminalSpeed = 0.0;
  this->SpeedScalars = 0;
}

// Description:
// Specify the start of the streamline in the cell coordinate system. That is,
// cellId and subId (if composite cell), and parametric coordinates.
void vtkStreamer::SetStartLocation(int cellId, int subId, float pcoords[3])
{
  if ( cellId != this->StartCell || subId != this->StartSubId ||
  pcoords[0] !=  this->StartPCoords[0] || 
  pcoords[1] !=  this->StartPCoords[1] || 
  pcoords[2] !=  this->StartPCoords[2] )
    {
    this->Modified();
    this->StartFrom = START_FROM_LOCATION;

    this->StartCell = cellId;
    this->StartSubId = subId;
    this->StartPCoords[0] = pcoords[0];
    this->StartPCoords[1] = pcoords[1];
    this->StartPCoords[2] = pcoords[2];
    }
}

// Description:
// Specify the start of the streamline in the cell coordinate system. That is,
// cellId and subId (if composite cell), and parametric coordinates.
void vtkStreamer::SetStartLocation(int cellId, int subId, float r, float s, float t)
{
  float pcoords[3];
  pcoords[0] = r;
  pcoords[1] = s;
  pcoords[2] = t;

  this->SetStartLocation(cellId, subId, pcoords);
}

// Description:
// Get the starting location of the streamline in the cell corrdinate system.
int vtkStreamer::GetStartLocation(int& subId, float pcoords[3])
{
  subId = this->StartSubId;
  pcoords[0] = this->StartPCoords[0];
  pcoords[1] = this->StartPCoords[1];
  pcoords[2] = this->StartPCoords[2];
  return this->StartCell;
}

// Description:
// Specify the start of the streamline in the global coordinate system. Search
// must be performed to find initial cell to strart integration from.
void vtkStreamer::SetStartPosition(float x[3])
{
  if ( x[0] != this->StartPosition[0] || x[1] != this->StartPosition[1] || 
  x[2] != this->StartPosition[2] )
    {
    this->Modified();
    this->StartFrom = START_FROM_POSITION;

    this->StartPosition[0] = x[0];
    this->StartPosition[1] = x[1];
    this->StartPosition[2] = x[2];
    }
}

// Description:
// Specify the start of the streamline in the global coordinate system. Search
// must be performed to find initial cell to strart integration from.
void vtkStreamer::SetStartPosition(float x, float y, float z)
{
  float pos[3];
  pos[0] = x;
  pos[1] = y;
  pos[2] = z;

  this->SetStartPosition(pos);
}

// Description:
// Get the start position in global x-y-z coordinates.
float *vtkStreamer::GetStartPosition()
{
  return this->StartPosition;
}

// Description:
// Override update method because execution can branch two ways (Input 
// and Source)
void vtkStreamer::Update()
{
  // make sure input is available
  if ( this->Input == NULL )
    {
    vtkErrorMacro(<< "No input!");
    return;
    }

  // prevent chasing our tail
  if (this->Updating) return;

  this->Updating = 1;
  this->Input->Update();
  if ( this->Source ) this->Source->Update();
  this->Updating = 0;

  if (this->Input->GetMTime() > this->GetMTime() || 
  (this->Source && this->Source->GetMTime() > this->GetMTime()) || 
  this->GetMTime() > this->ExecuteTime || this->GetDataReleased() )
    {
    if ( this->StartMethod ) (*this->StartMethod)(this->StartMethodArg);
    this->Execute();
    this->ExecuteTime.Modified();
    this->SetDataReleased(0);
    if ( this->EndMethod ) (*this->EndMethod)(this->EndMethodArg);
    }

  if ( this->Input->ShouldIReleaseData() ) this->Input->ReleaseData();
  if ( this->Source && this->Source->ShouldIReleaseData() ) 
    this->Source->ReleaseData();
}

void vtkStreamer::Integrate()
{
  vtkDataSet *input=this->Input;
  vtkDataSet *source=this->Source;
  vtkPointData *pd=input->GetPointData();
  vtkScalars *inScalars;
  vtkVectors *inVectors;
  int numSourcePts;
  vtkStreamPoint *sNext, *sPtr;
  int i, j, ptId, offset, numSteps, subId;
  vtkCell *cell;
  vtkFloatVectors cellVectors(MAX_CELL_SIZE);
  vtkFloatScalars cellScalars(MAX_CELL_SIZE);
  float *v, x[3], xNext[3];
  vtkMath math;
  float d, step, dir, vNext[3], tol2, p[3];
  float w[MAX_CELL_SIZE], dist2;
  float closestPoint[3], stepLength;
  
  vtkDebugMacro(<<"Generating streamers");
  this->Initialize();
  this->NumberOfStreamers = 0;

  if ( ! (inVectors=pd->GetVectors()) )
    {
    vtkErrorMacro(<<"No vector data defined!");
    return;
    }

  inScalars = pd->GetScalars();
  tol2 = input->GetLength()/1000; tol2 = tol2*tol2;
//
// Create starting points
//
  this->NumberOfStreamers = numSourcePts = offset = 1;
  if ( this->Source )
    this->NumberOfStreamers = numSourcePts = source->GetNumberOfPoints();
 
  if ( this->IntegrationDirection == INTEGRATE_BOTH_DIRECTIONS )
    {
    offset = 2;
    this->NumberOfStreamers *= 2;
    }

  this->Streamers = new vtkStreamArray[this->NumberOfStreamers];

  if ( this->StartFrom == START_FROM_POSITION && !this->Source )
    {
    sPtr = this->Streamers[0].InsertNextStreamPoint();
    for (i=0; i<3; i++) sPtr->x[i] = this->StartPosition[i];
    sPtr->cellId = input->FindCell(this->StartPosition, NULL, 0.0, 
                                   sPtr->subId, sPtr->p, w);
    }

  else if ( this->StartFrom == START_FROM_LOCATION && !this->Source )
    {
    sPtr = this->Streamers[0].InsertNextStreamPoint();
    cell =  input->GetCell(sPtr->cellId);
    cell->EvaluateLocation(sPtr->subId, sPtr->p, sPtr->x, w);
    }

  else //START_FROM_SOURCE
    {
    for (ptId=0; ptId < numSourcePts; ptId++)
      {
      sPtr = this->Streamers[offset*ptId].InsertNextStreamPoint();
      source->GetPoint(ptId,sPtr->x);
      sPtr->cellId = input->FindCell(sPtr->x, NULL, tol2,
                                     sPtr->subId, sPtr->p, w);
      }
    }
//
// Finish initializing each streamer
//
  for (ptId=0; ptId < numSourcePts; ptId++)
    {
    this->Streamers[offset*ptId].Direction = 1.0;
    sPtr = this->Streamers[offset*ptId].GetStreamPoint(0);
    sPtr->d = 0.0;
    sPtr->t = 0.0;
    if ( sPtr->cellId >= 0 ) //starting point in dataset
      {
      cell = input->GetCell(sPtr->cellId);
      cell->EvaluateLocation(sPtr->subId, sPtr->p, xNext, w);

      inVectors->GetVectors(cell->PointIds,cellVectors);
      sPtr->v[0]  = sPtr->v[1] = sPtr->v[2] = 0.0;
      for (i=0; i < cell->GetNumberOfPoints(); i++)
        {
        v =  cellVectors.GetVector(i);
        for (j=0; j<3; j++) sPtr->v[j] += v[j] * w[i];
        }
      sPtr->speed = math.Norm(sPtr->v);

      if ( inScalars ) 
        {
        inScalars->GetScalars(cell->PointIds,cellScalars);
        for (sPtr->s=0, i=0; i < cell->GetNumberOfPoints(); i++)
          sPtr->s += cellScalars.GetScalar(i) * w[i];
        }
      }

    if ( this->IntegrationDirection == INTEGRATE_BOTH_DIRECTIONS )
      {
      this->Streamers[offset*ptId+1].Direction = -1.0;
      sNext = this->Streamers[offset*ptId+1].InsertNextStreamPoint();
      *sNext = *sPtr;
      }
    } //for each streamer
//
// For each streamer, integrate in appropriate direction (using RK2)
//
  for (ptId=0; ptId < this->NumberOfStreamers; ptId++)
    {
    //get starting step
    sPtr = this->Streamers[ptId].GetStreamPoint(0);
    if ( sPtr->cellId < 0 ) continue;

    dir = this->Streamers[ptId].Direction;
    cell = input->GetCell(sPtr->cellId);
    cell->EvaluateLocation(sPtr->subId, sPtr->p, xNext, w);
    step = this->IntegrationStepLength * sqrt((double)cell->GetLength2());
    inVectors->GetVectors(cell->PointIds,cellVectors);
    if ( inScalars ) inScalars->GetScalars(cell->PointIds,cellScalars);

    //integrate until time has been exceeded
    while ( sPtr->cellId >= 0 && sPtr->speed > this->TerminalSpeed &&
    sPtr->t < this->MaximumPropagationTime )
      {

      //compute updated position using this step (Euler integration)
      //use normalized velocity vector (to keep integration in cell)
      for (i=0; i<3; i++)
        xNext[i] = sPtr->x[i] + dir * step * sPtr->v[i] / sPtr->speed;

      //compute updated position using updated step
      cell->EvaluatePosition(xNext, closestPoint, subId, p, dist2, w);

      //interpolate velocity
      vNext[0] = vNext[1] = vNext[2] = 0.0;
      for (i=0; i < cell->GetNumberOfPoints(); i++)
        {
        v = cellVectors.GetVector(i);
        for (j=0; j < 3; j++) vNext[j] += v[j] * w[i];
        }

      //now compute final position
      for (i=0; i<3; i++)
        xNext[i] = sPtr->x[i] +   
                   dir * (step/2.0) * (sPtr->v[i] + vNext[i]) / sPtr->speed;

      sNext = this->Streamers[ptId].InsertNextStreamPoint();

      if ( cell->EvaluatePosition(xNext, closestPoint, sNext->subId, 
      sNext->p, dist2, w) == 1)
        { //integration still in cell
        for (i=0; i<3; i++) sNext->x[i] = closestPoint[i];
        sNext->cellId = sPtr->cellId;
        sNext->subId = sPtr->subId;
        }
      else
        { //integration has passed out of cell
        sNext->cellId = input->FindCell(xNext, cell, tol2, 
                                        sNext->subId, sNext->p, w);
        if ( sNext->cellId >= 0 ) //make sure not out of dataset
          {
          for (i=0; i<3; i++) sNext->x[i] = xNext[i];
          cell = input->GetCell(sNext->cellId);
          inVectors->GetVectors(cell->PointIds,cellVectors);
          if ( inScalars ) inScalars->GetScalars(cell->PointIds,cellScalars);
          step = this->IntegrationStepLength * sqrt((double)cell->GetLength2());
          }
        }

      if ( sNext->cellId >= 0 )
        {
        cell->EvaluateLocation(sNext->subId, sNext->p, xNext, w);
        sNext->v[0] = sNext->v[1] = sNext->v[2] = 0.0;
        for (i=0; i < cell->GetNumberOfPoints(); i++)
          {
          v = cellVectors.GetVector(i);
          for (j=0; j < 3; j++) sNext->v[j] += v[j] * w[i];
          }
        sNext->speed = math.Norm(sNext->v);
        if ( inScalars )
          for (sNext->s=0.0, i=0; i < cell->GetNumberOfPoints(); i++)
            sNext->s += cellScalars.GetScalar(i) * w[i];

        d = sqrt((double)math.Distance2BetweenPoints(sPtr->x,sNext->x));
        sNext->d = sPtr->d + d;
        sNext->t = sPtr->t + (2.0 * d / (sPtr->speed + sNext->speed));
        }

      sPtr = sNext;

      }//for elapsed time

    } //for each streamer
//
// Compute vorticity if desired.
//
  if ( this->Vorticity ) this->ComputeVorticity();
//
// Now create appropriate representation
//
  if ( this->SpeedScalars )
    {
    for (ptId=0; ptId < this->NumberOfStreamers; ptId++)
      {
      for ( sPtr=this->Streamers[ptId].GetStreamPoint(0), i=0; 
      i < this->Streamers[ptId].GetNumberOfPoints() && sPtr->cellId >= 0; 
      i++, sPtr=this->Streamers[ptId].GetStreamPoint(i) )
        {
        sPtr->s = sPtr->speed;
        }
      }
    }
}

void vtkStreamer::ComputeVorticity()
{
}

void vtkStreamer::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkDataSetToPolyFilter::PrintSelf(os,indent);

  if ( this->StartFrom == START_FROM_POSITION && !this->Source)
    {
    os << indent << "Starting Position: (" << this->StartPosition[0] << ","
       << this->StartPosition[1] << ", " << this->StartPosition[2] << ")\n";
    }
  else if ( this->StartFrom == START_FROM_LOCATION && !this->Source)
    {
    os << indent << "Starting Location:\n\tCell: " << this->StartCell 
       << "\n\tSubId: " << this->StartSubId << "\n\tP.Coordinates: ("
       << this->StartPCoords[0] << ", " 
       << this->StartPCoords[1] << ", " 
       << this->StartPCoords[2] << ")\n";
    }
  else
    {
    os << indent << "Starting Source: " << (void *)this->Source << "\n";
    }

  os << indent << "Maximum Propagation Time: " 
     << this->MaximumPropagationTime << "\n";

  if ( this->IntegrationStepLength == INTEGRATE_FORWARD )
    os << indent << "Integration Direction: FORWARD\n";
  else if ( this->IntegrationStepLength == INTEGRATE_BACKWARD )
    os << indent << "Integration Direction: BACKWARD\n";
  else
    os << indent << "Integration Direction: FORWARD & BACKWARD\n";

  os << indent << "Integration Step Length: " << this->IntegrationStepLength << "\n";

  os << indent << "Vorticity: " << (this->Vorticity ? "On\n" : "Off\n");

  os << indent << "Terminal Speed: " << this->TerminalSpeed << "\n";
}


