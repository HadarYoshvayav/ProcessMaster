# Process Master

## Overview
Process Master is a cross-platform command-line tool that allows users to manage system processes. It provides functionality to list, kill, suspend, resume processes, and more. This tool is designed for users who need to monitor and control processes on their system efficiently.

## Features
- **List Processes**: View all currently running processes with their PID and command line.
- **Kill a Process**: Terminate a process by its PID.
- **Suspend a Process**: Pause a process temporarily.
- **Resume a Process**: Continue a suspended process.
- **Show Process Details**: Display detailed information about a specific process.
- **Start a New Process**: Launch a new process using a command line.
- **Export Processes to CSV**: Save the list of processes to a CSV file for further analysis.
- **Search for a Process**: Find a specific process by its PID.

## Installation
To install Process Manager, clone this repository and compile the code using a C compiler:

```bash
git clone https://github.com/HadarYoshvayav/process-manager.git
cd process-manager
gcc -o process_manager process_manager.c
