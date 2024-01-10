# ATM System using Concurrent Processes and IPC Services

This program mimics a simple ATM system using concurrent processes and IPC services.
## Setup

REQUIRED: To compile the program, open the Linux terminal inside the project folder and use the following commands:
```bash
gcc db_server.c semaphore.c msg_queue.c shared_mem.c -o db_server

```
```bash
gcc db_editor.c semaphore.c msg_queue.c shared_mem.c -o db_editor

```
```bash
gcc atm.c -o atm
```

## Execution
To run the db_editor, use the following command:
```bash
./db_editor
```
To run the ATM interface, use the following command in a new terminal:
```bash
./atm
```
Then follow the prompts.


