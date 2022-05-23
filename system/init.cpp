#include "core/childProcess.hpp"
#include "core/tinyxml2.h"
#include <array>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <numeric>
#include <spdlog/spdlog.h>
#include <sstream>
#include <stdexcept>
#include <stdio.h>
#include <string>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <vector>

using namespace tinyxml2;

auto runService(const std::string &serviceName,
                const std::string &serviceCommand, std::vector<char*> args) -> int {
  // TODO: custom environment variables
  int inp[2], outp[2];
  pipe(inp);
  pipe(outp);

  switch (fork()) {
  case -1:
    spdlog::error("fork failed for {}", serviceName);
    return -1;

  case 0:
    close(inp[0]);
    close(1);
    dup(inp[1]);
    close(inp[1]);
    close(outp[1]);
    close(0);
    dup(outp[0]);
    close(outp[0]);

    execve(serviceCommand.c_str(), &args[0], environ);
    spdlog::error("exec failed for {}", serviceName);
  }

  return 0;
}

int main(int argc, char *argv[]) {
  // simple init system
  // remount rootfs as rw
  spdlog::info("mounting rootfs\n");
  mount("/", "/", "", MS_REMOUNT, nullptr);

  // mount /proc
  spdlog::info("mounting /proc\n");
  mount("proc", "/proc", "proc", 0, nullptr);

  printf("mounting /sys\n");

  XMLDocument doc;
  doc.LoadFile("/theos/system/boot.xml");

  auto service = doc.FirstChildElement("services")->FirstChildElement();

  if (service != nullptr) {
    std::string serviceName = service->FindAttribute("name")->Value();
    std::string serviceCommand = service->FindAttribute("command")->Value();
    printf("starting service %s\n", serviceName.c_str());
    // TODO: foreground/background

    std::vector<char *> args;

    args.push_back(&serviceCommand[0]);

    for (auto ele = service->FirstChild(); ele; ele = ele->NextSibling()) {
      std::string serviceArgument = ele->Value();
      args.push_back(&serviceArgument[0]);
    }

    args.push_back(nullptr);

    std::thread worker(runService, serviceName, serviceCommand, args);
  } else {
    throw std::runtime_error("no service metadata found\n");
  }
}
