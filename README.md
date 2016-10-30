# Ev3-Controller over WiFi

## Setting up desktop

- Download last stable [ASIO](http://think-async.com/Asio/Download)
  and place it in `~/libraries/asio`.  This is needed for networking.
  CMake files assume that the `include` directory is located at
  `~/libraries/asio/include`.
- Install `libprotobuf-dev` and `protobuf-compiler` from the package
  manager.
- Make working directory on host machine `mkdir ~/ev3dev && cd
  ~/ev3dev`.
- Clone controller code `git clone
  https://github.com/nickjmeyer/ev3-controller`.
- Build directory `cd ev3-controller && mkdir build && cd build`.
- Build library `cmake -DCMAKE_BUILD_TYPE=Release .. && make`.

## running on desktop
- Open a new terminal `cd ~/ev3dev/ev3-controller`.
- Run server `./build/src/main/ev3Server`.
- Open a new terminal and go to`cd ~/ev3dev/ev3-controller`.
- Run client `./build/src/main/ev3Client`.
- Open another new terminal and go to `~/ev3dev/ev3-controller`.
- Run client `./build/src/main/ev3Client`.
- Press `r` to refresh list of clients.  Should see two alpha-numeric
  strings.
- Use the arrow keys to scroll up and down and press enter to select a
  robot.
- After selecting a robot, use the arrow keys to control the robot's
  velocity.
- Press `q` to stop the current robot and go back to the main menu.
  - When at the main menu, press `q` to quit the program.


## Creating Docker image for Ev3
- Install Docker `sudo apt install docker`.
- Download the Ev3Dev base image `docker pull
  ev3dev/ev3dev-jessie-ev3-base`.
- Tag image `docker tag ev3dev/ev3dev-jessie-ev3-base ev3-base`.
- Start docker `docker run --name="ev3-controller" -it -v ~/:/src
  ev3-base`.
- Login as robot `login robot`. Password is `maker`.
- Create directory for asio `mkdir ~/libraries`.
- Copy asio `cp -r /src/libraries/asio ~/libraries`.
- Install libraries `sudo apt-get install build-essential cmake
  libprotobuf-dev protobuf-compiler`.
- Copy controller code `cp -r /src/ev3dev/ev3-controller ~/`.
- Move into build directory `cd ~/ev3-controller/build`.
- Remove desktop build code `rm -r *`.
- Set up to build for Ev3 `cmake -DCMAKE_BUILD_TYPE=Release
  -DEV3_SERVER_HOSTNAME=YOUR.SERVER -DBUILD_FOR_EV3=ON .. && make`
  where `YOUR.SERVER` is the address of the machine running the server
  code.
- Create a shortcut `cd ~/ && ln -sr
  ~/ev3-controller/build/src/main/ev3Client ev3Client`.
- Log out of user robot `exit`.
- Exit docker `exit`.
- Commit Docker container `docker commit ev3-controller
  ev3-controller`
- Navigate to top of working directory `cd ~/ev3dev`.
- Clone Brickstrap `git clone git clone
  git://github.com/ev3dev/brickstrap`.
- Navigate to Brickstrap `cd ~/ev3dev/brickstrap`.
- Install packages `sudo apt-get install docker-engine
  libguestfs-tools qemu-user-static`.
- If you have never used `libguestfs` before, set it up.
  - `sudo update-guestfs-appliance`
  - `sudo usermod -a -G kvm $USER`
  - `newgrp kvm`
  - `sudo chmod +r /boot/vmlinuz*`
- Create tar from Docker image `./src/brickstrap.sh create-tar
  ev3-controller ev3-controller.tar`.
- Create image from tar `./src/brickstrap.sh create-image
  ev3-controller.tar ev3-controller.img`.
- Burn the image to an SD-card using the instructions on
  the [ev3-dev website](http://www.ev3dev.org/docs/getting-started/).
