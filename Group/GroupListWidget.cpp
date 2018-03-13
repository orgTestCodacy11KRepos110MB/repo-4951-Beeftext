/// \file
/// \author Xavier Michelon
///
/// \brief Declaration of combo group list widget
///
/// Copyright (c) Xavier Michelon. All rights reserved.  
/// Licensed under the MIT License. See LICENSE file in the project root for full license information.  


#include "stdafx.h"
#include "GroupDialog.h"
#include "GroupListWidget.h"
#include "MimeDataUtils.h"
#include "Combo/ComboManager.h"
#include <XMiLib/Exception.h>


//**********************************************************************************************************************
/// \param[in] parent The parent of the widget
//**********************************************************************************************************************
GroupListWidget::GroupListWidget(QWidget* parent)
   : QWidget(nullptr)
   , contextMenu_(nullptr)
{
   ui_.setupUi(this);
   GroupList& groups = ComboManager::instance().comboListRef().groupListRef();
   ui_.listGroup->setModel(&groups);
   ui_.listGroup->viewport()->installEventFilter(this);
   this->setupContextMenu();
   this->setupGroupsMenu();
   QItemSelectionModel* selectionModel = ui_.listGroup->selectionModel();
   if (!selectionModel)
      throw xmilib::Exception("The group list selection model is null");
   connect(selectionModel, &QItemSelectionModel::selectionChanged, this, &GroupListWidget::onSelectionChanged);
   connect(&groups, &GroupList::groupMoved, this, &GroupListWidget::onGroupMoved);
   this->updateGui();
   QTimer::singleShot(0, [&]() {ui_.listGroup->setCurrentIndex(groups.index(0)); }); // delayed to be sure the signal/slot mechanism work
}


//**********************************************************************************************************************
/// \return the selected group
/// \return a null pointer if nor group is selected
//**********************************************************************************************************************
SPGroup GroupListWidget::selectedGroup() const
{
   GroupList const& list = ComboManager::instance().groupListRef();
   qint32 const index = this->selectedGroupIndex();
   return ((index >= 0) && (index < list.size())) ? list[index] : SPGroup();
}


//**********************************************************************************************************************
// 
//**********************************************************************************************************************
void GroupListWidget::clearSelection() const
{
   if (this->selectedGroup())
      ui_.listGroup->clearSelection();
}


//**********************************************************************************************************************
// 
//**********************************************************************************************************************
void GroupListWidget::updateGui()
{
   qint32 const index = this->selectedGroupIndex();
   bool const validIndex = (index >= 0) && (index < ComboManager::instance().groupListRef().size());
   ui_.actionDeleteGroup->setEnabled(validIndex);
   ui_.actionEditGroup->setEnabled(validIndex);
}


//**********************************************************************************************************************
// 
//**********************************************************************************************************************
void GroupListWidget::setupGroupsMenu()
{
   ui_.buttonGroups->setMenu(this->createMenu());
}


//**********************************************************************************************************************
// 
//**********************************************************************************************************************
void GroupListWidget::setupContextMenu()
{
   if (contextMenu_)
      contextMenu_->deleteLater();
   contextMenu_ = createMenu();
   ui_.listGroup->setContextMenuPolicy(Qt::CustomContextMenu);
   connect(ui_.listGroup, &QListView::customContextMenuRequested, this, &GroupListWidget::onContextMenuRequested);
}


//**********************************************************************************************************************
/// \param[in] event The event
//**********************************************************************************************************************
void GroupListWidget::changeEvent(QEvent *event)
{
   if (QEvent::LanguageChange == event->type())
      ui_.retranslateUi(this);
   QWidget::changeEvent(event);
}


//**********************************************************************************************************************
/// \return The index of the selected group
/// \return -1 if no group is selected
//**********************************************************************************************************************
qint32 GroupListWidget::selectedGroupIndex() const
{
   QItemSelectionModel* model = ui_.listGroup->selectionModel();
   if (!model)
      return -1;
   QModelIndexList const selectedIndexes = model->selectedIndexes();
   return selectedIndexes.isEmpty() ? -1 : selectedIndexes.front().row();
}


//**********************************************************************************************************************
/// This event filter implement a non default behavior for the Qt QListView single selection system. When clicking
/// on a selected item or an empty area, the selection is cleared.
/// 
/// \param[in] object The object
/// \param[in] event The event
//**********************************************************************************************************************
bool GroupListWidget::eventFilter(QObject *object, QEvent *event)
{
   if (event->type() == QEvent::DragEnter)
   {
      QDropEvent* dropEvent = dynamic_cast<QDropEvent*>(event);
      if (dropEvent)
      {
         QMimeData const* mimeData = dropEvent->mimeData();
         ComboManager::instance().groupListRef().setDropType(mimeData && mimeData->hasFormat(kUuuidListMimeType) ? 
            GroupList::ComboDrop : GroupList::GroupDrop);
      }
   }
   if (event->type() != QEvent::MouseButtonPress)
      return QObject::eventFilter(object, event);
   QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>(event);
   if (!mouseEvent)
      throw xmilib::Exception(QString("Internal error: %1(): could not retrieve mouse event.").arg(__FUNCTION__));
   QItemSelectionModel* selectionModel = ui_.listGroup->selectionModel();
   if (!selectionModel)
      throw xmilib::Exception(QString("Internal error: %1(): could not retrieve selection model.").arg(__FUNCTION__));
   QPoint const mousePos = mouseEvent->pos();
   QModelIndex const item = ui_.listGroup->indexAt(mousePos);
   if (!item.isValid())
   {
      clearSelection();
      return true;
   }
   return QObject::eventFilter(object, event);
}


//**********************************************************************************************************************
/// \return The menu
//**********************************************************************************************************************
QMenu* GroupListWidget::createMenu()
{
   QMenu* menu = new QMenu(this);
   menu->addAction(ui_.actionNewGroup);
   menu->addAction(ui_.actionEditGroup);
   menu->addSeparator();
   menu->addAction(ui_.actionDeleteGroup);
   return menu;
}


//**********************************************************************************************************************
// 
//**********************************************************************************************************************
void GroupListWidget::onActionNewGroup()
{
   try
   {
      SPGroup group = Group::create(QString());
      if (!GroupDialog::run(group, tr("New Group")))
         return;
      ComboManager& comboManager = ComboManager::instance();
      GroupList& groups = comboManager.groupListRef();
      if (!groups.append(group))
         throw xmilib::Exception(tr("The group could not be added to the list."));
      QString errorMessage;
      if (!comboManager.saveComboListToFile(&errorMessage))
         throw xmilib::Exception(errorMessage);
   }
   catch (xmilib::Exception const& e)
   {
      QMessageBox::critical(this, tr("Error"), e.qwhat());
   }
}


//**********************************************************************************************************************
// 
//**********************************************************************************************************************
void GroupListWidget::onActionEditGroup()
{
   try
   {
      ComboManager& comboManager = ComboManager::instance();
      GroupList& groups = comboManager.groupListRef();
      quint32 const index = this->selectedGroupIndex();
      if ((index < 0) || (index >= groups.size()))
         return;
      SPGroup group = groups[index];
      if (!GroupDialog::run(group, tr("Edit Group")))
         return;
      QString errorMessage;
      if (!comboManager.saveComboListToFile(&errorMessage))
         throw xmilib::Exception(errorMessage);
      this->updateGui();
   }
   catch (xmilib::Exception const& e)
   {
      QMessageBox::critical(this, tr("Error"), e.qwhat());
   }
}


//**********************************************************************************************************************
// 
//**********************************************************************************************************************
void GroupListWidget::onActionDeleteGroup()
{
   try
   {
      if (QMessageBox::Yes != QMessageBox::question(this, tr("Delete Group?"), tr("Are you sure you want to delete "
         "the selected group and all its combos?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No))
         return;
      ComboManager& comboManager = ComboManager::instance();
      GroupList& groups = comboManager.groupListRef();
      qint32 const index = this->selectedGroupIndex();
      if ((groups.size() <= 1) ||(index < 0) || (index >= groups.size()))
         return;
      comboManager.comboListRef().eraseCombosOfGroup(groups[index]);
      groups.erase(index);
      comboManager.saveComboListToFile();
      this->updateGui();
   }
   catch (xmilib::Exception const& e)
   {
      QMessageBox::critical(this, tr("Error"), e.qwhat());
   }
}


//**********************************************************************************************************************
/// \param[in] selected The items that got selected
/// \param[in] deselected The items that got deselected
//**********************************************************************************************************************
void GroupListWidget::onSelectionChanged(QItemSelection const& selected, QItemSelection const& deselected)
{
   this->updateGui();
   GroupList& groups = ComboManager::instance().groupListRef();
   qint32 index = this->selectedGroupIndex();
   emit selectedGroupChanged(((index < 0) || (index >= groups.size())) ? nullptr : groups[index]);

}


//**********************************************************************************************************************
/// \param[in] group The group
/// \param[in] newIndex The new index of the group in the list
//**********************************************************************************************************************
void GroupListWidget::onGroupMoved(SPGroup group, qint32 newIndex)
{
   if (!group)
      return;
   ComboManager& comboManager = ComboManager::instance();
   ui_.listGroup->setCurrentIndex(comboManager.groupListRef().index(newIndex));
   comboManager.saveComboListToFile();
}


//**********************************************************************************************************************
// 
//**********************************************************************************************************************
void GroupListWidget::onContextMenuRequested()
{
   if (contextMenu_)
      contextMenu_->exec(QCursor::pos());
}


