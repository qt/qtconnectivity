
This test requires btclient to running on a linux machine, and for it to
be discoverable and connectable to the device under test.

btclient is available in tests/btclient.  It requires a linux machine
with bluez 4 installed.  It does not depend on Qt.

The unit test attempts to use service discovery to locate btclient. For quick
testing this can be time consuming. It will also check the environment variable
TESTSERVER for the mac address bluetooth adaptor of the system running
btclient. You can use hciconfig on the linux system to find it's mac address.
If TESTSERVER is set and btclient is not running the test will fail.


