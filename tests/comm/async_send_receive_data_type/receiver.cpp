#include "rsg/actor.hpp"
#include "rsg/mailbox.hpp"
#include "rsg/host.hpp"

#include <xbt.h>
#include <simgrid/s4u.hpp>

#include <stdio.h>
#include <iostream>

#include "struct.h"

XBT_LOG_NEW_CATEGORY(RSG_THRIFT_CLIENT, "Remote SimGrid");
XBT_LOG_NEW_DEFAULT_SUBCATEGORY(RSG_THRIFT_REMOTE_SERVER, RSG_THRIFT_CLIENT , "RSG server (Remote SimGrid)");

using namespace ::RsgService;
using namespace ::simgrid;

int main()
{
    rsg::MailboxPtr mbox = rsg::Mailbox::byName("toto");

    char *buffer = NULL;
    {
        rsg::Comm &comm = rsg::Comm::recv_init(*mbox);
        comm.setDstData((void**)&buffer);
        comm.start();
        comm.wait();
    }
    XBT_INFO("Async Received : %s with size of %d ", buffer, (int) strlen(buffer));

    free(buffer);
    buffer = NULL;

    int *nbElement = NULL;
    {
        rsg::Comm &comm = rsg::Comm::recv_async(*mbox, (void**)&nbElement);
        comm.wait();
    }

    XBT_INFO("I will receive an array of %d elem ", *nbElement);

    int *array = NULL;
    {
        rsg::Comm &comm = rsg::Comm::recv_async(*mbox, (void**)&array);
        comm.wait();
    }
    for(int i = 0 ; i < *nbElement; i++)
    {
        XBT_INFO("array[%d] = %d ", i, array[i]);
    }

    free(nbElement);
    free(array);

    structMsg *recStruct = NULL;
    {
        rsg::Comm &comm = rsg::Comm::recv_async(*mbox, (void**)&recStruct);
        comm.wait();
    }
    XBT_INFO("recStruct->intMsg = %d", recStruct->intMsg);
    XBT_INFO("recStruct->msg = \"%s\"", recStruct->msg);

    free(recStruct);

    rsg::this_actor::quit();
    return 0;
}
