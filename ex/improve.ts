# Version, required
ts-version = 1.0;

#
# A simple two task example to illustrate the pessimism in the
# non-preemptive chunks algorithm from Baruah 2005.
#
# > maxchunks ex/improve.ts
#
# Will not find non-preepmtive chunk size for task 2
#
# > tpj ex/improve.ts
#
# Will find a non-preemptive chunk size for task 2 of 4, the entire WCET.
#
tasks = (
    {
        name = "t.1";
	threads = 1;
	period = 8; deadline = 8; wcet = (4);
    },
    {
	name = "t.2";
	threads = 1;
	period = 16; deadline = 16; wcet = (4);
    }
);
