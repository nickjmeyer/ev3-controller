#ifndef EV3_ROBOT_HPP
#define EV3_ROBOT_HPP

#include <sstream>
#include "command.pb.h"

#ifdef BUILD_FOR_EV3

#include "ev3dev.hpp"
namespace ev3 = ev3dev;

#endif

namespace Ev3Controller {


class Ev3Robot {
public:
#ifdef BUILD_FOR_EV3
    Ev3Robot(const ev3dev::address_type left, const ev3dev::address_type right,
            const float wheelBase);
#else
    Ev3Robot();
#endif

    void setVelocity(const Ev3Command & command);
    void logVelocity();

private:
    double x_l;
    double z_a;
#ifdef BUILD_FOR_EV3
    ev3dev::large_motor leftMotor;
    ev3dev::large_motor rightMotor;
    const float wheelBase;
#endif
};


} // namespace Ev3Controller


#endif // EV3_ROBOT_HPP
