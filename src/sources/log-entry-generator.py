import pybookwyrm as bw
import time

def find(wanted, bookwyrm):
    while not bookwyrm.terminating():
        bookwyrm.log(bw.loglevel.info, "log entry from " + __file__)
        time.sleep(1)
