# Version, required
ts-version = 1.0;

#
# This task set is not schedulable by non-preemptive BUNDLEP unless
# t.2 is divided.
#
# Anterior Task Set: { t.1, t.2 x 2 }
# Posterior Task Set: { t.1, t.2, t.2}
#
#
tasks = (
    {
        name = "t.1";
	period = 10; deadline = 5; wcet = (1);
	threads = 1;
    },
    {
	name = "t.2";
	period = 30; deadline = 30; wcet = (3, 6);
	threads = 2;
    }
);
