/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <poll.h>
#include <assert.h>
#include <fcntl.h>
#include <limits.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <bluetooth/l2cap.h>

#include <dbus/dbus.h>

#include "btclient.h"

#define UNUSED(x) do { (void)x; } while(0)

const char *xmldefn =
        "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"
        "\n"
        "<record>\n"
        "    <attribute id=\"0x0000\">\n"
        "        <uint32 value=\"0x00010010\" />\n"
        "    </attribute>\n"
        "    <attribute id=\"0x0001\">\n"
        "        <sequence>\n"
        "            <uuid value=\"" ECHO_SERVICE_UUID "\" />\n"
        "        </sequence>\n"
        "    </attribute>\n"
        "    <attribute id=\"0x0003\">\n"
        "        <uuid value=\"" ECHO_SERVICE_UUID "\" />\n"
        "    </attribute>\n"
        "    <attribute id=\"0x0004\">\n"
        "        <sequence>\n"
        "            <sequence>\n"
        "                <uuid value=\"0x0100\" />\n"
        "            </sequence>\n"
        "            <sequence>\n"
        "                <uuid value=\"0x0003\" />\n"
        "                <uint8 value=\"0x0a\" />\n"
        "            </sequence>\n"
        "        </sequence>\n"
        "    </attribute>\n"
        "    <attribute id=\"0x0005\">\n"
        "        <sequence>\n"
        "            <uuid value=\"0x1002\" />\n"
        "        </sequence>\n"
        "    </attribute>\n"
        "    <attribute id=\"0x0100\">\n"
        "        <text value=\"QtBluetooth Test Echo Server\" />\n"
        "    </attribute>\n"
        "    <attribute id=\"0x0101\">\n"
        "        <text value=\"QtBluetooth test echo server\" />\n"
        "    </attribute>\n"
        "    <attribute id=\"0x0102\">\n"
        "        <text value=\"Nokia, QtDF\" />\n"
        "    </attribute>\n"
        "</record>\n";

typedef void (*actionhandler_t)(int, short, void *);

struct fdlist {
    struct fdlist *next;
    int fd;
    short events;
    actionhandler_t hanlder;
    void *ptr;
};

struct fdlist *head = 0;

void removefd(struct fdlist *fdl, int fd){    
    struct fdlist *prev = fdl;
    while(fdl && fdl->fd != fd) {
        prev = fdl;
        fdl = fdl->next;    
    }
    assert(fdl);

    prev->next = fdl->next;
    free(fdl);
}

struct fdlist *addfd(struct fdlist *fdl, int fd, short events, actionhandler_t hanlder, void *data){
    struct fdlist *n = malloc(sizeof(struct fdlist));
    if(fdl){
        while(fdl->next)
            fdl = fdl->next;
        fdl->next = n;
    }

    n->next = 0;
    n->fd = fd;
    n->events = events;
    n->hanlder = hanlder;
    n->ptr = data;

    return n;

}

int mkpoll(struct fdlist *fdl, struct pollfd *fds, int max){
   int num = 0;
   while(fdl && num < max){
       fds[num].events = fdl->events;
       fds[num].fd = fdl->fd;
       fdl = fdl->next;
       num++;
   }
   assert(num <= max);
   return num;
}

int createListening(int channel){
    struct sockaddr_rc addr;
    int flags;
    socklen_t addrLength = sizeof(addr);
    int sk, opt = 1;

    /* Create echo socket on bt channel 10 */
    sk = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

    addr.rc_family = AF_BLUETOOTH;
    addr.rc_channel = channel;
    memset(&addr.rc_bdaddr, 0, sizeof(bdaddr_t));

    if (bind(sk, (struct sockaddr *)&addr, addrLength) < 0) {
        perror("Failed to bind to Bluetooth socket");
        return -1;
    }

    if(setsockopt(sk, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0){
        perror("Warning: Failed to set SO_REUSEADDR options");
    }

    // ensure that O_NONBLOCK is set on new connections.
    flags = fcntl(sk, F_GETFL, 0);
    if (!(flags & O_NONBLOCK))
        fcntl(sk, F_SETFL, flags | O_NONBLOCK);

    if (listen(sk, 10) < 0){
        perror("Can't start listening on bluetooth socket");
        return -1;
    }
    printf("Got socket: %d\n", sk);
    return sk;
}

int createListeningL2cap(int psm){
    struct sockaddr_l2 addr;
    int flags;
    socklen_t addrLength = sizeof(struct sockaddr_l2);
    int sk, opt = 1;

    sk = socket(AF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);

    memset(&addr,0, addrLength);
    addr.l2_family = AF_BLUETOOTH;
    addr.l2_psm = htobs(psm);
    memset(&addr.l2_bdaddr, 0, sizeof(bdaddr_t));

    if (bind(sk, (struct sockaddr *)&addr, addrLength) < 0) {
        perror("Failed to bind to Bluetooth socket");
        return -1;
    }

    if(setsockopt(sk, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0){
        perror("Warning: Failed to set SO_REUSEADDR options");
    }

    // ensure that O_NONBLOCK is set on new connections.
    flags = fcntl(sk, F_GETFL, 0);
    if (!(flags & O_NONBLOCK))
        fcntl(sk, F_SETFL, flags | O_NONBLOCK);

    if (listen(sk, 10) < 0){
        perror("Can't start listening on bluetooth socket");
        return -1;
    }
    printf("Got L2cap socket: %d\n", sk);
    return sk;
}

void echo_process(int fd, short revents, void *data){
    char buff[1024];

    UNUSED(data);

    if(revents&POLLHUP || revents&POLLNVAL){
        close(fd);
        removefd(head, fd);
        return;
    }
    int size = read(fd, buff, 1024);
    printf("%d: Read: %d bytes\n", fd, size);
    size = write(fd, buff, size);
    printf("%d: Wrote: %d bytes\n", fd, size);
}

void echo_connect(int fd, short revents, void *data){

    struct sockaddr_rc addr;
    unsigned char *a;
    socklen_t length = sizeof(struct sockaddr_rc);

    UNUSED(data);
    UNUSED(revents);

    printf("Echo connect started\n");

    int sk = accept(fd, (struct sockaddr *)&addr, &length);

    a = addr.rc_bdaddr.b;
    printf("Connect from: %02x:%02x:%02x:%02x:%02x:%02x port %d\n", a[5], a[4], a[3], a[2], a[1], a[0], addr.rc_channel);
    addfd(head, sk, POLLIN|POLLHUP|POLLNVAL, echo_process, 0);

}

struct connectioninfo {
    int socket;
    FILE *pipe;
    struct sockaddr_rc addr_them;
};

void id_greeting(int fd, short revents, void *data){
    int err;
    unsigned char *a, *b;
    struct sockaddr_rc addr;
    char buffer[1024];
    char name[100];
    socklen_t addrLength = sizeof(addr);
    struct connectioninfo *ci;

    if(revents&POLLHUP || revents&POLLNVAL){
        close(fd);
        removefd(head, fd);
        return;
    }

    ci = (struct connectioninfo *)data;

    err = getsockname(ci->socket, (struct sockaddr *)&addr, &addrLength);
    if(err < 0){
            perror("failed to get socket name");
            exit(-1);
    }
    a = addr.rc_bdaddr.b;

    b = ci->addr_them.rc_bdaddr.b;

    err = fscanf(ci->pipe, "%s", name);
    removefd(head, fd);
    pclose(ci->pipe);

    sprintf(buffer, "%02x:%02x:%02x:%02x:%02x:%02x %d %s %02x:%02x:%02x:%02x:%02x:%02x %d\n",
                    a[5], a[4], a[3], a[2], a[1], a[0], addr.rc_channel, name,
                    b[5], b[4], b[3], b[2], b[1], b[0], ci->addr_them.rc_channel);
    if(write(ci->socket, buffer, strlen(buffer)) < 0){
        perror("Failed to write greeting");
    }
    printf("%s", buffer);
    free(ci);

}

void id_process(int fd, short revents, void *data){
    int err;
    unsigned char *b;
    struct sockaddr_rc addr_them;
    char cmd[100];
    socklen_t addrLength = sizeof(addr_them);
    struct connectioninfo *ci;

    UNUSED(data);

    if(revents&POLLHUP || revents&POLLNVAL){
        close(fd);
        removefd(head, fd);
        return;
    }

    err = getpeername(fd, (struct sockaddr *)&addr_them, &addrLength);
    if(err < 0){
            perror("failed to get socket name");
            exit(-1);
    }
    b = addr_them.rc_bdaddr.b;

    sprintf(cmd, "hcitool name %02x:%02x:%02x:%02x:%02x:%02x", b[5], b[4], b[3], b[2], b[1], b[0]);

    FILE *f = popen(cmd, "r");
    ci = malloc(sizeof(struct connectioninfo));
    ci->socket = fd;
    ci->pipe = f;
    memcpy(&ci->addr_them, &addr_them, sizeof(struct sockaddr_rc));

    addfd(head, fileno(f), POLLIN, id_greeting, ci);

}

void id_connect(int fd, short revents, void *data){

    struct sockaddr_rc addr;
    unsigned char *a;
    socklen_t length = sizeof(struct sockaddr_rc);

    UNUSED(revents);
    UNUSED(data);

    printf("ID connect started\n");

    int sk = accept(fd, (struct sockaddr *)&addr, &length);

    a = addr.rc_bdaddr.b;
    printf("Connect from: %02x:%02x:%02x:%02x:%02x:%02x port %d\n", a[5], a[4], a[3], a[2], a[1], a[0], addr.rc_channel);
    addfd(head, sk, POLLIN|POLLHUP|POLLNVAL, id_process, 0);
    id_process(sk, POLLIN, 0x0);
}

void id_print(int socket){

    int err;
    unsigned char *a;
    struct sockaddr_rc addr;
    socklen_t addrLength = sizeof(addr);

    err = getsockname(socket, (struct sockaddr *)&addr, &addrLength);
    if(err < 0){
            perror("failed to get socket name");
            exit(-1);
    }
    a = addr.rc_bdaddr.b;
    printf("We are: %d:%d:%d:%d:%d:%d port %d\n", a[0], a[1], a[2], a[3], a[4], a[5], addr.rc_channel);
}

void l2_process(int fd, short revents, void *data){
    char buff[1024];

    UNUSED(data);

    if(revents&POLLHUP || revents&POLLNVAL){
        close(fd);
        removefd(head, fd);
        return;
    }
    int size = read(fd, buff, 1024);
    printf("%d: Read: %d bytes\n", fd, size);
    size = write(fd, buff, size);
    printf("%d: Wrote: %d bytes\n", fd, size);
}

void l2_connect(int fd, short revents, void *data){

    struct sockaddr_l2 addr;
    unsigned char *a;
    socklen_t length = sizeof(struct sockaddr_l2);

    UNUSED(revents);
    UNUSED(data);

    printf("L2 connect started\n");

    int sk = accept(fd, (struct sockaddr *)&addr, &length);

    a = addr.l2_bdaddr.b;
    printf("Connect from: %02x:%02x:%02x:%02x:%02x:%02x port %d\n", a[5], a[4], a[3], a[2], a[1], a[0], addr.l2_psm);
    addfd(head, sk, POLLIN|POLLHUP|POLLNVAL, l2_process, 0);
    l2_process(sk, POLLIN, 0x0);
}

void dbus_error(int fd, short revents, void *data){
    UNUSED(fd);
    UNUSED(data);
    if(revents&POLLHUP || revents&POLLNVAL){
        printf("dBus connection failed, shutting down\n");
        exit(-1);
    }
}

void registerService()
{
    DBusMessage* msg;
    DBusMessageIter args;
    DBusConnection* conn;
    DBusError err;
    DBusPendingCall* pending;
    dbus_uint32_t level;
    int fd;

    char path[PATH_MAX];
    const char *ret_string = 0;

    // initialiset the errors
    dbus_error_init(&err);
    conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
    if (dbus_error_is_set(&err)) {
       fprintf(stderr, "Connection Error (%s)\n", err.message);
       dbus_error_free(&err);
       return;
    }

    // create a new method call and check for errors
    msg = dbus_message_new_method_call("org.bluez",         // target for the method call
                                       "/",                 // object to call on
                                       "org.bluez.Manager", // interface to call on
                                       "DefaultAdapter");   // method name

    if (!dbus_connection_send_with_reply (conn, msg, &pending, -1)) {
       fprintf(stderr, "dbus-send failed: Out Of Memory!\n");
       return;
    }

    dbus_connection_flush(conn);

    dbus_message_unref(msg);
    dbus_pending_call_block(pending);
    msg = dbus_pending_call_steal_reply(pending);
    dbus_pending_call_unref(pending);

    if (!dbus_message_iter_init(msg, &args))
       fprintf(stderr, "Message has no arguments!\n");
    else if (DBUS_TYPE_OBJECT_PATH != dbus_message_iter_get_arg_type(&args))
       fprintf(stderr, "Argument is not an object path!\n");
    else
       dbus_message_iter_get_basic(&args, &ret_string);

    if(!ret_string){
        fprintf(stderr, "Failed to get bluez path\n");
        return;
    }

    strcpy(path, ret_string);
    // change path to use any
    strcpy(rindex(path, '/'), "/any");

    printf("Using path: %s\n", path);   

    dbus_message_unref(msg);

    // create a new method call and check for errors
    msg = dbus_message_new_method_call("org.bluez",         // target for the method call
                                       path,                // object to call on
                                       "org.bluez.Service", // interface to call on
                                       "AddRecord");        // method name

    if(!dbus_message_append_args(msg, DBUS_TYPE_STRING, &xmldefn, DBUS_TYPE_INVALID)){
        fprintf(stderr, "Failed to append args\n");
        return;
    }

    // send message and get a handle for a reply
    if (!dbus_connection_send_with_reply (conn, msg, &pending, -1)) { // -1 is default timeout
       fprintf(stderr, "Out Of Memory!\n");
       exit(1);
    }
    dbus_connection_flush(conn);

    // free message
    dbus_message_unref(msg);

    // block until we receive a reply
    dbus_pending_call_block(pending);

    // get the reply message
    msg = dbus_pending_call_steal_reply(pending);
    if (NULL == msg) {
       fprintf(stderr, "Reply Null\n");
       return;
    }
    // free the pending message handle
    dbus_pending_call_unref(pending);

    // read the parameters
    if (!dbus_message_iter_init(msg, &args))
       fprintf(stderr, "Message has no arguments!\n");
    else if (DBUS_TYPE_UINT32 != dbus_message_iter_get_arg_type(&args))
       fprintf(stderr, "Argument is not int!\n");
    else
       dbus_message_iter_get_basic(&args, &level);

    printf("Got handle: 0x%x\n", level);

    // free reply
    dbus_message_unref(msg);

    dbus_connection_get_socket(conn, &fd);
    addfd(head, fd, POLLHUP|POLLNVAL, dbus_error, 0x0);
}

int main(int argc, char **argv)
{
        int socket;        
#define MAX_POLL 256
        struct pollfd fds[MAX_POLL];        

        UNUSED(argc);
        UNUSED(argv);

        registerService();

        socket = createListening(10);
        head = addfd(head, socket, POLLIN, echo_connect, 0); // first creation

        socket = createListening(11);
        addfd(head, socket, POLLIN, id_connect, 0);

        socket = createListeningL2cap(0x1011); // must be > 0x1001 and odd
        addfd(head, socket, POLLIN, l2_connect, 0);

        while(1){
            int n = mkpoll(head, fds, MAX_POLL);
            if(poll(fds, n, -1)){                
                struct fdlist *fdl = head;
                int i;

                for(i = 0; i < n; i++){
                    if(fds[i].revents){
                        while(fdl && fdl->fd != fds[i].fd)
                            fdl = fdl->next;
                        assert(fdl);
                        fdl->hanlder(fds[i].fd, fds[i].revents, fdl->ptr);
                    }
                }
            }
            else {
                perror("Poll failed");
                exit(-1);
            }
        }

        return 0;
}
