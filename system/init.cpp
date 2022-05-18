#include "core/childProcess.hpp"
#include "core/tinyxml2.h"
#include <array>
#include <cstdio>
#include <memory>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <stdio.h>
#include <string>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

using namespace tinyxml2;

int main(int argc, char *argv[]) {
  // simple init system
  // remount rootfs as rw
  printf("mounting rootfs\n");
  mount("/", "/", "", MS_REMOUNT, nullptr);

  // mount /proc
  printf("mounting /proc\n");
  mount("proc", "/proc", "proc", 0, nullptr);

  printf("mounting /sys\n");

  XMLDocument doc;
  doc.LoadFile("/theos/system/boot.xml");

  auto service = doc.FirstChildElement("services")->FirstChildElement();

  if (service != nullptr) {
    const char *serviceName = service->FindAttribute("name")->Value();
    const char *serviceCommand = service->FindAttribute("command")->Value();
    printf("starting service %s\n", serviceName);
    // TODO: foreground/background

    std::vector<char *> args;

    for (auto ele = service->FirstChild(); ele; ele = ele->NextSibling()) {
      std::string serviceArgument = ele->Value();
      args.push_back(&serviceArgument[0]);
    }

    args.push_back(nullptr);

    createChild(args[0], &args[0], environ, "");
  } else {
    throw std::runtime_error("no service metadata found\n");
  }

  char *args[] = {"/bin/sh", nullptr};
  execve(args[0], args, environ);
}
