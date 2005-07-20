/*=========================================================================

  Program:   ParaView
  Module:    vtkPVIceTDesktopRenderModuleUI.cxx

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkPVIceTDesktopRenderModuleUI.h"
#include "vtkObjectFactory.h"
#include "vtkKWLabel.h"
#include "vtkKWCheckButton.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWScale.h"
#include "vtkPVApplication.h"
#include "vtkTimerLog.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkPVIceTDesktopRenderModuleUI);
vtkCxxRevisionMacro(vtkPVIceTDesktopRenderModuleUI, "1.6");

//----------------------------------------------------------------------------
vtkPVIceTDesktopRenderModuleUI::vtkPVIceTDesktopRenderModuleUI()
{
}


//----------------------------------------------------------------------------
vtkPVIceTDesktopRenderModuleUI::~vtkPVIceTDesktopRenderModuleUI()
{
}

//----------------------------------------------------------------------------
void vtkPVIceTDesktopRenderModuleUI::Create(vtkKWApplication *app)
{
  // Skip over LOD res and threshold.
  
  if (this->IsCreated())
    {
    vtkErrorMacro("RenderModuleUI already created");
    return;
    }

  this->Superclass::Create(app);

  this->Script("pack forget %s",
               this->ParallelRenderParametersFrame->GetWidgetName());
  //this->CompositeCompressionCheck->EnabledOff();

  //this->SquirtCheck->SetSelectedState(0);
  //this->SquirtLabel->EnabledOff();
  //this->SquirtCheck->EnabledOff();
  //this->SquirtLevelScale->EnabledOff();
  //this->SquirtLevelLabel->EnabledOff();

  //this->SetReductionFactor(1);
  //this->ReductionCheck->EnabledOff();
  //this->ReductionLabel->EnabledOff();
}

//----------------------------------------------------------------------------
void vtkPVIceTDesktopRenderModuleUI::EnableRenductionFactor()
{
  this->SetReductionFactor(2);
  this->ReductionCheck->EnabledOn();
  this->ReductionLabel->EnabledOn();
}



//----------------------------------------------------------------------------
void vtkPVIceTDesktopRenderModuleUI::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

