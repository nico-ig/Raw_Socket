#include "log.h"

int main()
{
  log arqLog("./", "raw_socket.log");
  arqLog.add_log("Primeira linha");
  arqLog.add_log("Segunda linha");
}
