# Scheduler
Multi-Processor System Simulator


## Features

- Multi-Processor Support: Simulates process execution across multiple processors, allowing for parallel processing and load balancing.



- Configurable Scheduling Algorithms: Implements multiple scheduling algorithms, including:





  First-Come-First-Serve (FCFS)
  
  
  
  Round-Robin (RR) with adjustable time quantum
  
  
  
  Shortest Job First (SJF)
  
  
  
  Priority Scheduling
    
  
  
  Highest-Response-Ratio-Next/HRRN 
  
  
  
  Multilevel Feedback Queue


- Input Flexibility: Accepts input via configuration files or command-line arguments to define processes and system parameters.



- Output Visualization: Provides detailed logs of process execution, including Gantt charts and performance summaries.



- Extensible Design: Modular C++ code allows for easy addition of new scheduling algorithms or system features.



- Cross-Platform Compatibility: Written in standard C++, ensuring compatibility across different operating systems with minimal dependencies.


## Overview

This project is a Multi-Processor System Simulator implemented in C++. It simulates the scheduling of processes across multiple processors, allowing users to experiment with different scheduling algorithms and analyze their performance in a multi-processor environment. The simulator models key aspects of process execution, such as CPU allocation, process states (e.g., ready, running, waiting), and performance metrics like turnaround time, waiting time, and CPU utilization. The goal is to provide a flexible framework for studying and comparing scheduling strategies in a controlled, simulated environment.

The simulator is designed to be extensible, supporting various scheduling algorithms (e.g., First-Come-First-Serve, Round-Robin, Shortest Job First) and configurable system parameters such as the number of processors, process arrival times, and burst times. It is an educational tool for understanding operating system concepts and evaluating the efficiency of scheduling policies.
