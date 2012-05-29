/************************************************************************************
 *   Copyright (C) 2009 by Savoir-Faire Linux                                       *
 *   Author : Jérémy Quentin <jeremy.quentin@savoirfairelinux.com>                  *
 *            Emmanuel Lepage Vallee <emmanuel.lepage@savoirfairelinux.com>         *
 *                                                                                  *
 *   This library is free software; you can redistribute it and/or                  *
 *   modify it under the terms of the GNU Lesser General Public                     *
 *   License as published by the Free Software Foundation; either                   *
 *   version 2.1 of the License, or (at your option) any later version.             *
 *                                                                                  *
 *   This library is distributed in the hope that it will be useful,                *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of                 *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU              *
 *   Lesser General Public License for more details.                                *
 *                                                                                  *
 *   You should have received a copy of the GNU Lesser General Public               *
 *   License along with this library; if not, write to the Free Software            *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA *
 ***********************************************************************************/

//Parent
#include <CallModel.h>

bool CallModelBase::dbusInit = false;
CallMap CallModelBase::m_sActiveCalls;

CallModelBase::CallModelBase(QObject* parent) : QObject(parent)
{
   if (!dbusInit) {
      CallManagerInterface& callManager = CallManagerInterfaceSingleton::getInstance();

      //SLOTS
      //             SENDER                                        SIGNAL                                      RECEIVER                             SLOT                                    /
      /**/connect(&callManager, SIGNAL( callStateChanged  (const QString &, const QString &                  ) ), this , SLOT( on1_callStateChanged  ( const QString &, const QString & ) ) );
      /**/connect(&callManager, SIGNAL( incomingCall      (const QString &, const QString &, const QString & ) ), this , SLOT( on1_incomingCall      ( const QString &, const QString & ) ) );
      /**/connect(&callManager, SIGNAL( conferenceCreated (const QString &                                   ) ), this , SLOT( on1_incomingConference( const QString &                  ) ) );
      /**/connect(&callManager, SIGNAL( conferenceChanged (const QString &, const QString &                  ) ), this , SLOT( on1_changingConference( const QString &, const QString & ) ) );
      /**/connect(&callManager, SIGNAL( conferenceRemoved (const QString &                                   ) ), this , SLOT( on1_conferenceRemoved ( const QString &                  ) ) );
      /**/connect(&callManager, SIGNAL( voiceMailNotify   (const QString &, int                              ) ), this , SLOT( on1_voiceMailNotify   ( const QString &, int             ) ) );
      /**/connect(&callManager, SIGNAL( volumeChanged     (const QString &, double                           ) ), this , SLOT( on1_volumeChanged     ( const QString &, double          ) ) );
      /*                                                                                                                                                                                    */
      dbusInit = true;
   }
}

void CallModelBase::on1_callStateChanged(const QString &callID, const QString &state)
{
   //This code is part of the CallModel iterface too
   qDebug() << "Signal : Call State Changed for call  " << callID << " . New state : " << state;
   Call* call = findCallByCallId(callID);
   if(!call) {
      qDebug() << "Call not found";
      if(state == CALL_STATE_CHANGE_RINGING) {
         call = addRingingCall(callID);
      }
      else {
         qDebug() << "Call doesn't exist in this client. Might have been initialized by another client instance before this one started.";
         return;
      }
   }
   else {
      qDebug() << "Call found" << call;
      call->stateChanged(state);
   }

   if (call->getCurrentState() == CALL_STATE_OVER) {
      addToHistory(call);
      emit historyChanged();
   }
   
   emit callStateChanged(call);
   
}

void CallModelBase::on1_incomingCall(const QString & accountID, const QString & callID)
{
   Q_UNUSED(accountID)
   qDebug() << "Signal : Incoming Call ! ID = " << callID;
   Call* call = addIncomingCall(callID);

   //NEED_PORT
//    SFLPhone::app()->activateWindow();
//    SFLPhone::app()->raise();
//    SFLPhone::app()->setVisible(true);

   //emit incomingCall(call);
   emit incomingCall(call);
}

void CallModelBase::on1_incomingConference(const QString &confID)
{
   Call* conf = addConference(confID);
   qDebug() << "---------------Adding conference" << conf << confID << "---------------";
   emit conferenceCreated(conf);
}

void CallModelBase::on1_changingConference(const QString &confID, const QString &state)
{
   Call* conf = getCall(confID);
   qDebug() << "Changing conference state" << conf << confID;
   if (conf && dynamic_cast<Call*>(conf)) { //Prevent a race condition between call and conference
      changeConference(confID, state);
      emit conferenceChanged(conf);
   }
   else {
      qDebug() << "Trying to affect a conference that does not exist (anymore)";
   }
}

void CallModelBase::on1_conferenceRemoved(const QString &confId)
{
   Call* conf = getCall(confId);
   emit aboutToRemoveConference(conf);
   removeConference(confId);
   emit conferenceRemoved(confId);
}

void CallModelBase::on1_voiceMailNotify(const QString &accountID, int count)
{
   qDebug() << "Signal : VoiceMail Notify ! " << count << " new voice mails for account " << accountID;
   emit voiceMailNotify(accountID,count);
}

void CallModelBase::on1_volumeChanged(const QString & device, double value)
{
   emit volumeChanged(device,value);
}

Call* CallModelBase::addCall(Call* call, Call* parent)
{
   emit callAdded(call,parent);

   connect(call, SIGNAL(isOver(Call*)), this, SLOT(removeActiveCall(Call*)));
   return call;
}

Call* CallModelBase::addConferenceS(Call* conf)
{
   emit conferenceCreated(conf);
   return conf;
}

///Remove it from active calls
void CallModelBase::removeActiveCall(Call* call)
{
   Q_UNUSED(call);
   //There is a race condition
   //m_sActiveCalls[call->getCallId()] = nullptr;
}

//More code in CallModel.hpp