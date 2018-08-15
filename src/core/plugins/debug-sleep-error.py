# Sleep for a while and then throw an exception
import pybookwyrm as bw
import time

def find(wanted, bookwyrm):
    bookwyrm.log(bw.log_level.debug, "inside " + __file__)
    raise ValueError('Something very bad happened')
