QT_FOR_CONFIG += 3dcore-private
!tools-only:qtConfig(system-assimp):!if(cross_compile:host_build) {
    QMAKE_USE_PRIVATE += assimp
} else {
    include(assimp.pri)
}
