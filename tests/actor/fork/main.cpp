// This autogenerated skeleton file illustrates how to build a server.
// You should copy it to another filename to avoid overwriting it.

#include "rsg/actor.hpp"
#include "rsg/mailbox.hpp"
#include "rsg/comm.hpp"
#include "rsg/host.hpp"
#include "rsg/engine.hpp"

#include "xbt.h"
#include "simgrid/s4u.hpp"

#include <iostream>
#define UNUSED(x) (void)(x)


XBT_LOG_NEW_CATEGORY(RSG_THRIFT_CLIENT, "Remote SimGrid");
XBT_LOG_NEW_DEFAULT_SUBCATEGORY(RSG_THRIFT_REMOTE_CLIENT, RSG_THRIFT_CLIENT , "RSG server (Remote SimGrid)");

using boost::shared_ptr;
using namespace ::simgrid;


int main(int argc, char **argv) {
  // XBT_INFO("[parent]My id is  : %d",rsg::this_actor::getPid());
  rsg::MailboxPtr mbox = rsg::Mailbox::byName("toto");
  
  pid_t pid = rsg::this_actor::fork();
  if(0 == pid) { // child
    XBT_INFO("[child]My pid is  : %d",rsg::this_actor::getPid());
    XBT_INFO("[child]Fork returned : %d", pid);
    
    XBT_INFO("[child]Message from Daddy : %s", rsg::this_actor::recv(*mbox));
    rsg::this_actor::quit();
    return 0;
  }
  const char *msg = "GaBuZoMeu";
  rsg::this_actor::send(*mbox, msg, strlen(msg) + 1);
  
  XBT_INFO("[parent]My pid is : %d",rsg::this_actor::getPid());
  XBT_INFO("[parent]The pid of my child is : %d", pid);
  rsg::this_actor::quit();
  return 0;
}
