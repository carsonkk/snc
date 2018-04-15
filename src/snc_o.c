#include "server.h"
#include "client.h"

int main (int argc, char *argv[])
{
  opterr = 0;
  int ret = 0;
  int l_flag = 0;
  int u_flag = 0;
  int c;
  int port_int;
  long port_long;
  err_msg = "internal error: ";
  char *s_value = NULL;
  char *h_arg = NULL;
  char *p_arg = NULL;
  char *usage = "invalid or missing options\nusage: snc [-l] [-u] [-s source_ip_address] [hostname] port\n";
  char *end;

  //begin command-line parsing
  while(1)
  {
    //setup the options array
    static struct option long_options[] = 
      {
        {"listen",  no_argument,    0,  'l'},
        {"udp",    no_argument,    0,  'u'},
        {"source",  required_argument,  0,  's'},
        {0, 0, 0, 0}
      };

    //initialize the index and c
    int option_index = 0;
    c = getopt_long(argc, argv, "lus:", long_options, &option_index);

    //make sure the end hadn't been reached
    if(c == -1)
      break;

    //cycle through the arguments
    switch(c)
    {
      case 'l':
      {
        l_flag = 1;
        break;  
      }
      case 'u':
      {
        u_flag = 1;
        break;
      }
      case 's':
      {
        s_value = optarg;
        break;
      }
      case '?':
      {
        ret = 1;
        break;
      }
      default:
      {
        ret = 1;
        break;
      }
    }
  }

  //post-parsing error handling
  if(ret == 1 || (l_flag == 1 && s_value != NULL) || (l_flag == 0 && optind != argc - 2))
  {
    fprintf(stdout, "%s", usage);
    ret = 1;
    return ret;
  }

  //no hostname specified, only port
  if(optind == argc - 1)
  {
    h_arg = NULL;
    p_arg = argv[optind++];
  }
  //hostname and port specified
  else if(optind == argc - 2)
  {
    h_arg = argv[optind++];
    p_arg = argv[optind++];
  }
  //incorrect number of extra arguments
  else
  {
    fprintf(stdout, "%s", usage);
    ret = 1;
    return ret;
  }

  //Making sure the port is specified correctly/within range
  errno = 0;
  port_long = strtol(p_arg, &end, 10);
  if(*end != 0 || errno != 0 || port_long < 0 || port_long > 65535)
  {
    fprintf(stdout, "%s", usage);
    ret = 1;
    return ret;
  }
  port_int = (int)port_long;

  //server
  if(l_flag == 1)
  {
    ret = server_handler(u_flag, h_arg, port_int);
  }
  //client
  else
  {
    ret = client_handler(u_flag, s_value, h_arg, port_int);  
  }

  return ret;
}