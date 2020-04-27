# imgui4xp
Imgui Template For X-Plane 11 Also Showing How To Support Multi Platform From Linux

I am documenting the process I took to get a multiplatform stand alone plugin that supports imgui on X-Plane built on Ubuntu 16.04 LTS. My goal it to make it easier for the next developer to amcomplish the same task. How I plan to use this is to develop an idea on FlyWithLua NG using its new support for imgui and when developed far enough then convert it to a stand alone plugin with no need for lua. I hope someone will find this usfull. 

First clone imgui4xp using this command "git clone https://github.com/sparker256/imgui4xp.git" and put the resulting imgui4xp folder wherever you like. I normanly put myself in the folder where I want the repository folder to reside first. 

Since I develop on Linux my IDE of choice is QT Creator which is installed from my package manager.

I have scripts and make files that alow me to click on one button on my desktop and automaticly build for all three platforms.

You will need a compiler to build the linux files and the following command will get that for you.

sudo apt-get install build-essential

To build for Windows on Ubuntu you will need to use this command. sudo apt-get install mingw-w64

To build the Mac files on Ubuntu you need to go to the osxcross GitHub site found here.

https://github.com/tpoechtrager/osxcross

And follow there instructions and then you will need to install the clang compiler using this command.

sudo apt-get install clang-3.8

With all this correctly installed you to should be able to build for all three platforms on Ubuntu using Cmake.

I have now also added the ability of using Docker to build for all three platforms.
