# Ev3-Controller over WiFi

## Setting up desktop
- Install `libprotobuf-dev` and `protobuf-compiler` from the package
  manager.
- Make working directory on host machine `mkdir ~/ev3dev && cd
  ~/ev3dev`.
- Clone controller code `git clone
  https://github.com/nickjmeyer/ev3-controller`.
- Setup Asio submodule `cd ev3-controller && git submodule init && git
  submodule upate`.
- Build directory `mkdir build && cd build`.
- Build library `cmake -DCMAKE_BUILD_TYPE=Release .. && make`.

## running on desktop
To run on a desktop, clone the `nickjmeyer/ev3-robot-client`
repository and compile.

- Open a new terminal `cd ~/ev3dev/ev3-controller`.
- Run server `./build/src/main/ev3Server`.
- Open a new terminal and go to`cd ~/ev3dev/ev3-controller`.
- Run client `ev3Client` from the ev3-robot-client repo.
- Press `r` to refresh list of clients.  Should see alpha-numeric
  strings for each client running.
- Use the arrow keys to scroll up and down and press enter to select a
  robot.
- After selecting a robot, use the arrow keys to control the robot's
  velocity.
- Press `q` to stop the current robot and go back to the main menu.
  - When at the main menu, press `q` to quit the program.
