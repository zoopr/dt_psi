## Drone-based t-PSI 

This is an example implementation of drone-based threshold-PSI secure multi-party computation protocol. 
It can be imported as a library or tested directly through the compiled binary.

Clone this image with `git clone --recursive [this repo url]` to pull the required submodules.

We recommend you build the Docker image with `./build_docker.sh` and run the container with `./run_all_tests.sh`.

If scripts are not running, make sure to `chmod +x ./*.sh`.

More details on the experiment structure and the rest of the scripts in `ARTIFACT-APPENDIX.md`.

You can also compile from source directly.
Technical requisites:
- `cmake`
- `ninja-build` (recommended, not strictly necessary)
