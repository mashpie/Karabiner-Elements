#pragma once

#include "constants.hpp"
#include "device_grabber.hpp"
#include "event_manipulator.hpp"
#include "gcd_utility.hpp"
#include "logger.hpp"
#include "notification_center.hpp"
#include "receiver.hpp"
#include "session.hpp"
#include "version_monitor.hpp"
#include <sys/stat.h>

namespace krbn {
class connection_manager final {
public:
  connection_manager(const connection_manager&) = delete;

  connection_manager(version_monitor& version_monitor,
                     manipulator::event_manipulator& event_manipulator,
                     device_grabber& device_grabber) : version_monitor_(version_monitor),
                                                       event_manipulator_(event_manipulator),
                                                       device_grabber_(device_grabber),
                                                       timer_(nullptr),
                                                       last_uid_(0) {
    timer_ = std::make_unique<gcd_utility::main_queue_timer>(
        dispatch_time(DISPATCH_TIME_NOW, 1.0 * NSEC_PER_SEC),
        1.0 * NSEC_PER_SEC,
        0,
        ^{
          if (auto uid = session::get_current_console_user_id()) {
            if (last_uid_ != *uid) {
              last_uid_ = *uid;
              logger::get_logger().info("current_console_user_id: {0}", *uid);

              version_monitor_.manual_check();

              receiver_ = nullptr;
              receiver_ = std::make_unique<receiver>(event_manipulator_, device_grabber_);
            }
          }
        });
  }

  ~connection_manager(void) {
    timer_ = nullptr;
    receiver_ = nullptr;
  }

private:
  version_monitor& version_monitor_;
  manipulator::event_manipulator& event_manipulator_;
  device_grabber& device_grabber_;

  std::unique_ptr<gcd_utility::main_queue_timer> timer_;

  uid_t last_uid_;

  std::unique_ptr<receiver> receiver_;
};
}
