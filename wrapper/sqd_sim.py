import signal
import os

IN_SIM = 'SQD_SIM' in os.environ

# If we are in the simulator, we can't write debug messages
# to stdout - we're using stdout for communication with the
# MSP software. Instead, log to some file.
if IN_SIM:
    def push_msg(msg: str):
        if 'SF_WRAPPER_PY_LOGFILE' in os.environ:
            fname = os.environ['SF_WRAPPER_PY_LOGFILE']
        else:
            fname = 'sf_wrapper.log'
        with open(fname, 'a') as f:
            print("[DEBUG]:", msg, file=f)
else:
    def push_msg(msg: str):
        print("[DEBUG]:", msg)

def prevent_ctrlc_in_sim():
    if IN_SIM:
        def handler(signum, frame):
            push_msg('Ignoring SIGINT in sim')
        signal.signal(signal.SIGINT, handler)
