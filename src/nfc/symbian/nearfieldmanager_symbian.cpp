/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt Mobility Components.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "nearfieldmanager_symbian.h"
#include "nearfieldtargetfactory_symbian.h"
#include "../qnearfieldmanager_symbian_p.h"
#include "nearfieldutility_symbian.h"

#include <ndefmessage.h>
#include "debug.h"

/*
    \class CNearFieldManager
    \brief The CNearFieldManager class provides symbian backend implementation to access NFC service.

    \ingroup connectivity-nfc
    \inmodule QtConnectivity
    \since 5.0
    \internal

    A Symbian implementation class to support symbian NFC backend.
*/

/*
    Constructs a CNearFieldManager.
*/
void CNearFieldManager::ConstructL()
    {
    BEGIN
    User::LeaveIfError(iServer.Open());
    END
    }

/*
    Start listening all type tags.
*/
void CNearFieldManager::StartTargetDetectionL(const QList<QNearFieldTarget::Type> &aTargetTypes)
    {
    BEGIN
    if (aTargetTypes.size() > 0)
        {
        if (!iNfcTagDiscovery)
            {
            iNfcTagDiscovery = CNfcTagDiscovery::NewL( iServer );
            User::LeaveIfError(iNfcTagDiscovery->AddTagConnectionListener( *this ));
            }
        else
            {
            iNfcTagDiscovery->RemoveTagSubscription();
            }

        if (!iTagSubscription)
            {
            iTagSubscription = CNfcTagSubscription::NewL();
            }
        else
            {
            iTagSubscription->RemoveAllConnectionModes();
            }
        for (int i = 0; i < aTargetTypes.size(); ++i)
            {
            switch(aTargetTypes[i])
                {
                case QNearFieldTarget::NfcTagType1:
                    iTagSubscription->RemoveConnectionMode(TNfcConnectionInfo::ENfcType1);
                    iTagSubscription->AddConnectionModeL( TNfcConnectionInfo::ENfcType1 );
                    break;
                case QNearFieldTarget::NfcTagType2:
                    iTagSubscription->RemoveConnectionMode(TNfcConnectionInfo::ENfcType2);
                    iTagSubscription->AddConnectionModeL( TNfcConnectionInfo::ENfcType2 );
                    break;
                case QNearFieldTarget::NfcTagType3:
                    iTagSubscription->RemoveConnectionMode(TNfcConnectionInfo::ENfcType3);
                    iTagSubscription->AddConnectionModeL( TNfcConnectionInfo::ENfcType3 );
                    break;
                case QNearFieldTarget::NfcTagType4:
                    iTagSubscription->RemoveConnectionMode(TNfcConnectionInfo::ENfc14443P4);
                    iTagSubscription->AddConnectionModeL( TNfcConnectionInfo::ENfc14443P4 );
                    break;
                case QNearFieldTarget::MifareTag:
                    iTagSubscription->RemoveConnectionMode(TNfcConnectionInfo::ENfcMifareStd);
                    iTagSubscription->AddConnectionModeL( TNfcConnectionInfo::ENfcMifareStd );
                    break;
                case QNearFieldTarget::AnyTarget:
                    iTagSubscription->RemoveAllConnectionModes();
                    iTagSubscription->AddConnectionModeL( TNfcConnectionInfo::ENfcType1 );
                    iTagSubscription->AddConnectionModeL( TNfcConnectionInfo::ENfcType2 );
                    iTagSubscription->AddConnectionModeL( TNfcConnectionInfo::ENfcType3 );
                    iTagSubscription->AddConnectionModeL( TNfcConnectionInfo::ENfc14443P4 );
                    iTagSubscription->AddConnectionModeL( TNfcConnectionInfo::ENfcMifareStd );
                    if (!iLlcpProvider)
                        {
                        //create LLCP provider api
                        iLlcpProvider = CLlcpProvider::NewL( iServer );
                        iLlcpProvider->AddLlcpLinkListenerL( *this );
                        }
                    break;
                case QNearFieldTarget::ProprietaryTag:
                    //No conterpart in symbian api
                    break;
                case QNearFieldTarget::NfcForumDevice:
                    if (!iLlcpProvider)
                        {
                        //create LLCP provider api
                        iLlcpProvider = CLlcpProvider::NewL( iServer );
                        iLlcpProvider->AddLlcpLinkListenerL( *this );
                        }
                    break;
                default:
                    break;
                }
            }
        }
    iNfcTagDiscovery->AddTagSubscriptionL( *iTagSubscription );
    END
    }

/*
    Stop listening all type tags.
*/
void CNearFieldManager::stopTargetDetection()
    {
    BEGIN
    if (iNfcTagDiscovery)
        {
        iNfcTagDiscovery->RemoveTagConnectionListener();
        iNfcTagDiscovery->RemoveTagSubscription();

        if (iTagSubscription)
            {
            delete iTagSubscription;
            iTagSubscription = NULL;
            }
        delete iNfcTagDiscovery;
        iNfcTagDiscovery = NULL;
        }
    if (iLlcpProvider)
        {
        iLlcpProvider->RemoveLlcpLinkListener();
        delete iLlcpProvider;
        iLlcpProvider = NULL;
        }

    END
    }

CNdefRecord::TNdefRecordTnf CNearFieldManager::QTnf2CTnf(const QNdefRecord::TypeNameFormat aQTnf)
    {
    CNdefRecord::TNdefRecordTnf ret = CNdefRecord::EEmpty;
    switch(aQTnf)
        {
        case QNdefRecord::Empty:
            break;
        case QNdefRecord::NfcRtd:
            ret = CNdefRecord::ENfcWellKnown;
            break;
        case QNdefRecord::Mime:
            ret = CNdefRecord::EMime;
            break;
        case QNdefRecord::Uri:
            ret = CNdefRecord::EUri;
            break;
        case QNdefRecord::ExternalRtd:
            ret = CNdefRecord::ENfcExternal;
            break;
        case QNdefRecord::Unknown:
            ret = CNdefRecord::EUnknown;
            break;
        default:
            break;
        }
    return ret;
    }

/*
    Register interested TNF NDEF message to NFC server.
*/
TInt CNearFieldManager::AddNdefSubscription( const QNdefRecord::TypeNameFormat aTnf,
                                       const QByteArray& aType )
    {
    BEGIN
    TInt err = KErrNone;
    if ( !iNdefDiscovery )
        {
        TRAP(err, iNdefDiscovery = CNdefDiscovery::NewL( iServer ));
        if (err != KErrNone)
            {
            END
            return err;
            }
        err = iNdefDiscovery->AddNdefMessageListener( *this );
        if (err != KErrNone)
            {
            END
            return err;
            }

        }
    TPtrC8 type(QNFCNdefUtility::QByteArray2TPtrC8(aType));
    err = iNdefDiscovery->AddNdefSubscription( QTnf2CTnf(aTnf), type );
    END
    return err;
    }

/*
    Unregister interested TNF NDEF message to NFC server.
*/
void CNearFieldManager::RemoveNdefSubscription( const QNdefRecord::TypeNameFormat aTnf,
                                          const QByteArray& aType )
    {
    BEGIN
    if ( iNdefDiscovery )
        {
        TPtrC8 type(QNFCNdefUtility::QByteArray2TPtrC8(aType));
        iNdefDiscovery->RemoveNdefSubscription( QTnf2CTnf(aTnf), type );
        }
    END
    }

/*
    Callback function when the tag found by NFC symbain services.
*/
void CNearFieldManager::TagDetected( MNfcTag* aNfcTag )
    {
    BEGIN
    if (aNfcTag)
        {
        QNearFieldTarget* tag = TNearFieldTargetFactory::CreateTargetL(aNfcTag, iServer, &iCallback);
        TInt error = KErrNone;
        QT_TRYCATCH_ERROR(error, iCallback.targetFound(tag));
        Q_UNUSED(error);//just skip the error
        }
    END
    }

/*
    Callback function when the tag lost event found by NFC symbain services.
*/
void CNearFieldManager::TagLost()
    {
    BEGIN
    TInt error = KErrNone;
    QT_TRYCATCH_ERROR(error, iCallback.targetDisconnected());//just skip the error
    Q_UNUSED(error);//just skip the error
    END
    }

/*
    Callback function when the LLCP peer found by NFC symbain services.
*/
void CNearFieldManager::LlcpRemoteFound()
    {
    BEGIN
    TInt error = KErrNone;
    QNearFieldTarget* tag = NULL;
    TRAP(error, tag = TNearFieldTargetFactory::CreateTargetL(NULL, iServer, &iCallback));
    if (error == KErrNone)
        {
        QT_TRYCATCH_ERROR(error, iCallback.targetFound(tag) );
        Q_UNUSED(error);//just skip the error
        }
    END
    }

/*
    Callback function when the LLCP peer lost event found by NFC symbain services.
*/
void CNearFieldManager::LlcpRemoteLost()
    {
    BEGIN
    TInt error = KErrNone;
    QT_TRYCATCH_ERROR(error, iCallback.targetDisconnected());
    Q_UNUSED(error);//just skip the error
    END
    }

/*
    Callback function when the registerd NDEF message found by NFC symbain services.
*/
void CNearFieldManager::MessageDetected( CNdefMessage* aMessage )
    {
    BEGIN
    if ( aMessage )
        {
        TInt error = KErrNone;
        QNdefMessage msg;
        TRAP(error, msg = QNFCNdefUtility::CNdefMsg2QNdefMsgL( *aMessage));
        if (error == KErrNone)
            {
            QT_TRYCATCH_ERROR(error, iCallback.invokeNdefMessageHandler(msg));
            Q_UNUSED(error);//just skip the error
            }
        delete aMessage;
        }
    END
    }

/*
    New a CNearFieldManager instance.
*/
CNearFieldManager::CNearFieldManager( QNearFieldManagerPrivateImpl& aCallback)
    : iCallback(aCallback)
    {
    }

/*
    Create a new instance of this class.
*/
CNearFieldManager* CNearFieldManager::NewL( QNearFieldManagerPrivateImpl& aCallback)
    {
    BEGIN
    CNearFieldManager* self = new (ELeave) CNearFieldManager(aCallback);
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    END
    return self;
    }

/*
    Destructor.
*/
CNearFieldManager::~CNearFieldManager()
    {
    BEGIN
    if ( iNfcTagDiscovery )
        {
        iNfcTagDiscovery->RemoveTagConnectionListener();
        iNfcTagDiscovery->RemoveTagSubscription();
        }

    delete iTagSubscription;
    delete iNfcTagDiscovery;

    if (iLlcpProvider)
        {
        iLlcpProvider->RemoveLlcpLinkListener();
        }
    delete iLlcpProvider;

    if (iNdefDiscovery)
        {
        iNdefDiscovery->RemoveAllNdefSubscription();
        }
    delete iNdefDiscovery;

    iServer.Close();
    END
    }
//EOF
