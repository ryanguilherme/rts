cmake_minimum_required(VERSION 3.12)

# Name project
SET(ProjectName idlehook)

# Set board to the Wireless version
SET(PICO_BOARD pico_w)

# Set any variables required for importing libraries
# SET(FREERTOS_KERNEL_PATH /home/ryan/FreeRTOS-Kernel)

# Import those libraries
include($ENV{FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/pico_sdk_import.cmake)
include($ENV{FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

# Define project
project(${ProjectName} C CXX ASM)

# Initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# Add subdirectories
add_subdirectory(src)

# if you have anything in "lib" folder then uncomment below - remember to add a CMakeLists.txt
# file to the "lib" directory
#add_subdirectory(lib)
