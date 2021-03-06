/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkSurfaceSource.h"
#include "mitkSurface.h"

mitk::SurfaceSource::SurfaceSource()
{
  // Create the output. We use static_cast<> here because we know the default
  // output must be of type TOutputImage
  itk::DataObject::Pointer output = this->MakeOutput(0);

  Superclass::SetNumberOfRequiredOutputs(1);
  Superclass::SetNthOutput(0, output);
}

mitk::SurfaceSource::~SurfaceSource()
{
}

itk::DataObject::Pointer mitk::SurfaceSource::MakeOutput(DataObjectPointerArraySizeType /*idx*/)
{
  return OutputType::New().GetPointer();
}

itk::DataObject::Pointer mitk::SurfaceSource::MakeOutput(const DataObjectIdentifierType &name)
{
  itkDebugMacro("MakeOutput(" << name << ")");
  if (this->IsIndexedOutputName(name))
  {
    return this->MakeOutput(this->MakeIndexFromOutputName(name));
  }
  return static_cast<itk::DataObject *>(mitk::Surface::New().GetPointer());
}

mitkBaseDataSourceGetOutputDefinitions(mitk::SurfaceSource)
