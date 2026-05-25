## Drone-based t-PSI 

This is an example implementation of drone-based threshold-PSI secure multi-party computation protocol. 
It can be imported as a library or tested directly through src/main.cc

Technical requisites:
- `cmake`
- `ninja-build` (recommended, not strictly necessary)

To only run the timing experiments:
- Edit src/main.cc with the desired parameters.
- Build the new application binary by navigating to the build/ folder, then running `cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..`, followed by `ninja`
- Run `./app`.
