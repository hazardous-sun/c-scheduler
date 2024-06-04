# Preemptive Priority Scheduling Algorithm

This project implements a simulation Preemptive Priority Scheduling algorithm in C.

## Requirements

- A text file named "processes.txt" containing process information in the following format:
  - The first line specifies the meaning of each data point separated by commas. (e.g., PID, ARRIVAL, BURST, PRIORITY)
  - Subsequent lines list each process with its data points separated by commas. (e.g., 1,0,10,2)
  - No blank lines are allowed at the end of the file.

## Example processes.txt:

```
PID, ARRIVAL, BURST, PRIORITY
1,0,10,2
2,2,5,1
3,4,8,3
4,5,6,1
5,3,3,2
```

## Running the Program

1. Compile the C code. The specific compiler command will vary depending on your environment.
2. Execute the compiled program.

### Example (Linux/macOS)
```Bash
gcc -o scheduler scheduler.c
./scheduler
```

## Output

The program will:

1. Print a list of the processes sorted by priority.
2. Simulate the scheduling of the processes using the Preemptive Priority Scheduling algorithm.
3. Print the final state of the processes with their remaining burst times set to zero.
4. Print the wait time information for each process.
5. Calculate and print the average wait time for all processes.

## Additional Notes

- The sleep(1); line in the code has been commented out. This line was used to introduce a one-second delay between scheduling steps for demonstration purposes. You can uncomment this line if you wish to slow down the output for better visualization.
