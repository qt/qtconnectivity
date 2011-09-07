This test currently can not be run automatically.

To make this test work, manual connections must be started from a client
machine.  There are 2 options for doing this, either use the rfcommclient application in the subdirectory,
or run the test manually as described below. When using rfcommclient, start qrfcommserver first and then
start rfcommclient on the other device.  There might be some timing issues, so beware.

Steps to run this test manually:

1. start tst_qrfcommserver

2. Wait for test display:

QDEBUG : tst_QRfcommServer::tst_pendingConnections(1 connection) Waiting for 1 simultaneous connections

On a linux machine run:

rfcomm connect hci0 00:1A:9F:92:9E:5A

On the linux machine you will see:

Connected /dev/rfcomm0 to 00:1A:9F:92:9E:5A on channel 1
Press CTRL-C for hangup
Disconnected

and the test will pass.

3. Wait for the test to then display:

QDEBUG : tst_QRfcommServer::tst_receive(test) Listening on RFCOMM channel: 1

On the linux machine run:

rfcomm connect hci0 00:1A:9F:92:9E:5A

You will see:

Connected /dev/rfcomm0 to 00:1A:9F:92:9E:5A on channel 1
Press CTRL-C for hangup

now run:

echo -en "hello\r\n" > /dev/rfcomm0

The test should then pass.

A linux machine is not required, but simple used as an example for how to make
easy rfcomm connections. Any system that will connect to a device and rfcomm
port will work. The format of the string for the second test is very important
for the test to pass.
