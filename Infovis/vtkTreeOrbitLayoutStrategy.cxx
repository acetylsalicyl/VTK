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

#include "vtkTreeOrbitLayoutStrategy.h"

#include "vtkAbstractArray.h"
#include "vtkAdjacentVertexIterator.h"
#ifdef VTK_USE_BOOST
#include "vtkBoostBreadthFirstSearchTree.h"
#endif
#include "vtkDataArray.h"
#include "vtkIdTypeArray.h"
#include "vtkIntArray.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkSmartPointer.h"
#include "vtkTree.h"
#include "vtkTreeDFSIterator.h"

vtkCxxRevisionMacro(vtkTreeOrbitLayoutStrategy, "$Revision$");
vtkStandardNewMacro(vtkTreeOrbitLayoutStrategy);

vtkTreeOrbitLayoutStrategy::vtkTreeOrbitLayoutStrategy()
{
  this->LogSpacingValue = 1.0;
  this->LeafSpacing = 0.9;
}

vtkTreeOrbitLayoutStrategy::~vtkTreeOrbitLayoutStrategy()
{

}

// Helper method for recursively orbiting children
// around their parents
void vtkTreeOrbitLayoutStrategy::OrbitChildren(vtkTree *t,
  vtkPoints *p, vtkIdType parent, double radius)
{

  // Get the current position of the parent
  double pt[3];
  double xCenter,yCenter;
  p->GetPoint(parent, pt);
  xCenter = pt[0];
  yCenter = pt[1];
  
  // Check for leaf_count array
  vtkIntArray* leaf_count = vtkIntArray::SafeDownCast(
                            t->GetVertexData()->GetArray("leaf_count"));
  if (!leaf_count)
    {
    vtkErrorMacro("vtkTreeOrbitLayoutStrategy has to have a leaf_count array");
    exit(1);
    }
  
  // Get the total number of children for this node
//  vtkIdType totalChildren = leaf_count->GetValue(parent);
  vtkIdType immediateChildren = t->GetNumberOfChildren(parent);
    
  // Now simply orbit the children around the
  // parent's centerpoint
  vtkIdType subChildrenCount = 0;
  for (vtkIdType i=0; i < immediateChildren; ++i)
    {
    vtkIdType childID = t->GetChild(parent, i); 
    vtkIdType subChildren = leaf_count->GetValue(childID);
    
    double angle = i/static_cast<double>(immediateChildren);
    //double angle = subChildrenCount/static_cast<double>(totalChildren);
    double x = cos(2.0*vtkMath::Pi()*angle);
    double y = sin(2.0*vtkMath::Pi()*angle);

    float radiusFactor = subChildren+1;
    double xOrbit = x * radius * radiusFactor + xCenter;
    double yOrbit = y * radius * radiusFactor + yCenter;
    p->SetPoint(childID, xOrbit, yOrbit, 0);
    
    // Now recurse with a reduced radius
    this->OrbitChildren(t,p,childID, radius*radiusFactor/4.0);
    
    // Accumulate subchildren
    subChildrenCount += subChildren;
    }
}

// Tree layout method
void vtkTreeOrbitLayoutStrategy::Layout()
{
  vtkTree* tree = vtkTree::SafeDownCast(this->Graph);
  if (tree == NULL)
    {
#ifdef VTK_USE_BOOST
    // Use the BFS search tree to perform the layout
    vtkBoostBreadthFirstSearchTree* bfs = vtkBoostBreadthFirstSearchTree::New();
    bfs->CreateGraphVertexIdArrayOn();
    bfs->SetInput(this->Graph);
    bfs->Update();
    tree = vtkTree::New();
    tree->ShallowCopy(bfs->GetOutput());
    bfs->Delete();
#else
    vtkErrorMacro("Layout only works on vtkTree unless VTK_USE_BOOST is on.");
#endif
    }
  
  // Create a new point set
  vtkIdType numVertices = tree->GetNumberOfVertices();
  vtkPoints *newPoints = vtkPoints::New();
  newPoints->SetNumberOfPoints(numVertices);
  
  // Setting the root to position 0,0 but this could
  // be whatever you want and should be controllable
  // through ivars in the future
  vtkIdType currentRoot = tree->GetRoot();
  newPoints->SetPoint(currentRoot, 0, 0, 0);
  
  // Now traverse the tree and have all children 
  // orbit their parents recursively
  double radius=1;
  this->OrbitChildren(tree, newPoints, currentRoot, radius);
  
  // Copy coordinates back into the original graph
  if (vtkTree::SafeDownCast(this->Graph))
    {
    this->Graph->SetPoints(newPoints);
    }
#ifdef VTK_USE_BOOST
  else
    {
    // Reorder the points based on the mapping back to graph vertex ids
    vtkPoints* reordered = vtkPoints::New();
    reordered->SetNumberOfPoints(newPoints->GetNumberOfPoints());
    for (vtkIdType i = 0; i < reordered->GetNumberOfPoints(); i++)
      {
      reordered->SetPoint(i, 0, 0, 0);
      }
    vtkIdTypeArray* graphVertexIdArr = vtkIdTypeArray::SafeDownCast(
      tree->GetVertexData()->GetAbstractArray("GraphVertexId"));
    for (vtkIdType i = 0; i < graphVertexIdArr->GetNumberOfTuples(); i++)
      {
      reordered->SetPoint(graphVertexIdArr->GetValue(i), newPoints->GetPoint(i));
      }
    this->Graph->SetPoints(reordered);
    tree->Delete();
    reordered->Delete();
    }
#endif

  // Clean up.
  newPoints->Delete();
} 

void vtkTreeOrbitLayoutStrategy::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "LogSpacingValue: " << this->LogSpacingValue << endl;
  os << indent << "LeafSpacing: " << this->LeafSpacing << endl;
}
