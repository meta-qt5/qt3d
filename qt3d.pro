!tools-only:requires(contains(QT_CONFIG, opengl))

load(configure)
qtCompileTest(assimp)

CONFIG += examples_need_tools
load(qt_parts)

#module_qt3d_tutorials.subdir = tutorials
#module_qt3d_tutorials.target = sub-tutorials
#module_qt3d_tutorials.depends = sub_src
#module_qt3d_tutorials.CONFIG = no_default_target no_default_install

#!package: SUBDIRS += module_qt3d_tutorials

#gcov: SUBDIRS -= sub_tools

OTHER_FILES += \
    sync.profile

tools-only {
    sub_tools.depends -= sub_src
    SUBDIRS = sub_tools
}
