#
# This task set is not schedulable by non-preemptive BUNDLEP unless
# t.1 is divided.
#
# For brevity, the task 1 with four threads is named t.1:4. If the task
# is divided into two tasks, one with three threads and one with one
# they are named: t.1:3, t.1:1
#
# Original task set = {t.1:2, t.2:1}
# Non-preemptive schedulable task set = {t.1:1, t.1:1, t2:1}
#
tasks = (
    {
        name = "t.1";
	period = 30; deadline = 30; wcet = (8, 13);
	threads = 2;
    },
    {
	name = "t.2";
	period = 30; deadline = 30; wcet = (5);
	threads = 1;
    }
);
