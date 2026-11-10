"""Keep compiler memory usage low, including builds started from the IDE."""
from SCons.Script import SetOption

SetOption("num_jobs", 1)
