#
# libconfig readable description of a task set with one task
#

#
# Structure of the task set
#
# tasks = (
#   { <TASK_1> },
#   { <TASK_2> },
#   ...
#   { <TASK_N> }
# );
#
# tasks
#   Required top level list container of all tasks
#
# Individual tasks are arrays with the following required attributes
#   name = <STRING> the name of the task
#
#   period = <INT> the minimum inter arrival time of the task
#
#   deadline = <INT> the relative deadline of the task
#
#   threads = <INT> the number of threads released with each job
#
#   wcet = <LIST OF INTS> the WCET of an increasing number of threads,
#     the wcet list must contain *exactly* the same number of threads
#     that are listed in the "threads" attribute. Values in the list
#     are the total execution time for the number of threads in the
#     index.
#
#     For example, if "threads = 3;" and "wcet = (10, 15, 17)" the
#     WCET to complete 1 thread  is 10 units, the
#     WCET to complete 2 threads is 15 units, the
#     WCET to complete 3 threads is 17 units
#
#     In the degenerate case of one thread, "threads = 1;" is required
#     as is the WCET of one thread, ie "wcet = (10);".
#
tasks = (
    {
        name = "t.1";
	period = 8;
	deadline = 8;
	threads = 1;
	wcet = (2);
    }
);
