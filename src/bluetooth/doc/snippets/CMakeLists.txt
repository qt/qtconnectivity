# ![0]
find_package(Qt6 COMPONENTS Bluetooth REQUIRED)
target_link_libraries(mytarget PRIVATE Qt6::Bluetooth)
# ![0]
