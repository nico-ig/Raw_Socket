#ifndef _LOG_
#define _LOG_

#include <fstream>

using namespace std;

class log
{
  private:
    string nomeArq;
    fstream arq;

  public:
    log(string nome);
    log(string caminho, string nome);
    void add_log(string str); 
};

log::log(string nome) { nomeArq = "/var/log/" + nome; }
log::log(string caminho, string nome) { nomeArq = caminho + nome; }

void log::add_log(string str) 
{ 
  arq.open(nomeArq, ios::binary | ios::trunc | ios::in | ios::app); 
  arq << str << "\n"; 
  arq.close(); 
}

#endif
