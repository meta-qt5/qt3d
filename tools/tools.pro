TEMPLATE = subdirs

!tools-only:!qtHaveModule(3dcore): \
    return()

QT_FOR_CONFIG += 3dcore-private
tools-only|qtConfig(assimp):qtConfig(commandlineparser): {
    SUBDIRS += qgltf
}
