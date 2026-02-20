[![License: Apache-2.0](https://img.shields.io/badge/License-Apache--2.0-green?label=Licence)](https://github.com/Open-CMSIS-Pack/lwIP/blob/main/LICENSE)
[![Pack build](https://img.shields.io/github/actions/workflow/status/Open-CMSIS-Pack/lwIP/pack.yml?logo=arm&logoColor=0091bd&label=Build%20pack)](https://github.com/Open-CMSIS-Pack/lwIP/tree/main/.github/workflows/pack.yml)


# lwIP
CMSIS Pack for the lwIP - A Lightweight TCP/IP stack


## Repository top-level structure
Directory                   | Description
:---------------------------|:--------------
[.github/workflows](https://github.com/Open-CMSIS-Pack/lwIP/tree/main/.github/workflows)  | [GitHub Actions](#github-actions).
[examples](https://github.com/Open-CMSIS-Pack/lwIP/tree/main/examples)                    |  Contains examples
[ports](https://github.com/Open-CMSIS-Pack/lwIP/tree/main/ports)                          |  Contains ports for CMSIS RTOS and FreeRTOS as well as no RTOS
[rte](https://github.com/Open-CMSIS-Pack/lwIP/tree/main/rte)                              |  Contains configuration files for the runtime environment


## GitHub Actions

The repository uses GitHub Actions to generate the pack:

- `.github/workflows/pack.yml` based on [lwIP/gen-pack-action](https://github.com/Open-CMSIS-Pack/lwIP) generates pack using the [Generate software pack](#generate-software-pack) scripts.


## Generate software pack

The software pack is generated using bash shell scripts.

- `./gen_pack.sh` based on [Open-CMSIS-Pack/gen-pack](
https://github.com/Open-CMSIS-Pack/gen-pack)) generates the software pack. Run this script locally with:

      lwIP $ ./gen_pack.sh


## Issues

Please feel free to raise an [issue on GitHub](https://github.com/Open-CMSIS-Pack/lwIP/issues)
to report misbehavior (i.e. bugs) or start discussions about enhancements. This
is your best way to interact directly with the maintenance team and the community.
We encourage you to append implementation suggestions as this helps to decrease the
workload of the maintenance team.