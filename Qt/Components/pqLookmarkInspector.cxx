/*=========================================================================

   Program: ParaView
   Module:    pqLookmarkInspector.cxx

   Copyright (c) 2005,2006 Sandia Corporation, Kitware Inc.
   All rights reserved.

   ParaView is a free software; you can redistribute it and/or modify it
   under the terms of the ParaView license version 1.1. 

   See License_v1.1.txt for the full ParaView license.
   A copy of this license can be obtained by contacting
   Kitware Inc.
   28 Corporate Drive
   Clifton Park, NY 12065
   USA

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/

#include "pqLookmarkInspector.h"
#include "ui_pqLookmarkInspector.h"

// Qt includes
#include <QVBoxLayout>
#include <QScrollArea>
#include <QPushButton>
#include <QTabWidget>
#include <QApplication>
#include <QStyle>
#include <QStyleOption>

// vtk includes
#include "QVTKWidget.h"

// ParaView Server Manager includes
#include <vtkSMProxy.h>

// ParaView includes
#include "pqApplicationCore.h"
#include "pqAutoGeneratedObjectPanel.h"
#include "pqLoadedFormObjectPanel.h"
#include "pqServerManagerObserver.h"
#include "pqPropertyManager.h"
#include "pqRenderViewModule.h"
#include "pqServerManagerModel.h"
#include "pqPipelineSource.h"
#include "pqObjectPanelInterface.h"
#include "pqCutPanel.h"
#include "pqClipPanel.h"
#include "pqCalculatorPanel.h"
#include "pqStreamTracerPanel.h"
#include "pqThresholdPanel.h"
#include "pqContourPanel.h"
#include "pqExodusPanel.h"
#include "pqXDMFPanel.h"
#include "pqPluginManager.h"
#include "pqLookmarkBrowserModel.h"
#include <QItemSelectionModel>
#include "pqServerStartupBrowser.h"
#include "pqServer.h"

class pqLookmarkInspectorForm : public Ui::pqLookmarkInspector {};


//-----------------------------------------------------------------------------
pqLookmarkInspector::pqLookmarkInspector(pqLookmarkBrowserModel *model, QWidget *p)
  : QWidget(p)
{
  this->setObjectName("lookmarkInspector");

  this->BrowserModel = model;
  this->Form = new pqLookmarkInspectorForm();
  this->Form->setupUi(this);
  this->Form->PropertiesFrame->hide();
  this->Form->ControlsFrame->hide();
  this->ActiveServer = 0;

  this->connect(this->Form->SaveButton, SIGNAL(clicked()), SLOT(save()));
  this->connect(this->Form->LoadButton, SIGNAL(clicked()), SLOT(load()));
  this->connect(this->Form->DeleteButton, SIGNAL(clicked()), SLOT(remove()));

  this->Form->SaveButton->setEnabled(false);
  this->Form->LoadButton->setEnabled(false);
  this->Form->DeleteButton->setEnabled(false);

  // Disable the restore data button until a fix can be made for the crash that's ocurring as ParaView closes when a lookmark that has this option turned off has been loaded.
  this->Form->RestoreData->setEnabled(false);

  this->connect(this->Form->RestoreData, 
                SIGNAL(stateChanged(int)),
                SLOT(onRestoreDataModified()));

  this->connect(this->Form->RestoreCamera, 
                SIGNAL(stateChanged(int)),
                SLOT(onRestoreCameraModified()));

  this->connect(this->Form->LookmarkName, 
                SIGNAL(textChanged(const QString &)),
                SLOT(onLookmarkNameModified()));

  this->connect(this->Form->LookmarkComments, 
                SIGNAL(textChanged()),
                SLOT(onLookmarkCommentsModified()));

  this->connect(this, 
                SIGNAL(modified()),
                SLOT(onModified()));

}

//-----------------------------------------------------------------------------
pqLookmarkInspector::~pqLookmarkInspector()
{
}


void pqLookmarkInspector::load()
{

  if(this->ActiveServer && this->CurrentSelection.at(0).isValid())
    {
    this->BrowserModel->loadLookmark(this->CurrentSelection.at(0), this->ActiveServer);
    }
  else
    {
    pqServerStartupBrowser* const server_browser = new pqServerStartupBrowser(
      pqApplicationCore::instance()->serverStartups(),
      *pqApplicationCore::instance()->settings(),
      this);
    server_browser->setAttribute(Qt::WA_DeleteOnClose);  // auto delete when closed
    QObject::connect(server_browser, SIGNAL(serverConnected(pqServer*)), 
      this, SLOT(load(pqServer*)), Qt::QueuedConnection);
    server_browser->setModal(true);
    server_browser->show();
    return;
    }
}


//-----------------------------------------------------------------------------
void pqLookmarkInspector::load(pqServer *server)
{ 
  if(this->CurrentSelection.at(0).isValid())
    {
    this->BrowserModel->loadLookmark(this->CurrentSelection.at(0),server);
    }

  this->ActiveServer = server;
}


//-----------------------------------------------------------------------------
void pqLookmarkInspector::remove()
{ 
  if(this->CurrentSelection.at(0).isValid())
    {
    this->BrowserModel->removeLookmark(this->BrowserModel->getLookmarkName(this->CurrentSelection.at(0)));
    }

  // this should change the selection in the browser model which will call this->onLookmarkSelectionChanged()
}


void pqLookmarkInspector::save()
{

  if(this->CurrentSelection.count()==1)
    {
    this->BrowserModel->setLookmarkName(this->CurrentSelection.at(0),this->Form->LookmarkName->text());
    this->BrowserModel->setLookmarkComments(this->CurrentSelection.at(0),this->Form->LookmarkComments->toPlainText());
    }

  QList<QModelIndex>::iterator iter;
  for(iter = this->CurrentSelection.begin(); iter!=this->CurrentSelection.end(); ++iter)
    {
    this->BrowserModel->setLookmarkRestoreDataFlag(this->CurrentSelection.at(0),this->Form->RestoreData->isChecked());
    this->BrowserModel->setLookmarkRestoreCameraFlag(this->CurrentSelection.at(0),this->Form->RestoreCamera->isChecked());
    }

  this->Form->SaveButton->setEnabled(false);
}

//-----------------------------------------------------------------------------
void pqLookmarkInspector::onRestoreCameraModified()
{
  emit this->modified();
}

//-----------------------------------------------------------------------------
void pqLookmarkInspector::onRestoreDataModified()
{
  emit this->modified();
}


//-----------------------------------------------------------------------------
void pqLookmarkInspector::onLookmarkCommentsModified()
{
  emit this->modified();
}


//-----------------------------------------------------------------------------
void pqLookmarkInspector::onLookmarkNameModified()
{
  emit this->modified();
}

//-----------------------------------------------------------------------------
void pqLookmarkInspector::onModified()
{
  this->Form->SaveButton->setEnabled(true);
}


//-----------------------------------------------------------------------------
void pqLookmarkInspector::onLookmarkSelectionChanged(const QItemSelection &selected) //, const QItemSelection &deselected)
{
  this->CurrentSelection = selected.indexes();

  if(selected.count()==0)
    {
    // don't display anything if nothing is selected
    this->Form->PropertiesFrame->hide();
    this->Form->ControlsFrame->hide();
    this->Form->LoadButton->setEnabled(false);
    this->Form->SaveButton->setEnabled(false);
    this->Form->DeleteButton->setEnabled(false);
    }
  else if(selected.count()>1)
    {
    // only display the lookmark settings that are applicable to multiple lookmarks
    this->Form->PropertiesFrame->hide();
    this->Form->ControlsFrame->show();
    this->Form->LoadButton->setEnabled(false);
    this->Form->SaveButton->setEnabled(false);
    this->Form->DeleteButton->setEnabled(true);
    }  
  else if(selected.count()==1)
    {
    this->Form->LookmarkName->setText(this->BrowserModel->getLookmarkName(this->CurrentSelection.at(0)));
    this->Form->LookmarkData->setText(this->BrowserModel->getLookmarkDataName(this->CurrentSelection.at(0)));
    this->Form->LookmarkComments->setText(this->BrowserModel->getLookmarkComments(this->CurrentSelection.at(0)));
    QImage img;
    img = this->BrowserModel->getLargeLookmarkIcon(this->CurrentSelection.at(0));
    if(!img.isNull())
      {
      this->Form->LookmarkIcon->setPixmap(QPixmap::fromImage(img));
      }
    img = this->BrowserModel->getLookmarkPipeline(this->CurrentSelection.at(0));
    if(!img.isNull())
      {
      this->Form->LookmarkPipeline->setPixmap(QPixmap::fromImage(img));
      }
    this->Form->RestoreData->setChecked(this->BrowserModel->getLookmarkRestoreDataFlag(this->CurrentSelection.at(0)));
    this->Form->RestoreCamera->setChecked(this->BrowserModel->getLookmarkRestoreCameraFlag(this->CurrentSelection.at(0)));

    this->Form->PropertiesFrame->show();
    this->Form->ControlsFrame->show();
    this->Form->LoadButton->setEnabled(true);
    this->Form->SaveButton->setEnabled(false);
    this->Form->DeleteButton->setEnabled(true);
    }
}


QSize pqLookmarkInspector::sizeHint() const
{
  // return a size hint that would reasonably fit several properties
  ensurePolished();
  QFontMetrics fm(font());
  int h = 20 * (qMax(fm.lineSpacing(), 14));
  int w = fm.width('x') * 25;
  QStyleOptionFrame opt;
  opt.rect = rect();
  opt.palette = palette();
  opt.state = QStyle::State_None;
  return (style()->sizeFromContents(QStyle::CT_LineEdit, &opt, QSize(w, h).
                                    expandedTo(QApplication::globalStrut()), this));
}
