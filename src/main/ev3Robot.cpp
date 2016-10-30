#include "ev3Robot.hpp"
#include <iostream>

namespace Ev3Controller {


#ifdef BUILD_FOR_EV3
Ev3Robot::Ev3Robot(const ev3dev::address_type left, const ev3dev::address_type right,
        const float wheelBase)
    : x_l(0.0), z_a(0.0),
      leftMotor(left), rightMotor(right),
      wheelBase(wheelBase) {
    this->leftMotor.stop();
    this->rightMotor.stop();
}
#else
Ev3Robot::Ev3Robot()
    : x_l(0.0), z_a(0.0){
}
#endif


void Ev3Robot::setVelocity(const Ev3Command & command){
    x_l = command.xvel();
    z_a = command.zrot();

#ifdef BUILD_FOR_EV3
    double left_sp = x_l - (1.0 - 2.0*(x_l < 0)) * z_a * this->wheelBase;
    double right_sp = x_l + (1.0 - 2.0*(x_l < 0)) * z_a * this->wheelBase;

    if(left_sp > 100.0)
        left_sp = 100.0;
    else if(left_sp < -100.0)
        left_sp = -100.0;

    if(right_sp > 100.0)
        right_sp = 100.0;
    else if(right_sp < -100.0)
        right_sp = -100.0;


    this->leftMotor.set_duty_cycle_sp(left_sp);
    this->rightMotor.set_duty_cycle_sp(right_sp);
    this->leftMotor.run_direct();
    this->rightMotor.run_direct();
    std::cout << "Set motor speeds"
              << "(" << left_sp << ", " << right_sp << ")"
              << std::endl;

#endif

    this->logVelocity();
}



void Ev3Robot::logVelocity(){
    std::stringstream ss;
    ss << "speed: [" << x_l << ", " << z_a << "]";
    std::cout << ss.str() << std::endl;
}

} // namespace Ev3Controller
