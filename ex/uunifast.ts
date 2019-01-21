#
# tasks -- top level container for all tasks
#
tasks = (
    {
	name = "t.1";
	period = 8;
	deadline = 8;	# ignored
	threads = 0;	# indicates WCET will be changed
	wcet = ();	# will be modified
    },
    {
	name = "t.2";
	period = 0;		# indicates period will be changed
	deadline = 8;		# ignored
	threads = 3;		# must be non-zero
	wcet = (6, 8, 10);	# WCET of 10 used
    }
);
