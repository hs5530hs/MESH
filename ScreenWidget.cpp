/* $Id$ */


/*
 *
 *  Copyright (C) 2001-2004 EPFL (Swiss Federal Institute of Technology,
 *  Lausanne) This program is free software; you can redistribute it
 *  and/or modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 *  USA.
 *
 *  In addition, as a special exception, EPFL gives permission to link
 *  the code of this program with the Qt non-commercial edition library
 *  (or with modified versions of Qt non-commercial edition that use the
 *  same license as Qt non-commercial edition), and distribute linked
 *  combinations including the two.  You must obey the GNU General
 *  Public License in all respects for all of the code used other than
 *  Qt non-commercial edition.  If you modify this file, you may extend
 *  this exception to your version of the file, but you are not
 *  obligated to do so.  If you do not wish to do so, delete this
 *  exception statement from your version.
 *
 *  Authors : Nicolas Aspert, Diego Santa-Cruz and Davy Jacquet
 *
 *  Web site : http://mesh.epfl.ch
 *
 *  Reference :
 *   "MESH : Measuring Errors between Surfaces using the Hausdorff distance"
 *   in Proceedings of IEEE Intl. Conf. on Multimedia and Expo (ICME) 2002, 
 *   vol. I, pp. 705-708, available on http://mesh.epfl.ch
 *
 */







#include "ScreenWidget.h"

#include <q3hbox.h>
#include <qlayout.h>
#include <qaction.h>
#include <qapplication.h>
#include <qmessagebox.h>
#include <qmenubar.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <q3buttongroup.h>
//#include <qvbuttongroup.h>
#include <q3groupbox.h>
#include <qcheckbox.h>
#include <qstring.h>
#include <qlabel.h>
//Added by qt3to4:
#include <Q3GridLayout>
#include <Q3Frame>
#include <Q3PopupMenu>
#include "Lighted3DViewerWidget.h"
#include <Error3DViewerWidget.h>
#include <ColorMapWidget.h>
#include <mesh.h>
#include <mesh_run.h>
#include <qdesktopwidget.h>

ScreenWidget::ScreenWidget(struct model_error *model1,
                           struct model_error *model2,
                           const struct args *pargs,
                           QWidget *parent, 
                           const char *name ):QWidget(parent,name) {
  QAction *fileQuitAction;
  QPushButton *lineSwitch1, *lineSwitch2;
  QMenuBar *mainBar;
  Q3PopupMenu *fileMenu, *infoMenu, *helpMenu;
  Q3HBox *frameModel1, *frameModel2;
  Q3HBox *qhbTimer;
  Q3GridLayout *bigGrid, *smallGrid;
  Lighted3DViewerWidget *glModel2; // Right GLWidget
  Error3DViewerWidget *glModel1; // Left GLWidget
  ColorMapWidget *errorColorBar;
  QPushButton *quitBut;
  QRadioButton *verrBut, *fmerrBut, *serrBut;
  QRadioButton *linBut, *logBut;
  QRadioButton *gsBut, *hsvBut;
  Q3ButtonGroup *dispInfoGrp=NULL, *histoGrp=NULL, *histoColGrp=NULL, *rmodGrp;
  QCheckBox  *qcbLight;
  Q3GroupBox *qgbTimer;
  QLabel *qlM1Name, *qlM2Name, *qlTimer;
  QString tmp;
  int do_texture = pargs->do_texture;
  const float p = 0.95f; // max proportion of screen to use
  const float wLF = 0.2f; // proportion for width of Line/Fill switch
  const float wSync = 0.3f; // proportion for width of Sync. switch
  const float wQ = 0.15f; // proportion for width of Quit button
  int max_ds; // maximum downsampling value
  int i;
  
  tmp.sprintf("MESH %s - Visualization", version);
  setCaption(tmp);
  setMinimumWidth(400); // this is a reasonable assumption... and
                        // we need it to have a correct appearance of
                        // all the buttons in this widget (in fact
                        // only when using Qt >= 3.0.0)

//  fileQuitAction = new QAction( "Quit", "Quit", Qt::CTRL+Qt::Key_Q, this, "quit" );
  fileQuitAction = new QAction( "Quit", this);
  connect(fileQuitAction, SIGNAL(activated()) , 
	  qApp, SLOT(closeAllWindows()));


  // Create the 'File' menu
  mainBar = new QMenuBar(this);
  fileMenu = new Q3PopupMenu(this);
  mainBar->insertItem("&File",fileMenu);
  fileQuitAction->addTo(fileMenu);

  // Create the 'Info' menu
  infoMenu = new Q3PopupMenu(this);
  // save the adresses of these structures for later
  locMod1 = model1;
  locMod2 = model2;
  mainBar->insertItem("&Info", infoMenu);
  infoMenu->insertItem("Left model information", this, 
                       SLOT(infoLeftModel()));
  infoMenu->insertItem("Right model information", this, 
                       SLOT(infoRightModel()));

  //Create the 'Help' menu
  helpMenu = new Q3PopupMenu(this);
  mainBar->insertItem("&Help", helpMenu);
  helpMenu->insertItem("&Key utilities", this, SLOT(aboutKeys()),Qt::CTRL+Qt::Key_H);
  helpMenu->insertItem("&Bug", this, SLOT(aboutBugs()));
  helpMenu->insertItem("&About", this, SLOT(aboutMesh()));

  // --------------
  // Create the GUI
  // --------------

  // Create frames to put around the OpenGL widgets
  tmp.sprintf("%s", pargs->m1_fname);
  qlM1Name = new QLabel(tmp, this);
  frameModel1 = new Q3HBox(this, "frameModel1");
  frameModel1->setFrameStyle(Q3Frame::Sunken | Q3Frame::Panel);
  frameModel1->setLineWidth(2);

  tmp.sprintf("%s", pargs->m2_fname);
  qlM2Name = new QLabel(tmp, this);
  frameModel2 = new Q3HBox(this, "frameModel2");
  frameModel2->setFrameStyle(Q3Frame::Sunken | Q3Frame::Panel);
  frameModel2->setLineWidth(2);

  glModel1 = new Error3DViewerWidget(model1, (bool)do_texture, 
				     frameModel1, "ErrorViewer");

  glModel1->setFocusPolicy(Qt::StrongFocus);
  glModel2 = new Lighted3DViewerWidget(model2,
				       frameModel2, "LightedViewer");
  glModel2->setFocusPolicy(Qt::StrongFocus);
  errorColorBar = new ColorMapWidget(model1, this, "errorColorBar");

  // This is to synchronize the viewpoints of the two models
  // We need to pass the viewing matrix from one RawWidget
  // to another
  connect(glModel1, SIGNAL(transferViewParams(double,double,double,double*)), 
	  glModel2, SLOT(setViewParams(double,double,double,double*)));
  connect(glModel2, SIGNAL(transferViewParams(double,double,double,double*)), 
	  glModel1, SLOT(setViewParams(double,double,double,double*)));


  // Build synchro and quit buttons
  syncBut = new QPushButton("Synchronize\nviewpoints", this);
  syncBut->setToggleButton(TRUE);
  syncBut->setMinimumWidth((int)(minimumWidth()*wSync));

  connect(syncBut, SIGNAL(toggled(bool)), 
	  glModel1, SLOT(switchSync(bool))); 
  connect(syncBut, SIGNAL(toggled(bool)), 
	  glModel2, SLOT(switchSync(bool)));
  connect(glModel1, SIGNAL(toggleSync()),syncBut, SLOT(toggle()));
  connect(glModel2, SIGNAL(toggleSync()),syncBut, SLOT(toggle()));

  quitBut = new QPushButton("Quit", this);
  connect(quitBut, SIGNAL(clicked()), this, SLOT(close()));
  quitBut->setMinimumWidth((int)(minimumWidth()*wQ));

  // Build the two line/fill toggle buttons
  lineSwitch1 = new QPushButton("Line/Fill", this);
  lineSwitch1->setToggleButton(TRUE);
  lineSwitch1->setMinimumWidth((int)(minimumWidth()*wLF));

  connect(lineSwitch1, SIGNAL(toggled(bool)), 
	  glModel1, SLOT(setLine(bool)));
  connect(glModel1, SIGNAL(toggleLine()),lineSwitch1, SLOT(toggle()));


  lineSwitch2 = new QPushButton("Line/Fill", this);
  lineSwitch2->setToggleButton(TRUE);
  lineSwitch2->setMinimumWidth((int)(minimumWidth()*wLF));

  connect(lineSwitch2, SIGNAL(toggled(bool)), 
	  glModel2, SLOT(setLine(bool)));
  connect(glModel2, SIGNAL(toggleLine()),lineSwitch2, SLOT(toggle()));
  
  // Build the checkboxes for right-model parameters
  rmodGrp = new Q3VButtonGroup("Right model", this);

  qcbInvNorm = new QCheckBox("Invert normals", rmodGrp);
  connect(qcbInvNorm, SIGNAL(toggled(bool)), 
	  glModel2, SLOT(invertNormals(bool)));
  connect(glModel2, SIGNAL(toggleNormals()),qcbInvNorm,
	  SLOT(toggle()));
  rmodGrp->insert(qcbInvNorm);

  qcbTwoSide = new QCheckBox("One-sided material", rmodGrp);
  connect(qcbTwoSide, SIGNAL(toggled(bool)), 
	  glModel2, SLOT(setTwoSidedMaterial(bool)));
  connect(glModel2, SIGNAL(toggleTwoSidedMaterial()),qcbTwoSide, 
	  SLOT(toggle()));
  rmodGrp->insert(qcbTwoSide);

  qcbLight = new QCheckBox("Light mode", rmodGrp);
  if (model2->mesh->normals) { // parameters make sense if the
                               // model has normals...
    qcbLight->setChecked(TRUE);
    connect(qcbLight, SIGNAL(toggled(bool)), 
            glModel2, SLOT(setLight(bool)));
    connect(glModel2, SIGNAL(toggleLight()), qcbLight, SLOT(toggle()));
    connect(qcbLight, SIGNAL(toggled(bool)), 
            this, SLOT(updatecbStatus(bool)));
  } 
  else
    qcbLight->setDisabled(TRUE);

  rmodGrp->insert(qcbLight);

  // Build error mode selection buttons
  dispInfoGrp = new Q3VButtonGroup("Displayed information (left)",this);
  dispInfoGrp->layout()->setMargin(3);
  verrBut = new QRadioButton("Vertex error", dispInfoGrp);
  verrBut->setChecked(TRUE);
  fmerrBut = new QRadioButton("Face mean error", dispInfoGrp);
  serrBut = new QRadioButton("Sample error", dispInfoGrp);
  dispInfoGrp->insert(verrBut, Error3DViewerWidget::VERTEX_ERROR);
  dispInfoGrp->insert(fmerrBut, Error3DViewerWidget::MEAN_FACE_ERROR);
  dispInfoGrp->insert(serrBut, Error3DViewerWidget::SAMPLE_ERROR);
  if (!do_texture) 
    serrBut->setDisabled(TRUE);
  connect(dispInfoGrp, SIGNAL(clicked(int)), 
          glModel1, SLOT(setErrorMode(int)));
  connect(dispInfoGrp, SIGNAL(clicked(int)), 
          this, SLOT(disableSlider(int)));

  // Build downsampling control
  for (i=0, max_ds=1; i<model1->mesh->num_faces; i++) {
    if (model1->fe[i].sample_freq > max_ds) 
      max_ds = model1->fe[i].sample_freq;
  }
  // This is needed s.t. we can add children widget to the GroupBox
  qgbSlider = new 
    Q3GroupBox(tmp.sprintf("Subsampling factor of the error = %d", max_ds), 
               this);

  qslidDispSampDensity = new QSlider(1, max_ds, 1, max_ds, 
                                     Qt::Horizontal, qgbSlider);
  qslidDispSampDensity->setTickInterval((max_ds-1)/5);
  qslidDispSampDensity->setTickmarks(QSlider::TicksBothSides);
  qslidDispSampDensity->setTracking(FALSE);
  glModel1->setVEDownSampling(max_ds); // Initialization

  qspSampDensity = new QSpinBox(1, max_ds, 1, qgbSlider);
  qspSampDensity->setValue(max_ds);


  // Connect the slider and spinbox to a 'phony' slot s.t. we avoid
  // loops between valueChanged signals and setValue slots
  connect(qspSampDensity, SIGNAL(valueChanged(int)), this, 
          SLOT(trapChanges(int)));
  connect(qslidDispSampDensity, SIGNAL(valueChanged(int)), this, 
          SLOT(trapChanges(int)));
  // The dsValChange signal is emitted once when there has been a real
  // change 
  connect(this, SIGNAL(dsValChange(int)), 
          glModel1, SLOT(setVEDownSampling(int)));

  // Build the demo mode stuff (toggle + speed vario.)
  qgbTimer = new Q3GroupBox("Demo mode parameters", this);
  qhbTimer = new Q3HBox(qgbTimer);

  qspTimerSpeed = new QSpinBox(1, 100, 1, qhbTimer);
  qlTimer = new QLabel("Speed", qhbTimer);
  qcbTimer = new QCheckBox("Demo mode on/off", qgbTimer);
  qcbTimer->setChecked(FALSE);

  connect(qcbTimer, SIGNAL(toggled(bool)), glModel1,
          SLOT(setTimer(bool)));
  connect(qcbTimer, SIGNAL(toggled(bool)), glModel2,
          SLOT(setTimer(bool)));
  connect(qspTimerSpeed, SIGNAL(valueChanged(int)), glModel1,
          SLOT(changeSpeed(int)));
  connect(qspTimerSpeed, SIGNAL(valueChanged(int)), glModel2,
          SLOT(changeSpeed(int)));
  connect(glModel1, SIGNAL(toggleTimer()), qcbTimer, SLOT(toggle()));
  connect(glModel2, SIGNAL(toggleTimer()), qcbTimer, SLOT(toggle()));
  connect(qcbTimer, SIGNAL(toggled(bool)), this, SLOT(disableSync(bool)));
  

  // Build scale selection buttons for the histogram
  histoGrp = new Q3VButtonGroup("X scale",this);
  linBut = new QRadioButton("Linear", histoGrp);
  linBut->setChecked(TRUE);
  logBut = new QRadioButton("Log", histoGrp);
  histoGrp->insert(linBut, ColorMapWidget::LIN_SCALE);
  histoGrp->insert(logBut, ColorMapWidget::LOG_SCALE);
  connect(histoGrp, SIGNAL(clicked(int)), 
          errorColorBar, SLOT(doHistogram(int)));

  // Build the colorspace selection button for the histogram
  histoColGrp = new Q3VButtonGroup("Colormap", this);
  hsvBut = new QRadioButton("HSV",histoColGrp);
  hsvBut->setChecked(TRUE);
  gsBut = new QRadioButton("Gray", histoColGrp);
  histoColGrp->insert(hsvBut, ColorMapWidget::HSV);
  histoColGrp->insert(gsBut, ColorMapWidget::GRAYSCALE);
  connect(histoColGrp, SIGNAL(clicked(int)), 
          errorColorBar, SLOT(setColorMap(int)));
  connect(histoColGrp, SIGNAL(clicked(int)), 
          glModel1, SLOT(setColorMap(int)));

  // Build the topmost grid layout
  bigGrid = new Q3GridLayout (this, 4, 7, 5, -1);
  bigGrid->setMenuBar(mainBar);
  bigGrid->addWidget(errorColorBar, 1, 0);
  bigGrid->addMultiCellWidget(qlM1Name, 0, 0, 1, 3, Qt::AlignCenter);
  bigGrid->addMultiCellWidget(qlM2Name, 0, 0, 4, 6, Qt::AlignCenter);
  bigGrid->addMultiCellWidget(frameModel1, 1, 1, 1, 3);
  bigGrid->addMultiCellWidget(frameModel2, 1, 1, 4, 6);
  bigGrid->addWidget(lineSwitch1, 2, 2, Qt::AlignCenter);
  bigGrid->addWidget(lineSwitch2, 2, 5, Qt::AlignCenter);
  bigGrid->addMultiCellWidget(syncBut, 2, 2, 3, 4, Qt::AlignCenter);
  bigGrid->addWidget(histoGrp, 2, 0, Qt::AlignTop);
  bigGrid->addWidget(histoColGrp, 3, 0, Qt::AlignTop);

  // sub layout for dispInfoGrp and Quit button -> avoid resize problems
  smallGrid = new Q3GridLayout(1, 6, 3);
  smallGrid->addWidget(dispInfoGrp, 0, 0, Qt::AlignCenter);
  smallGrid->addMultiCellWidget(qgbSlider, 0, 0, 1, 2, Qt::AlignCenter);
  smallGrid->addWidget(qgbTimer, 0, 3, Qt::AlignCenter);
  smallGrid->addWidget(rmodGrp, 0, 4, Qt::AlignCenter);
  smallGrid->addWidget(quitBut, 0, 5, Qt::AlignCenter);
  bigGrid->addMultiCellLayout(smallGrid, 3, 3, 1, 6);

  // Now set a sensible default widget size
  QSize prefSize = layout()->sizeHint();
  QSize screenSize = QApplication::desktop()->size();


  if (prefSize.width() > p*screenSize.width()) {
    prefSize.setWidth((int)(p*screenSize.width()));
  }
  if (prefSize.height() > p*screenSize.height()) {
    prefSize.setHeight((int)(p*screenSize.height()));
  }
  resize(prefSize.width(),prefSize.height());

}

void ScreenWidget::infoLeftModel()
{
  infoModel(locMod1, LEFT_MODEL);
}

void ScreenWidget::infoRightModel()
{
  infoModel(locMod2, RIGHT_MODEL);
}

void ScreenWidget::infoModel(struct model_error *model, int id) 
{
  QString tmp, fullText;
  fullText.sprintf("%d vertices\n%d triangles\n", model->mesh->num_vert, 
                   model->mesh->num_faces);

  // Orientable model ?
  if (model->info->orientable)
    fullText += "Orientable model\n";
  else 
    fullText += "Non-orientable model\n";

  // Manifold model ?
  if (model->info->manifold)
    fullText += "Manifold model\n";
  else
    fullText += "Non-manifold model\n"; 

  // Closed model ?
  if (model->info->closed)
    fullText += "Closed model\n";
  else
    fullText += "Non-closed model\n"; 

  fullText += tmp.sprintf("%d connected component(s)\n", 
                          model->info->n_disjoint_parts);

  if (model->info->n_degenerate > 0) 
    fullText += tmp.sprintf("%d degenerate triangle(s)\n", 
                            model->info->n_degenerate);

  switch(id) {
  case LEFT_MODEL:
    QMessageBox::information(this, "Left Model Information", fullText);
    break;
  case RIGHT_MODEL:
    QMessageBox::information(this, "Right Model Information", fullText);
    break;
  default:
    QMessageBox::warning(this, "Error", "Invalid paremeter !!\n");
    break;
  }
  
}

void ScreenWidget::changeGroupBoxTitle(int n) 
{
  QString tmp;

  qgbSlider->setTitle(tmp.sprintf("Subsampling factor of the error = %d", n));
}

void ScreenWidget::disableSlider(int errMode) 
{
  switch (errMode) {
  case (Error3DViewerWidget::VERTEX_ERROR):
    qgbSlider->setDisabled(FALSE);
    break;
  case (Error3DViewerWidget::MEAN_FACE_ERROR): 
  case (Error3DViewerWidget::SAMPLE_ERROR):
    qgbSlider->setDisabled(TRUE);
    break;
  default: /* should never get here */
    break;
  }
  return;
}

void ScreenWidget::disableSync(bool state) {
  if (state)
    syncBut->setDisabled(TRUE);
  else
    syncBut->setDisabled(FALSE);
}

void ScreenWidget::trapChanges(int n) 
{
  int slv = qslidDispSampDensity->value(); // value of the slider
  int spv = qspSampDensity->value(); // value of the spinbox
  bool hasChanged = FALSE;

  if (slv == n && spv == n)
    return;

  if (slv != n) {
    qslidDispSampDensity->setValue(n);
    hasChanged = TRUE;
  }

  if (spv != n) {
    qspSampDensity->setValue(n);
    hasChanged = TRUE;
  }
  
  if (hasChanged) {
    changeGroupBoxTitle(n);
    emit dsValChange(n);  
  }
}

void ScreenWidget::updatecbStatus(bool state) {
  if (!state) { // no light
    qcbTwoSide->setDisabled(TRUE);
    qcbInvNorm->setDisabled(TRUE);
  } else {
    qcbTwoSide->setDisabled(FALSE);
    qcbInvNorm->setDisabled(FALSE);
  }
    
}

void ScreenWidget::aboutMesh()
{
  QString msg;

  msg.sprintf("MESH version %s\n"
              "Copyright (C) %s\n"
              "Authors: Nicolas Aspert, Diego Santa Cruz, Davy Jacquet\n\n"
              "This is free software; see the source for copying conditions.\n"
              "There is NO warranty; not even for MERCHANTABILITY or\n"
              "FITNESS FOR A PARTICULAR PURPOSE.",
              version, copyright);
  QMessageBox::about(this, "MESH", msg);
}

void ScreenWidget::aboutKeys()
{
    QMessageBox::about( this, "Key bindings",
                        "T : Toggle demo mode\n"
			"F1: Toggle Wireframe/Fill\n"
			"F2: Toggle lighting (right model only)\n"
			"F3: Toggle viewpoint synchronization\n"
			"F4: Invert normals (right model only)\n"
                        "F5: Toggle two sided material (right model only)");
}

void ScreenWidget::aboutBugs()
{
    QMessageBox::about( this, "Bug",
			"If you found a bug, please send an e-mail to :\n"
			"Nicolas.Aspert@epfl.ch and/or\n"
			"Diego.SantaCruz@epfl.ch");
}

void ScreenWidget::quit()
{
  QApplication::exit(0);
}


