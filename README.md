# Integrated Itinerary and Fare Engine - Metro System of the Valle de Aburra

Practice 2 - SI1002-7 Languages and Paradigms of Computation (2026-2), Universidad EAFIT.

## Team members

- Matías Palacio Gómez
- Natalia Vergara Mendoza

## What the program does

It charges a passenger for a journey made of several legs (metro, tram, cable, tourist cable and feeder route). The fare depends on the mode, on the passenger profile and on an integration rule: the first payment opens a 90-minute window and up to two transfers inside it are free.

## How to build and run

```
g++ -std=c++17 -Wall -Wextra -o practice main.cpp
./practice
```

The build produces zero warnings. The program reads nothing from standard input.

## Project structure

Everything is in one file, `main.cpp`:

- Part 1: `TransportMode` (abstract) and its five modes.
- Part 2: `Passenger` (Non-Virtual Interface) and its five profiles.
- Part 3: `Network` (owns the modes with `std::unique_ptr`) and `chargeItinerary`.
- Part 4: five report functions built with STL algorithms.

## Written answers to the defence questions

PENDIENTE: confirm with the professor which is the second defence question. Question 1 is named in the practice PDF; the other two answers below are the likely candidates.

**Question 1. What happens if the destructor of `TransportMode` is not virtual?**
The modes are stored as `std::unique_ptr<TransportMode>`, so they are destroyed through a pointer to the base class. Without a virtual destructor only the base destructor runs, which is undefined behaviour: the derived part of the object is not destroyed properly. With a virtual destructor the derived destructor runs first and then the base one.

**Question 2 (candidate A). Why an abstract class with pure virtual functions instead of an enum plus a switch?**
With an enum and a switch, every new mode forces us to open and modify several places. With polymorphism a new mode is one new class and nothing else changes. It also avoids asking an object what type it is, which is forbidden by Rule A.

**Question 2 (candidate B). What does the STL give us over a hand-written loop?**
Each algorithm says what is done (`accumulate` adds, `count_if` counts) instead of how the container is traversed, so the code is shorter, easier to read and has fewer chances of off-by-one mistakes. A lambda keeps the small rule next to the call, and `std::function` lets the caller inject a criterion without the network knowing it.
