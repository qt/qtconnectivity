# Generated from nfc.pro.

if(TARGET Qt::Widgets)
    add_subdirectory(annotatedurl)
    add_subdirectory(ndefeditor)
endif()
if(TARGET Qt::Quick)
    add_subdirectory(poster)
    add_subdirectory(corkboard)
endif()
