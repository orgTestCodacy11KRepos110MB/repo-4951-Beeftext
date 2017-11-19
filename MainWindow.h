/// \file
/// \author Xavier Michelon
///
/// \brief Declaration of the application's main window
///  
/// Copyright (c) Xavier Michelon. All rights reserved.  
/// Licensed under the MIT License. See LICENSE file in the project root for full license information.  


#ifndef BEEFTEXT__MAIN__WINDOW__H
#define BEEFTEXT__MAIN__WINDOW__H


#include "ui_MainWindow.h"
#include <memory>


//**********************************************************************************************************************
/// \brief Main window class
//**********************************************************************************************************************
class MainWindow: public QMainWindow
{
   Q_OBJECT
public: // member functions
   MainWindow(); ///< Default constructor
   ~MainWindow() = default; ///< Default destructor
	
private: // member functions
   MainWindow(MainWindow const&) = delete; ///< Disabled copy constructor
   MainWindow(MainWindow&&) = delete; ///< Disabled move constructor
   MainWindow& operator=(MainWindow const&) = delete; ///< Disabled assignment operator
   MainWindow& operator=(MainWindow&&) = delete; ///< Disabled move assignment operator
   void setupComboTable(); ///< Setup the combo table
   void setupActions(); ///< Setup the Qt actions for the main window
   void setupSystemTrayIcon(); ///< Setup the system tray icon

private slots: 
   void onSystemTrayIconActivated(QSystemTrayIcon::ActivationReason reason); ///< Slot for the activation of the system tray icon
   void onActionShowMainWindow(); ///< Slot for the  'Show Main Window' action
   void onActionExit(); ///< Slot for the 'Exit' action

protected:
   void closeEvent(QCloseEvent *event) override; ///< Window close event handler

private: // data members
   Ui::MainWindow ui_; ///< The GUI for the window
   QSystemTrayIcon systemTrayIcon_; ///< The system tray icon
};


#endif // #ifndef BEEFTEXT__MAIN__WINDOW__H

