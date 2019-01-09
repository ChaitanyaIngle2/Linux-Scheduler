// Chaitanya Ingle, 2000475661, Project 3: Linux Scheduler

#include <iostream>
#include <vector>
#include <string>
#include <stdlib.h>
#include <sstream>

using namespace std;



// *******************************************************************************************************
// *******************************************************************************************************
// DATA STRUCTURES




struct process {

  	int pid; 
  	int start_time; 
  	int end_time; 
  	int priority; 
	vector<int> CPU_bursts; 
  	vector<int> IO_bursts; 
  	 
  	int CPUB_i;
        int IOB_i;
	int arrival_time; 
	int orig_priority; 
  	int timeslice; 
  	int total_CPU_time;      // total clock cycles spent in CPU
  	int total_IO_time;       // total clock cycles spent in IO
  	bool running;            // Variable that records if proc is running
  	
};




// ******************************************************************************************************
// ******************************************************************************************************
// FUNCTION PROTOTYPES



void read_input(vector<process>&);
void submit_to_ready (int, vector<process>&, vector<process>&);
void running_to_waiting(vector<process>&, vector<process>&, int);
void set_running_process(vector<process>&, vector<process>&, int);
int choose_higher_priority (vector<process>&, vector<process>&);
void preempt(vector<process>&, vector<process>&, int, int);
void update_running_process(vector<process>&);
void active_to_expired(vector<process>&, vector<process>&, int);
void update_IO_queue(vector<process>&);
void IO_queue_to_expired (vector<process>&, vector<process>&, int, int);
void check_IO_processes (vector<process>&, vector<process>&, vector<process>&, int);
void move_to_terminated(vector<process>&, vector<process>&, int);
void print_stats(vector<process>);



// ***************************************************************************************************
// ***************************************************************************************************
// MAIN



int main() {

// Declare all queues and variables --------------------

    vector<process> processes;
    vector<process> active_array;
    vector<process> expired_array;
    vector<process> running_proc;
    vector<process> IO_procs;
    vector<process> terminated;
    int highest_priority;

// Linux Scheduler Algorithm (from pdf) ----------------

    read_input(processes); 
    int clock = 0;

    while (true) {
      
    // Populate RQ ------------------------------------------------------------

	submit_to_ready(clock, active_array, processes);

	if (running_proc.size() == 0 && active_array.size() != 0) { 
	  
	    set_running_process(active_array, running_proc, clock);
	}

    // Preempt process if necessary -------------------------------------------
	
	if (running_proc.size() != 0 && active_array.size() != 0) { 
	  
	    highest_priority = choose_higher_priority(active_array, running_proc);
	    if (highest_priority != -1) {
	      
		preempt(active_array, running_proc, highest_priority, clock);
	     }
	}

	update_running_process(running_proc);
	update_IO_queue(IO_procs);

	// Check process progress (bursts)--------------------------------------
	
	if (running_proc.size() != 0) { 
	  
	    if (running_proc[0].CPU_bursts[running_proc[0].CPUB_i] == 0) {
	      
		running_proc[0].CPUB_i++;

        // If  process is completely done --------------------------------------
		
		if (running_proc[0].CPUB_i == running_proc[0].CPU_bursts.size() && running_proc[0].IOB_i == running_proc[0].IO_bursts.size()) 
		  
		    move_to_terminated(running_proc, terminated, clock);

	// Else, move back to RQ -----------------------------------------------
		
	       	else

		  running_to_waiting(running_proc, IO_procs, clock);
	    }
	    
	    if (running_proc[0].timeslice == 0)
	      
		active_to_expired(running_proc, expired_array, clock);
	      
	}

	check_IO_processes(active_array, expired_array, IO_procs, clock); 

     // If all processes have terminated -------------------------------------------

	if (terminated.size() == processes.size()) 
	  
	    break;

     // Change active and expired array pointers if necessary -----------------------
	
	if (active_array.size() == 0 && running_proc.size() == 0 && expired_array.size() != 0) { 
	  
	  active_array.swap(expired_array);
	  cout << "[" << clock << "] *** Queue Swap" << endl;
	}

     // Increment number of clock cycles --------------------------------------------
	
	clock++;
    }

    print_stats(terminated);

    return 0;
}




// ******************************************************************************************************
// ******************************************************************************************************
// FUNCTION DEFINITIONS



// *****************************************************************************************
// void read_input();
// Input: processes
// Results: Will read from input file and populate proccesses list

void read_input(vector<process>& processes) {

// Initializations ---------------------------

    string in_line;
    string token;
    process curr_proc;
    stringstream ss;
    
    int temp;
    int temp2;
    int pc = 0;
    
// Get line of input -------------------------

    while (getline(cin, in_line)) {

     	ss.clear(); 
      	ss << in_line; 
      	ss >> token;
      
      	if (token == "***")
            return;

	istringstream(token) >> temp; 

    // Create + initialize new process --------------

	processes.push_back (curr_proc); 
	processes[pc].running = false; 
	processes[pc].total_IO_time = 0; 
	processes[pc].total_CPU_time = 0; 
	processes[pc].CPUB_i = 0; 
	processes[pc].IOB_i = 0; 
	processes[pc].pid = pc;
	processes[pc].priority = (int)((((temp + 20)/39.0)*30)+.5) +105; 
	processes[pc].orig_priority = processes[pc].priority; 
	temp = processes[pc].priority; 
	processes[pc].timeslice = (int)(((1-(temp/150.0))*395)+.5)+5; 

        ss >> temp;   // start_time
        processes[pc].arrival_time = temp; 
	ss >> temp;   // burst_count

    // Determine number of CPU and IO bursts ---------

	for (int i = 0; i < 2 * temp - 1; i++) {

            if (i%2 == 0) {  
	      
	       	ss >> temp2;
		processes[pc].CPU_bursts.push_back (temp2); 
	    }
            else { 
		
		ss >> temp2;
		processes[pc].IO_bursts.push_back (temp2); 
	    }
	}

	pc++;
    }
    
}

// *********************************************************************************************
// void submit_to_ready()
// Input: clock, active_array, processes
// Results: Processes from "Submit" are added to Active Array in RQ 

void submit_to_ready (int clock, vector<process>& active_array, vector<process>& processes) {
  
	for (int i = 0; i < processes.size(); i++) { 

      		if (processes[i].arrival_time == clock) {

	  		active_array.push_back(processes[i]); //put process in ready queue
	  		cout << "[" << processes[i].arrival_time << "] <" << processes[i].pid << "> Enters the ready queue (Priority: "
			     << processes[i].priority << ", Timeslice: 				" << processes[i].timeslice << ")" << endl;
		}
    	}

}

// ********************************************************************************************
// void assign_CPU()
// Input: active_array, running_proc, clock
// Results: Proccess with highest priority will get CPU.

void set_running_process(vector<process>& active_array, vector<process>& running_proc, int clock) {
  
// Determine process with highest priority --------------------

	int index = 0;
  	int high_prior = active_array[0].priority;
  	int curr = active_array[0].arrival_time;
    
  	for (int i = 1; i < active_array.size(); i++) {
    
      		if (active_array[i].priority < high_prior || (active_array[i].priority == high_prior && active_array[i].arrival_time < curr)) {
	
	  		high_prior = active_array[i].priority;
	  		curr = active_array[i].arrival_time;
	  		index = i;
		}
    	}

// Update running process variables ----------------------------

  	running_proc.push_back(active_array[index]); 

  	if (running_proc[0].running == false) {
      
		running_proc[0].start_time = clock;
     		running_proc[0].running = true;
    	}

  	active_array.erase(active_array.begin() + index); 
  	cout << "[" << clock << "] <" << running_proc[0].pid << "> Enters the CPU" << endl;

}

// *****************************************************************************************
// int choose_higher_priority(active_array, running_proc)
// Input: active_array
// Results: Preempts process in CPU if one in RQ has higher priority

int choose_higher_priority (vector<process>& active_array, vector<process>& running_proc) {

  int index = 0;
  int high_prior = active_array[0].priority;
  
  for (int i = 1; i < active_array.size(); i++) {
    
      if (active_array[i].priority < high_prior) {

	  high_prior = active_array[i].priority;
	  index = i;
	}
  }

  if (active_array[index].priority < running_proc[0].priority) 
      return index; 
  
  else
      return -1;

}

// ****************************************************************************************
// void preempt()
// Input: active_array, running_proc, highest_priority, clock
// Results: Process in CPU will be preempted and process with higher priority will get CPU

void preempt(vector<process>& active_array, vector<process>& running_proc, int highest_priority, int clock) {

  cout << "[" << clock << "] <" << active_array[highest_priority].pid << "> Preempts Process " << running_proc[0].pid << endl;

  if (active_array[highest_priority].running == false) {
    
      active_array[highest_priority].start_time = clock;
      active_array[highest_priority].running = true;
  }

// Update active array + Running process --------- 

  running_proc.push_back(active_array[highest_priority]); 
  active_array.push_back(running_proc[0]); 

  running_proc.erase(running_proc.begin() + 0); 
  active_array.erase(active_array.begin() + highest_priority); 

}

// ****************************************************************************************
// void update_running_process()
// Input: running_proc
// Results: For running_proc, timeslice will be decremented, CPU burst will be decremented,
//          and CPU time will be incremented

void update_running_process(vector<process>& running_proc) {

  if (running_proc.size() != 0) { 
    
      running_proc[0].timeslice = running_proc[0].timeslice - 1; 
      running_proc[0].CPU_bursts[running_proc[0].CPUB_i]--;
      running_proc[0].total_CPU_time++;
  }

}

// ****************************************************************************************
// void update_IO_queue()
// Input: IO_procs
// Results: For all procceses in IO queue, IO burst will be decremented, and IO time will be
//	    incremented

void update_IO_queue(vector<process>& IO_procs) {

  if (IO_procs.size() != 0) {
    
      for (int i = 0; i < IO_procs.size(); i++) {
	
	  IO_procs[i].IO_bursts[IO_procs[i].IOB_i]--; 
	  IO_procs[i].total_IO_time++;
      }
  }

}

// ****************************************************************************************
// void move_to_terminated(running_proc, terminated, clock)
// Input: running_proc
// Results: running_proc will be moved to terminated list if no more CPU or IO
// 	    bursts remain.

void move_to_terminated(vector<process>& running_proc, vector<process>& terminated, int clock) {

  cout << "[" << clock << "] <" << running_proc[0].pid << "> Finishes and moves to the Terminated Queue" << endl;
  running_proc[0].end_time = clock; 
  terminated.push_back(running_proc[0]); 
  running_proc.erase(running_proc.begin() + 0); 

}

// ***************************************************************************************
// void running_to_waiting()
// Input: running_proc, IO_procs, clock
// Results: running_proc will be moved to IO waiting queue

void running_to_waiting(vector<process>& running_proc, vector<process>& IO_procs, int clock) {

  cout << "[" << clock << "] <" << running_proc[0].pid << "> Moves to the IO Queue" << endl;
  IO_procs.push_back(running_proc[0]); 
  running_proc.erase(running_proc.begin() + 0);

}

// ***************************************************************************************
// void active_to_expired()
// Input: running_proc, expired_array, clock
// Results: Moves a process that has finished its timeslice to Expired Array, and makes
//	    necessary updates.

void active_to_expired(vector<process>& running_proc, vector<process>& expired_array, int clock) {

// Calculate bonus ----------------------------------

  int bonus;

  if (running_proc[0].total_CPU_time < running_proc[0].total_IO_time) 
      bonus = (int)(((1 - (running_proc[0].total_CPU_time/(double)running_proc[0].total_IO_time)) * -5) - .5);
 
  else
      bonus = (int)(((1 - (running_proc[0].total_IO_time/(double)running_proc[0].total_CPU_time)) * 5) + .5);
    

// Calculate new priority + timesliece -------------

  running_proc[0].priority = running_proc[0].orig_priority + bonus; 
  running_proc[0].timeslice = (int)((1 - (running_proc[0].priority/150.0)) * 395 + .5 ) +5; 

  cout << "[" << clock << "] <" << running_proc[0].pid << "> Finishes its time slice and moves to the Expired Queue (Priority: "
       << running_proc[0].priority << ", Timeslice:" << running_proc[0].timeslice << ")" << endl;

// Update Active array and CPU ----------------------

  expired_array.push_back(running_proc[0]); 
  running_proc.erase(running_proc.begin()); 

}

// *************************************************************************************
// void IO_to_expired()
// Input: IO_procs, expired_array, clock, index
// Results: An IO_proc will be moved to the expired array

void IO_queue_to_expired (vector<process>& IO_procs, vector<process>& expired_array, int clock, int i) {

// Calculate bonus ----------------------------------------------------

  int bonus;
  if (IO_procs[i].total_CPU_time < IO_procs[i].total_IO_time) 
      bonus = (int)(((1 - (IO_procs[i].total_CPU_time/(double)IO_procs[i].total_IO_time)) * -5) - .5);
    
  else
      bonus = (int)(((1 - (IO_procs[i].total_IO_time/(double)IO_procs[i].total_CPU_time)) * 5) + .5);
    
// Update priority, timeslice, active array, expired array, and CPU ---

  IO_procs[i].priority = IO_procs[i].orig_priority + bonus; 
  IO_procs[i].timeslice = (int)((1 - (IO_procs[i].priority/150.0)) * 395 + .5 ) +5; 

  cout << "[" << clock << "] <" << IO_procs[i].pid << "> Finishes its IO  and moves to the Expired Queue (Priority: " << IO_procs[i].priority
       << ", Timeslice:" << IO_procs[i].timeslice << ")" << endl;

  expired_array.push_back(IO_procs[i]); 
  IO_procs[i].IOB_i++; 
  IO_procs.erase(IO_procs.begin() + i);

}

// ************************************************************************************
// void check_IO_processes()
// Input: active_array, expired_array, IO_procs, clock
// Results: For processes IO waiting queue, if IO burst is done, then if timeslice is complete,
//	    move to expired array, else move back to RQ

void check_IO_processes (vector<process>& active_array, vector<process>& expired_array, vector<process>& IO_procs, int clock) {

  if (IO_procs.size() != 0) {
    
      for (int i = 0; i < IO_procs.size(); i++) {
	
	  if (IO_procs[i].IO_bursts[IO_procs[i].IOB_i] == 0) {
	    
	  // Check if timeslice is complete ---------------------

	      if (IO_procs[i].timeslice == 0) { 
		
		  IO_queue_to_expired(IO_procs, expired_array, clock, i); 
		}

	  // Else -----------------------------------------------

	      else {
		
		  IO_procs[i].IOB_i++; 
		  active_array.push_back(IO_procs[i]); 

		  cout << "[" << clock << "] <" << IO_procs[i].pid << "> Finishes IO and moves to the Ready queue" << endl;

		  IO_procs.erase(IO_procs.begin() + i); 
		} 
	    }
	}
    }

}

// *********************************************************************************
// void print_stats()
// Input: terminated
// Results: Prints out stats of all processes that have terminated

void print_stats(vector<process> terminated) {

  int curr_turn_around;
  int curr_wait_time;
  double curr_CPU_util;
  
  double total_turn_around_time = 0;
  double total_wait_time = 0;
  double total_CPU_util = 0.0; 

  cout << endl << endl << "========================== Overall Performance Output" << endl << "==========================" << endl ;

  for (int i = 0; i < terminated.size(); i++) {

      cout << "PID: <" << terminated[i].pid << "> Statistics:" << endl;
      curr_turn_around = terminated[i].end_time - terminated[i].arrival_time;
      
      curr_wait_time = curr_turn_around - terminated[i].total_IO_time - terminated[i].total_CPU_time;
      curr_CPU_util = terminated[i].total_CPU_time/(double)curr_turn_around * 100;

      cout << "1. Turn around time: " << curr_turn_around << endl
	   << "2. Total CPU time: " << terminated[i].total_CPU_time << endl
	   << "3. Waiting Time: " << curr_wait_time << endl
	   << "4. Percentage of CPU utilization time: " ;
      cout << curr_CPU_util;
      cout << "-------------------------------------------" << endl;

      total_turn_around_time += curr_turn_around;
      total_wait_time += curr_wait_time;
      total_CPU_util += curr_CPU_util;
    }

  total_turn_around_time = total_turn_around_time/(double)(terminated.size());
  total_wait_time = total_wait_time/(double)(terminated.size());
  total_CPU_util = total_CPU_util/(double)(terminated.size());

  cout << "Overall Statistics: " << endl;
  cout << "Average Waiting Time: ";
  cout << total_wait_time << endl;
  cout << "Average Turnaround Time: ";
  cout << total_turn_around_time << endl;
  cout << "Average CPU Utilization: ";
  cout << total_CPU_util << endl;

}
