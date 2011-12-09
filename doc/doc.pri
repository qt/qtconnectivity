OTHER_FILES += \
                $$PWD/qt5.qdocconf \
                $$PWD/qt5-dita.qdocconf

docs_target.target = docs
docs_target.commands = qdoc3 $$PWD/qt5.qdocconf

ditadocs_target.target = ditadocs
ditadocs_target.commands = qdoc3 $$PWD/qt5-dita.qdocconf

QMAKE_EXTRA_TARGETS = docs_target ditadocs_target
QMAKE_CLEAN += \
               "-r $$PWD/html" \
               "-r $$PWD/ditaxml"
