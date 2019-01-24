#
# libconfig readable description of a task set with the example task
# set from Baruah '05
#
# Version, required
ts-version = 1.0;

#
#
tasks = (
    {
        name = "t.1";
	period = 8; deadline = 8; wcet = (2);
	threads = 1;
    },
    {
        name = "t.2";
	period = 20; deadline = 10; wcet = (4);
	threads = 1;
    },   
    {
        name = "t.3";
	period = 25; deadline = 15; wcet = (2);
	threads = 1;
    },   
    {
        name = "t.4";
	period = 35; deadline = 30; wcet = (4);
	threads = 1;
    },
    {
        name = "t.5";
	period = 50; deadline = 50; wcet = (3);
	threads = 1;
    },   
    {
        name = "t.6";
	period = 90; deadline = 50; wcet = (4);
	threads = 1;
    },   
    {
        name = "t.7";
	period = 110; deadline = 60; wcet = (8);
	threads = 1;
    },   
    {
        name = "t.8";
	period = 105; deadline = 60; wcet = (5);
	threads = 1;
    },   
    {
        name = "t.9";
	period = 100; deadline = 60; wcet = (3);
	threads = 1;
    },   
    {
        name = "t.10";
	period = 110; deadline = 100; wcet = (4);
	threads = 1;
    }
);
