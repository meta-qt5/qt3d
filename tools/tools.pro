TEMPLATE = subdirs
QT_FOR_CONFIG += 3dcore-private
tools-only|qtConfig(assimp):qtConfig(commandlineparser): {
    SUBDIRS += qgltf
}
