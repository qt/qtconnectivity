TEMPLATE = subdirs
CONFIG += ordered

module_qtconnectivity_src.subdir = src
module_qtconnectivity_src.target = module-qtconnectivity-src

module_qtconnectivity_tests.subdir = tests
module_qtconnectivity_tests.target = module-qtconnectivity-tests
module_qtconnectivity_tests.depends = module_qtconnectivity_src
module_qtconnectivity_tests.CONFIG = no_default_install
!contains(QT_BUILD_PARTS,tests) {
    module_qtconnectivity_tests.CONFIG += no_default_target
}

module_qtconnectivity_examples.subdir = examples
module_qtconnectivity_examples.target = module-qtconnectivity-exampels
module_qtconnectivity_examples.depends = module_qtconnectivity_src
!contains(QT_BUILD_PARTS,examples) {
    module_qtconnectivity_examples.CONFIG = no_default_target no_default_install
    warning("No examples being used")
}

SUBDIRS += module_qtconnectivity_src \
           module_qtconnectivity_tests \
           module_qtconnectivity_examples

include(doc/doc.pri)
