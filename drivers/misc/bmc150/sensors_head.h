#ifndef ___SENSORS_HEAD_H_INCLUDED
#define ___SENSORS_HEAD_H_INCLUDED

#define PROBE_SUCCESS 1
#define PROBE_FAIL	  0
extern unsigned int board_hw_revision;


int sensors_register(struct device **dev,
	void *drvdata, struct device_attribute *attributes[],
	char *name);
void sensors_unregister(struct device *dev,
	struct device_attribute *attributes[]);

#endif

