# ewkino
Framework for high-level physics analysis.

## Introduction
This framework contains tools for high-level EDM data manipulation and analysis.
It is supposed to be run on flat ntuples produced from MiniAOD datasets (notably the ntuples produced with the heavyNeutrino ntuplizer).
Entries from the flat ntuples are read and parsed into an Event instance, allowing easy manipulation and analysis of its physics objects.

## How to use?
When starting a new analysis with this framework, it is advised to make a new branch for this analysis specifically.
Within this branch, make a new folder that will hold all the analysis-specific code, such as the event selection definitions for signal and control regions.
See [the tZq_new branch](https://github.com/LukaLambrecht/ewkino/tree/tZq_new) for an example.

## Future developments
   * Though the framework already contains a lot of very useful tools, some useful functionality is still scattered over the individual branches.
For example: tools for creating combine datacards and running appropriate combine commands. 
Ideally they should be retrieved from the individual branch and put in the master branch for general use.
   * Transitioning from MiniAOD to NanoAOD, the framework should be adapted to be able to run on both types of input.
