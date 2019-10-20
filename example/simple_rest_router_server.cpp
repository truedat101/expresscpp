#include <iostream>
#include <map>
#include <memory>
#include <vector>

#include "magic_enum.hpp"

#include "expresscpp/expresscpp.hpp"

void LoggerMiddleware(express_request_t req, express_response_t /*res*/) {
  std::time_t time_now_t = std::chrono::system_clock::to_time_t(req->timestamp_);
  std::cout << "LOGGER: time: "
            << "example time " << std::ctime(&time_now_t) << ", path: " << req->path_
            << ", method: " << magic_enum::enum_name(req->method_) << std::endl;
  //  next();
}

void AuthMddleware(express_request_t /*req*/, express_response_t res, NextRouter next) {
  std::cout << "exampleHandler called " << std::endl;
  //  if (req->headers.empty()) {
  next();
  //  }
  res->SetStatus(200);
}

class ThingAdmin {
 public:
  void Login(express_request_t /*req*/, express_response_t res) {
    std::cout << "thing admin called" << std::endl;
    res->Send("admining thing");
  }
  void Reboot(express_request_t /*req*/, express_response_t res) {
    std::cout << "reboot called" << std::endl;
    res->Send("rebooting");
  }
  void Update(express_request_t /*req*/, express_response_t res) {
    std::cout << "update called" << std::endl;
    res->Send("updating");
  }
};

class UserMagement {
 public:
  void Login(express_request_t /*req*/, express_response_t res) {
    std::cout << "login called" << std::endl;
    res->Send("logged in");
  }
  void Me(express_request_t /*req*/, express_response_t res) {
    std::cout << "me called" << std::endl;
    res->Send("you are great");
  }
  void Logout(express_request_t /*req*/, express_response_t res) {
    std::cout << "logout called" << std::endl;
    res->Send("logged out");
  }
};

int main() {
  std::cout << "Hello World!" << std::endl;
  using namespace std::placeholders;

  auto expresscpp = std::make_shared<ExpressCpp>();

  expresscpp->Use(LoggerMiddleware);

  expresscpp->Get("/", [](auto /*req*/, auto res) { res->Send("hello world!"); });

  // Api v0
  auto api_router = expresscpp->GetRouter("api router");
  expresscpp->Use("/api/v0", api_router);

  // Things
  auto things_router = expresscpp->GetRouter("thing router");
  api_router->Use("/things", things_router);

  things_router->Get("/", [](auto /*req*/, auto res) {
    std::cout << "things called" << std::endl;
    res->Send("getting all things");
  });
  things_router->Post("/", [](auto /*req*/, auto res) {
    std::cout << "things called" << std::endl;
    res->Send("adding thing");
  });
  things_router->Get("/:thing_id/status", [](auto /*req*/, auto res) {
    std::cout << "things called" << std::endl;
    res->Send("getting status of things");
  });

  // Things admin
  auto things_admin_router = expresscpp->GetRouter("thing router");
  things_router->Use("/:thing_id/admin", things_admin_router);
  ThingAdmin a;
  things_admin_router->Post("/login", std::bind(&ThingAdmin::Login, &a, _1, _2));
  things_admin_router->Post("/reboot", std::bind(&ThingAdmin::Reboot, &a, _1, _2));
  things_admin_router->Post("/update", std::bind(&ThingAdmin::Update, &a, _1, _2));

  // Users
  auto user_router = expresscpp->GetRouter("user router");
  api_router->Use("/user", user_router);
  UserMagement m;
  user_router->Post("/login", std::bind(&UserMagement::Login, &m, _1, _2));
  user_router->Get("/me", std::bind(&UserMagement::Me, &m, _1, _2));
  user_router->Post("/logout", std::bind(&UserMagement::Login, &m, _1, _2));

  // 404 if route not found
  expresscpp->Use("*", [](auto /*req*/, auto res) {
    res->SetStatus(404);
    res->Send("not found");
  });

  expresscpp->Stack();
  /*
  prints:
      GET: "/debug"
      ALL: ""
      GET: "/"
      ALL: "/api/v0"
      ALL: "*"
      ALL: "/things"
          GET: "/"
          POST: "/"
          GET: "/:thing_id/status"
          ALL: "/:thing_id/admin"
              POST: "/login"
              POST: "/reboot"
              POST: "/update"
      ALL: "/user"
          POST: "/login"
          GET: "/me"
          POST: "/logout"
  */
  // TODO: handle error of port already being used etc.
  constexpr uint16_t port = 8081u;
  expresscpp
      ->Listen(port, []() { std::cout << "Example app listening on port " << port << std::endl; })
      .Block();

  return 0;
}
