import pybookwyrm as bw
import time

def find(wanted, bookwyrm):
    i = 0
    while True:
        bookwyrm.log(bw.log_level.debug, "log entry from " + __file__ + " " + str(i))
        i += 1
        time.sleep(0.5)
