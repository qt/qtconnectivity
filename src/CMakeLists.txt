# Generated from src.pro.

add_subdirectory(bluetooth)
add_subdirectory(nfc)
if(ANDROID)
    add_subdirectory(android)
endif()
if(TARGET Qt::Quick)
    add_subdirectory(imports)
endif()
