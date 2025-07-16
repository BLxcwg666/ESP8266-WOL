#ifndef URL_PARSER_H
#define URL_PARSER_H

#include <string.h>
#include <stdlib.h>

struct ParsedUrl {
  char host[128];
  int port;
  char path[128];
  bool valid;
};

ParsedUrl parseWebSocketUrl(const char* url) {
  ParsedUrl result;
  result.valid = false;
  result.port = 80;
  strcpy(result.path, "/");
  
  const char* hostStart = NULL;
  if (strncmp(url, "ws://", 5) == 0) {
    hostStart = url + 5;
    result.port = 80;
  } else if (strncmp(url, "wss://", 6) == 0) {
    hostStart = url + 6;
    result.port = 443;
  } else {
    return result;
  }
  
  const char* hostEnd = strchr(hostStart, ':');
  const char* pathStart = strchr(hostStart, '/');
  
  if (hostEnd && (!pathStart || hostEnd < pathStart)) {
    int hostLen = hostEnd - hostStart;
    if (hostLen >= sizeof(result.host)) hostLen = sizeof(result.host) - 1;
    strncpy(result.host, hostStart, hostLen);
    result.host[hostLen] = '\0';
    result.port = atoi(hostEnd + 1);
    pathStart = strchr(hostEnd, '/');
  } else {
    if (pathStart) {
      int hostLen = pathStart - hostStart;
      if (hostLen >= sizeof(result.host)) hostLen = sizeof(result.host) - 1;
      strncpy(result.host, hostStart, hostLen);
      result.host[hostLen] = '\0';
    } else {
      strlcpy(result.host, hostStart, sizeof(result.host));
      pathStart = NULL;
    }
  }
  
  if (pathStart) {
    strlcpy(result.path, pathStart, sizeof(result.path));
  }
  
  result.valid = strlen(result.host) > 0;
  return result;
}

#endif // URL_PARSER_H