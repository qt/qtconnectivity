# ![0]
find_package(Qt6 REQUIRED COMPONENTS Bluetooth)
target_link_libraries(mytarget PRIVATE Qt6::Bluetooth)
# ![0]
