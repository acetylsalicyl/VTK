/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$
  Thanks:    Thanks to C. Charles Law who developed this class.

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
#include "vtkImageCache.h"
#include "vtkImageMirrorPad.h"



//----------------------------------------------------------------------------
// Just clip the request.
void vtkImageMirrorPad::ComputeRequiredInputUpdateExtent()
{
  int idx;
  int extent[6];
  int min, max, imageMin, imageMax;
  int minCycle, maxCycle, minRemainder, maxRemainder, minMirror, maxMirror;
  int *wholeExtent;
  
  this->Output->GetUpdateExtent(extent);
  wholeExtent = this->Input->GetWholeExtent();

  // determine input extent
  for (idx = 0; idx < 3; ++idx)
    {
    min = extent[idx * 2];
    max = extent[idx*2 + 1];
    imageMin = wholeExtent[idx * 2];
    imageMax = wholeExtent[idx * 2 + 1];
    // Relative to imageMin;
    min -= imageMin;
    max -= imageMin;
    // compute which cycle of the mirror the extent is in.
    // Negatives are a real pain.
    minCycle = (min >= 0) ? (min / (imageMax - imageMin + 1)) :
      ((1+min) / (imageMax - imageMin + 1)) - 1;
    maxCycle = (max >= 0) ? (max / (imageMax - imageMin + 1)) :
      ((1+max) / (imageMax - imageMin + 1)) - 1;
    // Determine if we are in the mirror image state
    // Odd => 1, even => 0 (Handle negatives explicitely)
    minMirror = (minCycle > 0) ? minCycle % 2 : -minCycle % 2; 
    maxMirror = (maxCycle > 0) ? maxCycle % 2 : -maxCycle % 2; 
    // compute the position in the cycle. (remainder)
    minRemainder = min - (imageMax - imageMin + 1) * minCycle;
    if (minMirror)
      {
      minRemainder = (imageMax - imageMin - minRemainder);
      }
    maxRemainder = max - (imageMax - imageMin + 1) * maxCycle;
    if (maxMirror)
      {
      maxRemainder = (imageMax - imageMin - maxRemainder);
      }
    // convert back relative to origin.
    minRemainder += imageMin;
    maxRemainder += imageMin;
    
    if (minCycle == maxCycle)
      {
      if (minMirror)
	{
	min = maxRemainder;
	max = minRemainder;
	}
      else
	{
	min = minRemainder;
	max = maxRemainder;
	}
      }
    else if (minCycle < maxCycle-1)
      {
      min = imageMin;
      max = imageMax;
      }
    else 
      {
      if (minMirror)
	{
	min = imageMin;
	max = (minRemainder > maxRemainder) ? minRemainder : maxRemainder;
	}
      else
	{
	max = imageMax;
	min = (minRemainder < maxRemainder) ? minRemainder : maxRemainder;
	}
      }
    extent[idx * 2] = min;
    extent[idx * 2 + 1] = max;
    }
  
  this->Input->SetUpdateExtent(extent);
}




//----------------------------------------------------------------------------
// Description:
// This templated function executes the filter for any type of data.
// First the corresponding input pointer is found.
// Mirror is generated by negating increments when the inIdxs loop.
// But inIdxs are always increasing. It is a bit messy.
template <class T>
static void vtkImageMirrorPadExecute(vtkImageData *inData,
				     vtkImageData *outData, T *outPtr,
				     int outExt[6])
{
  int min0, max0, min1, max1, min2, max2;
  int imageMin0, imageMax0, imageMin1, imageMax1, 
    imageMin2, imageMax2;
  int outIdx0, outIdx1, outIdx2;
  int inIdx0, inIdx1, inIdx2;
  int start0, start1, start2;
  int inInc0, inInc1, inInc2; // increment at the start of the row
  int mInc0, mInc1, mInc2; // mirror increment toggles +-+-
  int outInc0, outInc1, outInc2;
  T *inPtr0, *inPtr1, *inPtr2;
  T *outPtr0, *outPtr1, *outPtr2;
  int cycle, mirror;

  // Get information to march through data 
  inData->GetIncrements(inInc0, inInc1, inInc2);
  inData->GetExtent(imageMin0, imageMax0, imageMin1, imageMax1, 
		    imageMin2, imageMax2);
  outData->GetIncrements(outInc0, outInc1, outInc2);
  min0 = outExt[0];
  max0 = outExt[1];
  min1 = outExt[2];
  max1 = outExt[3];
  min2 = outExt[4];
  max2 = outExt[5];

  // initialize pointers to coresponding pixels.
  // axis0
  cycle = (min0 >= imageMin0) ? ((min0-imageMin0) / (imageMax0-imageMin0+1)) :
    ((1+min0-imageMin0) / (imageMax0-imageMin0+1)) - 1;
  mirror = (cycle > 0) ? cycle % 2 : -cycle % 2; 
  inIdx0 = start0 = (min0) - (imageMax0 - imageMin0 + 1) * cycle;
  if (mirror)
    {
    inIdx0 = (imageMax0 - imageMin0 - inIdx0);
    inInc0 = -inInc0;
    }
  start0 += imageMin0;
  // axis1
  cycle = (min1 >= imageMin1) ? ((min1-imageMin1) / (imageMax1-imageMin1+1)) :
    ((1+min1-imageMin1) / (imageMax1-imageMin1+1)) - 1;
  mirror = (cycle > 0) ? cycle % 2 : -cycle % 2; 
  inIdx1 = start1 = (min1) - (imageMax1 - imageMin1 + 1) * cycle;
  if (mirror)
    {
    inIdx1 = (imageMax1 - imageMin1 - inIdx1);
    inInc1 = -inInc1;
    }
  start1 += imageMin1;
  // axis2
  cycle = (min2 >= imageMin2) ? ((min2-imageMin2) / (imageMax2-imageMin2+1)) :
    ((1+min2-imageMin2) / (imageMax2-imageMin2+1)) - 1;
  mirror = (cycle > 0) ? cycle % 2 : -cycle % 2; 
  inIdx2 = start2 = (min2) - (imageMax2 - imageMin2 + 1) * cycle;
  if (mirror)
    {
    inIdx2 = (imageMax2 - imageMin2 - inIdx2);
    inInc2 = -inInc2;
    }
  start2 += imageMin2;

  // initialize pointers
  inPtr2 = (T *)(inData->GetScalarPointer(inIdx0, inIdx1, inIdx2));
  outPtr2 = outPtr; // (min0, min1, min2)
  
  // Loop through ouput pixels
  inIdx2 = start2;
  mInc2 = inInc2;
  for (outIdx2 = min2; outIdx2 <= max2; ++outIdx2)
    {
    outPtr1 = outPtr2;
    inPtr1 = inPtr2;
    inIdx1 = start1;
    mInc1 = inInc1;
    for (outIdx1 = min1; outIdx1 <= max1; ++outIdx1)
      {
      outPtr0 = outPtr1;
      inPtr0 = inPtr1;
      inIdx0 = start0;
      mInc0 = inInc0;
      for (outIdx0 = min0; outIdx0 <= max0; ++outIdx0)
	{
	// Copy Pixel
	*outPtr0 = *inPtr0;
	
	outPtr0 += outInc0;
	if (inIdx0 == imageMax0)
	  {
	  inIdx0 = imageMin0;
	  mInc0 = -mInc0;
	  }
	else
	  {
	  ++inIdx0;
	  inPtr0 += mInc0;
	  }
	}
      outPtr1 += outInc1;
      if (inIdx1 == imageMax1)
	{
	inIdx1 = imageMin1;
	mInc1 = -mInc1;
	}
      else
	{
	++inIdx1;
	inPtr1 += mInc1;
	}
      }
    outPtr2 += outInc2;
    if (inIdx2 == imageMax2)
      {
      inIdx2 = imageMin2;
      mInc2 = -mInc2;
      }
    else
      {
      ++inIdx2;
      inPtr2 += mInc2;
      }
    }
  
}



//----------------------------------------------------------------------------
// Description:
// This method is passed a input and output data, and executes the filter
// algorithm to fill the output from the input.
// It just executes a switch statement to call the correct function for
// the regions data types.
void vtkImageMirrorPad::ThreadedExecute(vtkImageData *inData, 
					 vtkImageData *outData,
					 int outExt[6])
{
  void *outPtr = outData->GetScalarPointerForExtent(outExt);
  
  vtkDebugMacro(<< "Execute: inData = " << inData 
		<< ", outData = " << outData);
  
  // this filter expects that input is the same type as output.
  if (inData->GetScalarType() != outData->GetScalarType())
    {
    vtkErrorMacro(<< "Execute: input ScalarType, " << inData->GetScalarType()
                  << ", must match out ScalarType " << outData->GetScalarType());
    return;
    }
  
  switch (inData->GetScalarType())
    {
    case VTK_FLOAT:
      vtkImageMirrorPadExecute(inData, outData, (float *)(outPtr), outExt);
      break;
    case VTK_INT:
      vtkImageMirrorPadExecute(inData, outData, (int *)(outPtr), outExt);
      break;
    case VTK_SHORT:
      vtkImageMirrorPadExecute(inData, outData, (short *)(outPtr), outExt);
      break;
    case VTK_UNSIGNED_SHORT:
      vtkImageMirrorPadExecute(inData, outData, 
			       (unsigned short *)(outPtr), outExt);
      break;
    case VTK_UNSIGNED_CHAR:
      vtkImageMirrorPadExecute(inData, outData, 
			       (unsigned char *)(outPtr), outExt);
      break;
    default:
      vtkErrorMacro(<< "Execute: Unknown ScalarType");
      return;
    }
}














