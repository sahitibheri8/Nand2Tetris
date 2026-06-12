# Nand2Tetris: Building a Computer from First Principles

This repository contains my implementation of the Nand2Tetris (The Elements of Computing Systems) course, where I built a complete computer system from the ground up—starting with basic logic gates and ending with software tools capable of executing high-level programs.

## What I Built

### Hardware Layer

* Basic logic gates using NAND as the primitive building block
* Arithmetic circuits including Adders and ALU
* Sequential components such as Registers and Program Counter
* Memory hierarchy including RAM8, RAM64, RAM512, RAM4K, and RAM16K
* Complete 16-bit Hack CPU and Computer

### Software Layer

* Hack Assembly programs (Mult, Fill)
* Two-pass Assembler written in C
* VM Translator written in C++ supporting:

  * Stack arithmetic
  * Memory access commands
  * Program flow commands
  * Function definitions
  * Function calls and returns
  * Bootstrap initialization
  * Multi-file VM programs

## Key Concepts Learned

* Computer Architecture
* Digital Logic Design
* CPU Design
* Memory Organization
* Assembly Language Programming
* Compiler and Translator Construction
* Stack-Based Virtual Machines
* Systems Programming

## Tech Stack

* HDL (Hardware Description Language)
* C
* C++
* Hack Assembly Language

This project provided end-to-end exposure to how modern computing systems are constructed, from elementary logic gates to software translation tools and executable machine code.
