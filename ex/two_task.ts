# Version, required
ts-version = 1.0;

#
# A simple two task example for use with ts-print
#
# > ts-print ex/two-task.ts
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
