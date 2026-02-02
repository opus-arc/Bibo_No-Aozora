# Project Documentation & DevLog

This directory contains the algorithm specifications, mathematical principles, and development logs for the Ear Training Project.

## Algorithms & Specifications
| Date       | Version | Document | Description |
|:-----------| :--- | :--- | :--- |
| 2026-01-27 | v1.0 | [SSP-MMC Algorithm Spec v1.0](introduction/algorithms/2026-01-27_SSP-MMC_Spec_v1.0.md) | Implementation guide for the DHP model (Ear Training adapted) and CSV lookup logic. |
| 2026-01-22 | - | [Original Paper (Maimemo)](introduction/algorithms/references/maimemo.pdf) | Reference paper: "A Stochastic Shortest Path Algorithm for Optimizing Spaced Repetition". |

## DevLog (Development Log)
> Tracking the evolution of our core logic and engineering decisions.

* **[2026-01-27]** Migration Strategy & Cold Start: Adapted the vocabulary model for music intervals; introduced scaling factor $K$ with a wild guess.
* **[2026-01-25]** Project Initialization: Decided to use Offline CSV Lookup instead of online Value Iteration.