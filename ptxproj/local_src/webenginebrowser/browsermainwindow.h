/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the demonstration applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef BROWSERMAINWINDOW_H
#define BROWSERMAINWINDOW_H

#include <QtWidgets/QMainWindow>
#include <QtGui/QIcon>
#include <QtCore/QUrl>
#include <QTextCodec>
#include "CustomSwipeGesture.h"
#include "cmdthread.h"


QT_BEGIN_NAMESPACE

class QPrinter;
class QWebEnginePage;
QT_END_NAMESPACE

class AutoSaver;
class BookmarksToolBar;
class ChaseWidget;
class TabWidget;
class ToolbarSearch;
class WebView;

class cmdThread;

/*!
    The MainWindow of the Browser Application.

    Handles the tab widget and all the actions
 */
class BrowserMainWindow : public QMainWindow
{
  Q_OBJECT

public:
  BrowserMainWindow(QWidget *parent = 0, Qt::WindowFlags flags = 0);
  ~BrowserMainWindow();
  QSize sizeHint() const;

  static const char *defaultHome;

  cmdThread * pCmdThread;
  void startCmdThread();

  void ReadWebengineConfigurationFile(QString sFile);
  void ReadPlcselectConfigFile(QString sFile, bool bReload = false);

public:
  TabWidget *tabWidget() const;
  WebView *currentTab() const;
  QByteArray saveState(bool withTabs = true) const;
  bool restoreState(const QByteArray &state);
  Q_INVOKABLE void runScriptOnOpenViews(const QString &);
  bool isNavbarVisible();

public slots:
  void loadPage(const QString &url);
  void slotHome();
  void slotCmdReceived(QString s);
  
  // gesture
  void scrollSlotVDown(QPoint pos);
  void scrollSlotVUp(QPoint pos);
  void scrollSlotHLeft(QPoint pos);
  void scrollSlotHRight(QPoint pos);
  void slotExecuteJsScrollbarVertCode();
  void slotExecuteJsScrollbarHorizCode();

protected:
  void closeEvent(QCloseEvent *event);
  void showEvent(QShowEvent *event);

private slots:
  void save();

  void slotLoadProgress(int);
  void slotUpdateWindowTitle(const QString &title = QString());

  void loadUrl(const QUrl &url);
  void slotPreferences();

  void slotFileNew();
  void slotFileOpen();
  void slotFilePrintPreview();
  void slotFilePrint();
  void slotFilePrintToPDF();
  void slotPrivateBrowsing();
  void slotFileSaveAs();
  void slotEditFind();
  void slotEditFindNext();
  void slotEditFindPrevious();
  void slotShowBookmarksDialog();
  void slotAddBookmark();
  void slotViewZoomIn();
  void slotViewZoomOut();
  void slotViewResetZoom();
  void slotViewToolbar();
  void slotViewBookmarksBar();

  void slotToggleInspector(bool enable);
  void slotDownloadManager();
  void slotSelectLineEdit();

  void slotAboutToShowBackMenu();
  void slotAboutToShowForwardMenu();
  void slotAboutToShowWindowMenu();
  void slotOpenActionUrl(QAction *action);
  void slotShowWindow();
  void slotSwapFocus();
  void slotHandlePdfPrinted(const QByteArray&);

  void slotHandlePagePrinted(bool result);
  void printRequested(QWebEnginePage *page);
  void geometryChangeRequested(const QRect &geometry);
  void updateToolbarActionText(bool visible);
  void updateBookmarksToolbarActionText(bool visible);

private:
  void loadDefaultState();
  void setupMenu();
  void setupToolBar();
  void handleFindTextResult(bool found);
  
  //gesture
  //Qt::GestureType m_gestureId;
  int iScrollWheelValue;
  CustomSwipeGesture* pRecognizer;
  Qt::GestureType SwipeGestureType;
  void InitScrollGestures();


  QToolBar *m_navigationBar;
  //ToolbarSearch *m_toolbarSearch;
  BookmarksToolBar *m_bookmarksToolbar;
  ChaseWidget *m_chaseWidget;
  TabWidget *m_tabWidget;
  AutoSaver *m_autoSaver;

  QAction *m_historyBack;
  QMenu *m_historyBackMenu;
  QAction *m_historyForward;
  QMenu *m_historyForwardMenu;
  QMenu *m_windowMenu;

  QAction *m_stop;
  QAction *m_reload;
  QAction *m_stopReload;
  QAction *m_viewToolbar;
  QAction *m_viewBookmarkBar;
  QAction *m_restoreLastSession;
  QAction *m_addBookmark;

  QPrinter *m_currentPrinter;

  QIcon m_reloadIcon;
  QIcon m_stopIcon;

  QString m_lastSearch;
  QString m_printerOutputFileName;
  friend class BrowserApplication;

  bool IsCursorOnScrollbarHorizontal(QPoint pos);
  bool IsCursorOnScrollbarVertical(QPoint pos);

};

#endif // BROWSERMAINWINDOW_H
