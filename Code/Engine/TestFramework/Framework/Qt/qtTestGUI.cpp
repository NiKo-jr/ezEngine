#include <TestFramework/PCH.h>

#ifdef EZ_USE_QT

#include <QtWidgets>
#include <QMessageBox>
#include <QStringBuilder>
#include <TestFramework/Framework/Qt/qtTestGUI.h>
#include <TestFramework/Framework/Qt/qtTestFramework.h>
#include <TestFramework/Framework/Qt/qtTestModel.h>
#include <TestFramework/Framework/Qt/qtTestDelegate.h>
#include <TestFramework/Framework/Qt/qtLogMessageDock.h>
#include <TestFramework/Utilities/TestOrder.h>

////////////////////////////////////////////////////////////////////////
// ezQtTestGUI public functions
////////////////////////////////////////////////////////////////////////

ezQtTestGUI::ezQtTestGUI(ezQtTestFramework& testFramework) : QMainWindow(), m_pTestFramework(&testFramework), m_pModel(NULL), m_bAbort(false)
{
  this->setupUi(this);

  // Status Bar
  m_pStatusText = new QLabel(this);
  testStatusBar->addWidget(m_pStatusText);

  // Model
  m_pModel = new ezQtTestModel(this, m_pTestFramework);
  testTreeView->setModel(m_pModel);

  // Delegate
  m_pDelegate = new ezQtTestDelegate(this);

  // View
  testTreeView->expandAll();
  testTreeView->resizeColumnToContents(0);
  testTreeView->header()->setStretchLastSection(false);
  testTreeView->setContextMenuPolicy(Qt::CustomContextMenu);
  testTreeView->setUniformRowHeights(true);
  testTreeView->setItemDelegate(m_pDelegate);
  testTreeView->setSelectionMode(QAbstractItemView::SingleSelection);
  testTreeView->setSelectionBehavior(QAbstractItemView::SelectRows);

  // Message Log Dock
  m_pMessageLogDock = new ezQtLogMessageDock(this);
  addDockWidget(Qt::RightDockWidgetArea, m_pMessageLogDock);

  // connect custom context menu
  connect(testTreeView, SIGNAL( customContextMenuRequested(const QPoint&) ), this, SLOT( onTestTreeViewCustomContextMenuRequested(const QPoint&) ));

  // connect current row changed signal
  QItemSelectionModel* pSelectionModel = testTreeView->selectionModel();
  bool bBla = connect(pSelectionModel, SIGNAL( currentRowChanged(const QModelIndex&, const QModelIndex&) ), this, SLOT( onSelectionModelCurrentRowChanged(const QModelIndex&) ));

  // Sync actions with test framework settings
  TestSettings settings = m_pTestFramework->GetSettings();
  this->actionAssertOnTestFail->setChecked(settings.m_bAssertOnTestFail);
  this->actionOpenHTMLOutput->setChecked(settings.m_bOpenHtmlOutput);
  this->actionKeepConsoleOpen->setChecked(settings.m_bKeepConsoleOpen);

  connect(m_pTestFramework, SIGNAL( TestResultReceived(qint32, qint32) ), this, SLOT( onTestFrameworkTestResultReceived(qint32, qint32) ) );

  UpdateButtonStates();
}

ezQtTestGUI::~ezQtTestGUI()
{
  testTreeView->setModel(NULL);
  testTreeView->setItemDelegate(NULL);
  delete m_pModel;
  m_pModel = NULL;
  delete m_pDelegate;
  m_pDelegate = NULL;
}


////////////////////////////////////////////////////////////////////////
// ezQtTestGUI public slots
////////////////////////////////////////////////////////////////////////

void ezQtTestGUI::on_actionAssertOnTestFail_triggered(bool bChecked)
{
  TestSettings settings = m_pTestFramework->GetSettings();
  settings.m_bAssertOnTestFail = bChecked;
  m_pTestFramework->SetSettings(settings);
  m_pTestFramework->SaveTestOrder();
}

void ezQtTestGUI::on_actionOpenHTMLOutput_triggered(bool bChecked)
{
  TestSettings settings = m_pTestFramework->GetSettings();
  settings.m_bOpenHtmlOutput = bChecked;
  m_pTestFramework->SetSettings(settings);
  m_pTestFramework->SaveTestOrder();
}

void ezQtTestGUI::on_actionKeepConsoleOpen_triggered(bool bChecked)
{
  TestSettings settings = m_pTestFramework->GetSettings();
  settings.m_bKeepConsoleOpen = bChecked;
  m_pTestFramework->SetSettings(settings);
  m_pTestFramework->SaveTestOrder();
}

void ezQtTestGUI::on_actionRunSelectedTests_triggered()
{
  m_pTestFramework->StartTests();
  m_pModel->Reset();
  m_pMessageLogDock->currentTestResultChanged(NULL);
  UpdateButtonStates();

  const ezUInt32 uiTestCount = m_pTestFramework->GetTestCount();
  for (ezUInt32 uiTestIdx = 0; uiTestIdx < uiTestCount; ++uiTestIdx)
  {
    // Check state of the test is evaluated by the execute function
    m_pTestFramework->ExecuteTest(uiTestIdx);
    if (m_bAbort)
    {
      break;
    }
  }
  m_pTestFramework->EndTests();
  UpdateButtonStates();

  if (m_bAbort)
  {
    QMessageBox::information(this, "Tests Aborted", "The tests were aborted by the user.", QMessageBox::Ok, QMessageBox::Ok);
    m_bAbort = false;
  }
  else
  {
    if (m_pTestFramework->GetTotalErrorCount() > 0)
    {
      QMessageBox::critical(this, "Tests Failed", "Some tests have failed.", QMessageBox::Ok, QMessageBox::Ok);
    }
    else
    {
      QMessageBox::information(this, "Tests Succeeded", "All tests succeeded.", QMessageBox::Ok, QMessageBox::Ok);
    }
  }
}

void ezQtTestGUI::on_actionRunAllTests_triggered()
{
  m_pTestFramework->SetAllTestsEnabledStatus(true);
  on_actionRunSelectedTests_triggered();
}

void ezQtTestGUI::on_actionAbort_triggered()
{
  m_bAbort = true;
}

void ezQtTestGUI::on_actionQuit_triggered()
{
  close();
}

void ezQtTestGUI::on_actionEnableOnlyThis_triggered()
{
  QModelIndex CurrentIndex = testTreeView->currentIndex();
  if (!CurrentIndex.isValid())
  {
    return;
  }
  CurrentIndex = m_pModel->index(CurrentIndex.row(), 0, CurrentIndex.parent());

  m_pTestFramework->SetAllTestsEnabledStatus(false);
  // Need to set data on column 0
  m_pModel->setData(CurrentIndex, Qt::Checked, Qt::CheckStateRole);
  QModelIndex ParentIndex = m_pModel->parent(CurrentIndex);
  if (ParentIndex.isValid())
  {
    // The parent is always at column 0 so there is no need to adjust it.
    m_pModel->setData(ParentIndex, Qt::Checked, Qt::CheckStateRole);
  }
  else
  {
    // There is no parent so the index is a test and must have sub-tests.
    ezInt32 iChildren = m_pModel->rowCount(CurrentIndex);
    for (ezInt32 i = 0; i < iChildren; ++i)
    {
      m_pModel->setData(CurrentIndex.child(i, 0), Qt::Checked, Qt::CheckStateRole);
    }
  }
  m_pModel->dataChanged(QModelIndex(),QModelIndex());
}

void ezQtTestGUI::on_actionEnableAll_triggered()
{
  m_pTestFramework->SetAllTestsEnabledStatus(true);
  m_pModel->dataChanged(QModelIndex(),QModelIndex());
}

void ezQtTestGUI::on_actionDisableAll_triggered()
{
  m_pTestFramework->SetAllTestsEnabledStatus(false);
  m_pModel->dataChanged(QModelIndex(),QModelIndex());
}

void ezQtTestGUI::on_actionExpandAll_triggered()
{
  testTreeView->expandAll();
}

void ezQtTestGUI::on_actionCollapseAll_triggered()
{
  testTreeView->collapseAll();
}

void ezQtTestGUI::onTestFrameworkTestResultReceived(qint32 iTestIndex, qint32 iSubTestIndex)
{
  m_pModel->TestDataChanged(iTestIndex, iSubTestIndex);

  QModelIndex TestModelIndex = m_pModel->index(iTestIndex, 0);
  QModelIndex LastSubTest = m_pModel->index(m_pModel->rowCount(TestModelIndex)-1, 0, TestModelIndex);
  testTreeView->scrollTo(LastSubTest);

  // Update status bar
  const ezUInt32 uiTestCount = m_pTestFramework->GetTestCount();
  const ezUInt32 uiFailed = m_pTestFramework->GetTestsFailedCount();
  const ezUInt32 uiPassed = m_pTestFramework->GetTestsPassedCount();
  const ezUInt32 uiErrors = m_pTestFramework->GetTotalErrorCount();
  double fTestDurationInSeconds = m_pTestFramework->GetTotalTestDuration() / 1000.0;

  // Get the current test's sub-test completion ratio
  ezTestEntry* pTest = m_pTestFramework->GetTest(m_pTestFramework->GetCurrentTestIndex());
  float fSubTestPercentage = 0.0f;
  if (pTest != NULL && iSubTestIndex != -1)
  {
    fSubTestPercentage = (float)pTest->GetSubTestCount(ezTestResultQuery::Executed) / (float)pTest->GetSubTestCount(ezTestResultQuery::Enabled);
  }

  float fProgress = 100.0f * (fSubTestPercentage + uiFailed + uiPassed) / uiTestCount;
  QString sStatusText = QLatin1String("[progress: ") % QString::number(fProgress, 'f', 2) % QLatin1String("%] [passed: ") % QString::number(uiPassed)
    % QLatin1String("] [failed: ") % QString::number(uiFailed) % QLatin1String("] [errors: ") % QString::number(uiErrors) % QLatin1String("] [time taken: ")
    % QString::number(fTestDurationInSeconds, 'f', 2) % QLatin1String(" seconds]");
  m_pStatusText->setText(sStatusText);

  QApplication::processEvents();
}

void ezQtTestGUI::onTestTreeViewCustomContextMenuRequested(const QPoint& pnt)
{
  QModelIndex CurrentIndex = testTreeView->currentIndex();

  QMenu ContextMenu;
  if (CurrentIndex.isValid())
  {
    ContextMenu.addAction(actionEnableOnlyThis);
  }
  ContextMenu.addAction(actionEnableAll);
  ContextMenu.addAction(actionDisableAll);
  ContextMenu.addSeparator();
  ContextMenu.addAction(actionExpandAll);
  ContextMenu.addAction(actionCollapseAll);

  ContextMenu.exec(testTreeView->viewport()->mapToGlobal(pnt));
}

void ezQtTestGUI::onSelectionModelCurrentRowChanged(const QModelIndex& index)
{
  if (!index.isValid())
  {
    m_pMessageLogDock->currentTestResultChanged(NULL);
  }

  const ezQtTestModelEntry* pEntry = (ezQtTestModelEntry*) index.internalPointer();
  const ezTestResult* pTestResult = pEntry->GetTestResult();

  m_pMessageLogDock->currentTestResultChanged(pTestResult);
}


////////////////////////////////////////////////////////////////////////
// ezQtTestGUI private functions
////////////////////////////////////////////////////////////////////////

void ezQtTestGUI::UpdateButtonStates()
{
  bool bTestsRunning = m_pTestFramework->GetTestsRunning();

  pushButtonQuit->setEnabled(!bTestsRunning);
  pushButtonAbort->setEnabled(bTestsRunning);
  pushButtonRunSelectedTests->setEnabled(!bTestsRunning);
  pushButtonRunAllTests->setEnabled(!bTestsRunning);
}


////////////////////////////////////////////////////////////////////////
// ezQtTestGUI public static functions
////////////////////////////////////////////////////////////////////////

void ezQtTestGUI::SetDarkTheme()
{
  //return;
  QApplication::setStyle(QStyleFactory::create("fusion"));
  //return;
  QPalette palette;

  palette.setColor(QPalette::WindowText, QColor(200, 200, 200, 255));
  palette.setColor(QPalette::Button, QColor(100, 100, 100, 255));
  palette.setColor(QPalette::Light, QColor(97, 97, 97, 255));
  palette.setColor(QPalette::Midlight, QColor(59, 59, 59, 255));
  palette.setColor(QPalette::Dark, QColor(37, 37, 37, 255));
  palette.setColor(QPalette::Mid, QColor(45, 45, 45, 255));
  palette.setColor(QPalette::Text, QColor(200, 200, 200, 255));
  palette.setColor(QPalette::BrightText, QColor(37, 37, 37, 255));
  palette.setColor(QPalette::ButtonText, QColor(200, 200, 200, 255));
  palette.setColor(QPalette::Base, QColor(42, 42, 42, 255));
  palette.setColor(QPalette::Window, QColor(68, 68, 68, 255));
  palette.setColor(QPalette::Shadow, QColor(0, 0, 0, 255));
  palette.setColor(QPalette::Highlight, QColor(103, 141, 178, 255));
  palette.setColor(QPalette::HighlightedText, QColor(255, 255, 255, 255));
  palette.setColor(QPalette::Link, QColor(0, 0, 238, 255));
  palette.setColor(QPalette::LinkVisited, QColor(82, 24, 139, 255));
  palette.setColor(QPalette::AlternateBase, QColor(46, 46, 46, 255));
  QBrush NoRoleBrush(QColor(0, 0, 0, 255), Qt::NoBrush);
  palette.setBrush(QPalette::NoRole, NoRoleBrush);
  palette.setColor(QPalette::ToolTipBase, QColor(255, 255, 220, 255));
  palette.setColor(QPalette::ToolTipText, QColor(0, 0, 0, 255));

  palette.setColor(QPalette::Disabled, QPalette::WindowText, QColor(128, 128, 128, 255));
  palette.setColor(QPalette::Disabled, QPalette::Button, QColor(80, 80, 80, 255));
  palette.setColor(QPalette::Disabled, QPalette::Text, QColor(105, 105, 105, 255));
  palette.setColor(QPalette::Disabled, QPalette::BrightText, QColor(255, 255, 255, 255));
  palette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(128, 128, 128, 255));
  palette.setColor(QPalette::Disabled, QPalette::Highlight, QColor(86, 117, 148, 255));

  QApplication::setPalette(palette);
}
#endif
