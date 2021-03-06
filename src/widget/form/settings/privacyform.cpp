/*
    Copyright (C) 2014 by Project Tox <https://tox.im>

    This file is part of qTox, a Qt-based graphical interface for Tox.

    This program is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    See the COPYING file for more details.
*/

#include "privacyform.h"
#include "ui_privacysettings.h"
#include "src/widget/form/settingswidget.h"
#include "src/misc/settings.h"
#include "src/historykeeper.h"
#include "src/core.h"
#include "src/widget/widget.h"
#include "src/widget/form/setpassworddialog.h"
#include <QMessageBox>

PrivacyForm::PrivacyForm() :
    GenericForm(tr("Privacy"), QPixmap(":/img/settings/privacy.png"))
{
    bodyUI = new Ui::PrivacySettings;
    bodyUI->setupUi(this);

    bodyUI->cbTypingNotification->setChecked(Settings::getInstance().isTypingNotificationEnabled());
    bodyUI->cbKeepHistory->setChecked(Settings::getInstance().getEnableLogging());
    bodyUI->cbEncryptHistory->setChecked(Settings::getInstance().getEncryptLogs());
    bodyUI->cbEncryptHistory->setEnabled(Settings::getInstance().getEnableLogging());
    bodyUI->cbEncryptTox->setChecked(Settings::getInstance().getEncryptTox());

    connect(bodyUI->cbTypingNotification, SIGNAL(stateChanged(int)), this, SLOT(onTypingNotificationEnabledUpdated()));
    connect(bodyUI->cbKeepHistory, SIGNAL(stateChanged(int)), this, SLOT(onEnableLoggingUpdated()));
    connect(bodyUI->cbEncryptHistory, SIGNAL(clicked()), this, SLOT(onEncryptLogsUpdated()));
    connect(bodyUI->cbEncryptTox, SIGNAL(clicked()), this, SLOT(onEncryptToxUpdated()));
}

PrivacyForm::~PrivacyForm()
{
    delete bodyUI;
}

void PrivacyForm::onEnableLoggingUpdated()
{
    Settings::getInstance().setEnableLogging(bodyUI->cbKeepHistory->isChecked());
    bodyUI->cbEncryptHistory->setEnabled(bodyUI->cbKeepHistory->isChecked());
    HistoryKeeper::getInstance()->resetInstance();
    Widget::getInstance()->clearAllReceipts();
}

void PrivacyForm::onTypingNotificationEnabledUpdated()
{
    Settings::getInstance().setTypingNotification(bodyUI->cbTypingNotification->isChecked());
}

void PrivacyForm::onEncryptLogsUpdated()
{
    bool encrytionState = bodyUI->cbEncryptHistory->isChecked();

    if (encrytionState)
    {
        if (!Core::getInstance()->isPasswordSet(Core::ptHistory))
        {
            SetPasswordDialog dialog;
            if (dialog.exec())
            {
                QString pswd = dialog.getPassword();
                if (pswd.size() == 0)
                    encrytionState = false;

                Core::getInstance()->setPassword(pswd, Core::ptHistory);
            } else {
                encrytionState = false;
                Core::getInstance()->clearPassword(Core::ptHistory);
            }
        }
    }

    Settings::getInstance().setEncryptLogs(encrytionState);
    if (encrytionState && !HistoryKeeper::checkPassword())
    {
        if (QMessageBox::Ok != QMessageBox::warning(nullptr, tr("Encrypted log"),
            tr("You already have history log file encrypted with different password\nDo you want to delete old history file?"),
            QMessageBox::Ok | QMessageBox::Cancel))
        {
            // TODO: ask user about reencryption with new password
            encrytionState = false;
        }
    }

    Settings::getInstance().setEncryptLogs(encrytionState);
    bodyUI->cbEncryptHistory->setChecked(encrytionState);

    if (encrytionState)
        HistoryKeeper::resetInstance();

    if (!Settings::getInstance().getEncryptLogs())
        Core::getInstance()->clearPassword(Core::ptHistory);
}

void PrivacyForm::onEncryptToxUpdated()
{
    bool encrytionState = bodyUI->cbEncryptTox->isChecked();

    if (encrytionState)
    {
        if (!Core::getInstance()->isPasswordSet(Core::ptMain))
        {
            SetPasswordDialog dialog;
            if (dialog.exec())
            {
                QString pswd = dialog.getPassword();
                if (pswd.size() == 0)
                    encrytionState = false;

                Core::getInstance()->setPassword(pswd, Core::ptMain);
            } else {
                encrytionState = false;
                Core::getInstance()->clearPassword(Core::ptMain);
            }
        }
    }

    bodyUI->cbEncryptTox->setChecked(encrytionState);
    Settings::getInstance().setEncryptTox(encrytionState);

    if (!Settings::getInstance().getEncryptTox())
        Core::getInstance()->clearPassword(Core::ptMain);
}
