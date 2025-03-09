# SchrodingerSim

SchrodingerSim is a application to simulate quantum wave function evolution for any time-independent potential that is defined within a well. That is, all wavefunctions have boundary conditions that $\psi(x=0) = \psi(x=L) = 0$. The application displays the evolution of the probability wavefunction.

## Installation

SchrodingerSim is built on clang 15.0.0. Make sure your system has clang and is accessible through the path. There is no support at the moment for Windows systems.

SchrodingerSim heavily depends on [Raylib](https://github.com/raysan5/raylib) for the display and interactivity of the simulation. Furthermore, this repo contains an implementation of a HashMap in C written by [tidwall](https://github.com/tidwall/hashmap.c).

### To Run

Clone this repository *recursively* with `git clone --recursive https://github.com/akpiya/1DSchrodingerSim.git` (raylib is used as a submodule here).

## Usage & Controls

After the steps above, simply run `make run` in the root directory.

**Controls**:

|Key|Action| 
|---| --- |
|Space|Pause/Resume|
|Right-Click + Drag | Pan Camera |
|Left-Click | Select/Draw |
|Scroll Wheel | Zoom |

## Demo
Here is a demo of the eigenstate solver and interactive gui to display the eigenstates for arbitrary potential functions.

https://github.com/akpiya/1DSchrodingerSim/assets/48157425/8709b22b-44e0-422c-8fa1-04d10f16d422

Below are some sample images. The black line represents the potential while the colored lines are some of the corresponding eigenstates.

![Quad](assets/quadratic.png)
*First 3 eigenstates squared of a quadratic potential*

![User](assets/usr-def.png)
*First 3 eiegenstates squared of a user-specified, drawn potential. Each eigenstate is most likely to be found in some potential well*

## Authors
Akash Piya
