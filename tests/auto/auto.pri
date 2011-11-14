# include() this file to get appropriate INSTALLS values for autotest installation
TEST_INSTALL_BASE = $$[QT_INSTALL_LIBS]/qtconnectivity-tests

target.path = $$TEST_INSTALL_BASE/$$TARGET/
INSTALLS += target
