# mips_pipelined_processor

A simulation of a 5-stage MIPS processor with pipelining is created using C++ and computer architecture concepts.

## Overview

- An input file is taken, parsed, and structured.

- Data hazards are handled by looking at previous instructions in the simulation, and checking for register dependencies. If the current instruction has depencies on previous instructions that have yet to reach the WB stage, it will stall.

- Control hazards are handled by making a prediction (branch is taken) until the branching decision is found. If the branch is not executed, then the instructions initiated after the branching prediction are flushed.

- An output file containing the clock cycles (with the instruction's current stage), registers, and memory.

## Comments

- The .pptx and .docx files contain more in-depth information on the project.