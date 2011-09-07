/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the test suite of the Qt Toolkit.
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

#define QT_NO_KEYWORDS
#define signals int
#define slots int
#define emit public:;
#define foreach public:;
#define forever public:;

//include all public headers and ensure that they compile with CONFIG+=no_keywords
//this is a bit messy as we have to manually mention every header

//QtBearer
#include <qnetworkconfiguration.h>
#include <qnetworkconfigmanager.h>
#include <qnetworksession.h>

//QtContacts
#include <qcontactabstractrequest.h>
#include <details/qcontactnickname.h>
#include <details/qcontactgeolocation.h>
#include <details/qcontacttype.h>
#include <details/qcontactanniversary.h>
#include <details/qcontactname.h>
#include <details/qcontacturl.h>
#include <details/qcontacttag.h>
#include <details/qcontactonlineaccount.h>
#include <details/qcontacttimestamp.h>
#include <details/qcontactglobalpresence.h>
#include <details/qcontactguid.h>
#include <details/qcontactaddress.h>
#include <details/qcontactphonenumber.h>
#include <details/qcontactthumbnail.h>
#include <details/qcontactsynctarget.h>
#include <details/qcontactgender.h>
#include <details/qcontactdetails.h>
#include <details/qcontactbirthday.h>
#include <details/qcontactfamily.h>
#include <details/qcontactpresence.h>
#include <details/qcontactavatar.h>
#include <details/qcontactnote.h>
#include <details/qcontactorganization.h>
#include <details/qcontactemailaddress.h>
#include <details/qcontactdisplaylabel.h>
#include <details/qcontactringtone.h>
#include <qcontactsortorder.h>
#include <filters/qcontactunionfilter.h>
#include <filters/qcontactintersectionfilter.h>
#include <filters/qcontactinvalidfilter.h>
#include <filters/qcontactdetailfilter.h>
#include <filters/qcontactdetailrangefilter.h>
#include <filters/qcontactchangelogfilter.h>
#include <filters/qcontactrelationshipfilter.h>
#include <filters/qcontactfilters.h>
#include <filters/qcontactlocalidfilter.h>
#include <filters/qcontactactionfilter.h>
#include <qcontactdetaildefinition.h>
#include <qcontactmanagerenginefactory.h>
#include <qtcontacts.h>
#include <qcontactdetailfielddefinition.h>
#include <qtcontactsglobal.h>
#include <qcontactactionfactory.h>
#include <qcontactid.h>
#include <qcontactfilter.h>
#include <qcontactaction.h>
#include <qcontactchangeset.h>
#include <qcontactmanagerengine.h>
#include <requests/qcontactrelationshipremoverequest.h>
#include <requests/qcontactdetaildefinitionremoverequest.h>
#include <requests/qcontactlocalidfetchrequest.h>
#include <requests/qcontactrequests.h>
#include <requests/qcontactremoverequest.h>
#include <requests/qcontactsaverequest.h>
#include <requests/qcontactrelationshipfetchrequest.h>
#include <requests/qcontactfetchrequest.h>
#include <requests/qcontactrelationshipsaverequest.h>
#include <requests/qcontactdetaildefinitionfetchrequest.h>
#include <requests/qcontactdetaildefinitionsaverequest.h>
#include <qcontactrelationship.h>
#include <qcontactdetail.h>
#include <qcontact.h>


//QtFeedback
#include <qfeedbackeffect.h>
#include <qfeedbackplugininterfaces.h>
#include <qfeedbackactuator.h>

//QtGallery
#include <qgallerytype.h>
#include <qgalleryquerymodel.h>
#include <qgallerytyperequest.h>
#include <qgalleryfilter.h>
#include <qdocumentgallery.h>
#include <qgalleryabstractresponse.h>
#include <qgalleryitemrequest.h>
#include <qgalleryabstractrequest.h>
#include <qabstractgallery.h>
#include <qgalleryqueryrequest.h>
#include <qgalleryresource.h>
#include <qgalleryresultset.h>
#include <qgalleryproperty.h>

//QtLocation
#include <qnmeapositioninfosource.h>
#include <qgeoplace.h>
#include <qgeocoordinate.h>
#include <qgeopositioninfo.h>
#include <qgeoaddress.h>
#include <maps/qgeoroutesegment.h>
#include <maps/qgeoserviceproviderfactory.h>
#include <maps/qgeomappingmanagerengine.h>
#include <maps/qgeoroutingmanager.h>
#include <maps/qgeomapcircleobject.h>
#include <maps/qgeoroutereply.h>
#include <maps/qgeoserviceprovider.h>
#include <maps/qgeosearchmanager.h>
#include <maps/qgeomaprouteobject.h>
//#include <maps/qgeonavigationinstruction.h>
#include <maps/qgeomapdata.h>
#include <maps/qgeoroutingmanagerengine.h>
#include <maps/qgeomappolygonobject.h>
//#include <maps/qgeonavigator.h>
#include <maps/qgeoroute.h>
#include <maps/qgeomappolylineobject.h>
#include <maps/qgeomappingmanager.h>
//#include <maps/qgeomapmarkerobject.h>
#include <maps/qgeosearchreply.h>
#include <maps/tiled/qgeotiledmapreply.h>
#include <maps/tiled/qgeotiledmapdata.h>
#include <maps/tiled/qgeotiledmappingmanagerengine.h>
#include <maps/tiled/qgeotiledmaprequest.h>
#include <maps/qgeomaprectangleobject.h>
#include <maps/qgraphicsgeomap.h>
#include <maps/qgeomapobject.h>
#include <maps/qgeorouterequest.h>
#include <maps/qgeosearchmanagerengine.h>
#include <qgeoareamonitor.h>
#include <landmarks/qlandmarksortorder.h>
//#include <landmarks/qlandmarkfreetextfilter.h>
#include <landmarks/qlandmarkabstractrequest.h>
#include <landmarks/qlandmarkproximityfilter.h>
#include <landmarks/qlandmarkmanager.h>
#include <landmarks/qlandmarkexportrequest.h>
#include <landmarks/qlandmark.h>
//#include <landmarks/qlandmarkfetchhint.h>
#include <landmarks/qlandmarknamesort.h>
#include <landmarks/qlandmarkcategorysaverequest.h>
#include <landmarks/qlandmarkremoverequest.h>
#include <landmarks/qlandmarksaverequest.h>
#include <landmarks/qlandmarknamefilter.h>
#include <landmarks/qlandmarkcategoryfilter.h>
#include <landmarks/qlandmarkmanagerenginefactory.h>
#include <landmarks/qlandmarkcategoryid.h>
#include <landmarks/qlandmarkcategoryremoverequest.h>
#include <landmarks/qlandmarkunionfilter.h>
#include <landmarks/qlandmarkintersectionfilter.h>
#include <landmarks/qlandmarkidfilter.h>
//#include <landmarks/qlandmarkdistancesort.h>
#include <landmarks/qlandmarkmanagerengine.h>
#include <landmarks/qlandmarkcategory.h>
#include <landmarks/qlandmarkimportrequest.h>
#include <landmarks/qlandmarkattributefilter.h>
#include <landmarks/qlandmarkfetchrequest.h>
#include <landmarks/qlandmarkidfetchrequest.h>
#include <landmarks/qlandmarkfilter.h>
#include <landmarks/qlandmarkid.h>
#include <landmarks/qlandmarkboxfilter.h>
#include <landmarks/qlandmarkcategoryfetchrequest.h>
#include <landmarks/qlandmarkcategoryidfetchrequest.h>
#include <qgeoboundingbox.h>
#include <qgeosatelliteinfo.h>
#include <qgeopositioninfosource.h>
#include <qgeosatelliteinfosource.h>

//QtMessaging
#include <qmessagefolder.h>
#include <qmessagedatacomparator.h>
#include <qmessage.h>
#include <qmessageaddress.h>
#include <qmessageaccountid.h>
#include <qmessageglobal.h>
#include <qmessagefolderfilter.h>
#include <qmessageaccountfilter.h>
#include <qmessagefilter.h>
#include <qmessageservice.h>
#include <qmessagefoldersortorder.h>
#include <qmessageaccountsortorder.h>
#include <qmessagecontentcontainerid.h>
#include <qmessageaccount.h>
#include <qmessagefolderid.h>
#include <qmessagecontentcontainer.h>
#include <qmessagemanager.h>
#include <qmessagestore_p.h>
#include <qmessagesortorder.h>
#include <qmessageid.h>

//QtMultimediaKit
#include <qmediarecorder.h>
#include <qcameracontrol.h>
#include <qmediaplaylistcontrol.h>
#include <qmediaplaylistioplugin.h>
#include <qcameraimagecapture.h>
#include <qcameraflashcontrol.h>
#include <qmetadatawritercontrol.h>
#include <qlocalmediaplaylistprovider.h>
#include <qmediacontainercontrol.h>
#include <qradiotunercontrol.h>
#include <qmediaserviceproviderplugin.h>
#include <qmediatimerange.h>
#include <qmediabindableinterface.h>
#include <qvideodevicecontrol.h>
#include <qmediaplaylistprovider.h>
#include <qmediaobject.h>
#include <video/qabstractvideosurface.h>
#include <video/qabstractvideobuffer.h>
#include <video/qvideosurfaceformat.h>
#include <video/qvideoframe.h>
#include <qvideowindowcontrol.h>
#include <qmediastreamscontrol.h>
#include <qcameralockscontrol.h>
#include <qmediacontrol.h>
#include <qmediaplayercontrol.h>
#include <qmediaserviceprovider.h>
#include <qcameraimagecapturecontrol.h>
#include <qcameraexposure.h>
#include <qcameraviewfinder.h>
#include <qaudioencodercontrol.h>
#include <audio/qaudioformat.h>
#include <audio/qaudiosystem.h>
#include <audio/qaudioinput.h>
#include <audio/qaudiosystemplugin.h>
#include <audio/qaudiodeviceinfo.h>
#include <audio/qaudio.h>
#include <audio/qaudiooutput.h>
#include <qmediarecordercontrol.h>
#include <qcamera.h>
#include <qvideowidget.h>
#include <qcameraexposurecontrol.h>
#include <qradiotuner.h>
#include <qaudiocapturesource.h>
#include <qmediaplaylistnavigator.h>
#include <qcamerafocuscontrol.h>
// #include <effects/qsoundeffect.h>
#include <qmediacontent.h>
#include <qmediaresource.h>
#include <qtmedianamespace.h>
#include <qvideowidgetcontrol.h>
#include <qcameraimageprocessing.h>
#include <qmediaencodersettings.h>
#include <qcamerafocus.h>
#include <qcameraimageprocessingcontrol.h>
#include <qvideorenderercontrol.h>
#include <qmediaplaylistsourcecontrol.h>
#include <qmediaplayer.h>
#include <qgraphicsvideoitem.h>
#include <qimageencodercontrol.h>
#include <qvideoencodercontrol.h>
#include <qaudioendpointselector.h>
#include <qmetadatareadercontrol.h>
#include <qmediaplaylist.h>
#include <qmediaservice.h>
#include <qmediaimageviewer.h>

//QtOrganizer
#include <qorganizeritemrecurrencerule.h>
#include <details/qorganizereventtimerange.h>
#include <details/qorganizeritemtimestamp.h>
#include <details/qorganizertodoprogress.h>
#include <details/qorganizeritemrecurrence.h>
#include <details/qorganizeritemdisplaylabel.h>
#include <details/qorganizerjournaltimerange.h>
#include <details/qorganizeritemguid.h>
#include <details/qorganizeritemlocation.h>
#include <details/qorganizeritemcomment.h>
#include <details/qorganizertodotimerange.h>
#include <details/qorganizeritemtype.h>
#include <details/qorganizeritemdetails.h>
#include <details/qorganizeritempriority.h>
#include <details/qorganizeritemdescription.h>
#include <details/qorganizeriteminstanceorigin.h>
#include <qorganizeritemmanagerengine.h>
#include <qorganizeritemabstractrequest.h>
#include <qtorganizerglobal.h>
#include <filters/qorganizeritemdatetimeperiodfilter.h>
#include <filters/qorganizeritemdetailrangefilter.h>
#include <filters/qorganizeritemlocalidfilter.h>
#include <filters/qorganizeriteminvalidfilter.h>
#include <filters/qorganizeritemintersectionfilter.h>
#include <filters/qorganizeritemunionfilter.h>
#include <filters/qorganizeritemdetailfilter.h>
#include <filters/qorganizeritemfilters.h>
#include <filters/qorganizeritemchangelogfilter.h>
#include <qorganizeritemdetailfielddefinition.h>
#include <qtorganizer.h>
#include <qorganizeritemmanagerenginefactory.h>
#include <qorganizeritemdetaildefinition.h>
#include <qorganizeritemfetchhint.h>
#include <qorganizeritemsortorder.h>
#include <qorganizeritemchangeset.h>
#include <requests/qorganizeritemsaverequest.h>
#include <requests/qorganizeritemdetaildefinitionsaverequest.h>
#include <requests/qorganizeritemremoverequest.h>
#include <requests/qorganizeriteminstancefetchrequest.h>
#include <requests/qorganizeritemdetaildefinitionremoverequest.h>
#include <requests/qorganizeritemrequests.h>
#include <requests/qorganizeritemfetchrequest.h>
#include <requests/qorganizeritemlocalidfetchrequest.h>
#include <requests/qorganizeritemdetaildefinitionfetchrequest.h>
#include <qorganizeritemdetail.h>
#include <qorganizeritemid.h>
#include <qorganizeritem.h>
#include <qorganizeritemmanager.h>
#include <qorganizeritemfilter.h>
#include <items/qorganizerevent.h>
#include <items/qorganizereventoccurrence.h>
#include <items/qorganizertodooccurrence.h>
#include <items/qorganizeritems.h>
#include <items/qorganizernote.h>
#include <items/qorganizertodo.h>
#include <items/qorganizerjournal.h>

//QtPublishSubscribe
#include <qvaluespace.h>
#include <qvaluespacepublisher.h>
#include <qvaluespacesubscriber.h>

//QtSensors
#include <qsensorplugin.h>
#include <qsensormanager.h>
#include <qaccelerometer.h>
#include <qrotationsensor.h>
#include <qorientationsensor.h>
#include <qsensorbackend.h>
#include <qsensor.h>
#include <qcompass.h>
#include <qtapsensor.h>
#include <qmagnetometer.h>
#include <qambientlightsensor.h>
#include <qproximitysensor.h>

//QtServiceFramework
#include <qservicecontext.h>
#include <qabstractsecuritysession.h>
#include <qserviceplugininterface.h>
#include <qservicefilter.h>
#include <qservice.h>
#include <qserviceinterfacedescriptor.h>
#include <qremoteserviceregister.h>
#include <qservicemanager.h>

//QtSystemInfo
#include <qsystemnetworkinfo.h>
#include <qsysteminfo.h>
#include <qsystemgeneralinfo.h>
#include <qsystemdisplayinfo.h>
#include <qsystemdeviceinfo.h>
#include <qsystemscreensaver.h>
#include <qsystemstorageinfo.h>

//QtVersit
#include <qversitcontacthandler.h>
#include <qversitproperty.h>
#include <qversitcontactexporter.h>
#include <qversitcontactimporter.h>
#include <qversitwriter.h>
#include <qversitreader.h>
#include <qversitdocument.h>
#include <qversitresourcehandler.h>

//QtVersitOrganizer
#include <qversitorganizerimporter.h>
#include <qversitorganizerexporter.h>
