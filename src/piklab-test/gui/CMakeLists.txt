include_directories(${CMAKE_CURRENT_SOURCE_DIR})

SET(build_check_SRCS
   gui_check.cpp
   build_check.cpp
)
automoc(${build_check_SRCS})
add_executable(build_check ${build_check_SRCS})
target_link_libraries(build_check
                      gui toollist
                      cc5xui boostui mpcui ccscui c18ui jalv2ui
                      jalui pic30ui piccui sdccui gputilsui
                      customtool cc5x boost mpc ccsc c18 jalv2 jal pic30 picc sdcc gputils
                      toolui toolbase
                      proglistui customprogui
                      picdembootloaderui pickit2bootloaderui tblbootloaderui
                      bootloaderui gpsimui pspui pickit1ui
                      pickit3ui pickit2v2ui pickit2ui icd1ui icd2ui directui progui
                      customprog
                      picdembootloader pickit2bootloader tblbootloader
                      bootloader gpsim psp pickit1
                      pickit3 pickit3data pickit2v2 pickit2 icd1 icd2 icd2data directprog
                      mem24prog picprog progmanager progbase
                      coff port global
                      devicelistui mem24ui picui deviceui commonui
                      pic picbase picxml mem24 mem24base mem24xml
                      devicebase common
                      ${GUI_LIBS}
)
