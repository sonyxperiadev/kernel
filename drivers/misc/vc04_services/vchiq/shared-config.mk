##############################################################################
#
#  This makefile snippet contains definitions which are shared between
#  the host and the videocore.
#
#  As such, the defines need to fairly simple and should make minimal 
#  assumptions about the build system being used.
#
##############################################################################

#
# VCHIQ_FLAVOUR - Determines which vchiq_implementation is being used. This
#                 is expexted to be the name of a directory contained
#                 within the interface directory.
#
VCHIQ_FLAVOUR := vchiq_arm

#
# VCHIQ_PLATFORM - For the vchiq_arm flavour on Big Island, chooses which
#                  processor is the master (handles the bulk transfers):
#                    bi    - ARM is the master
#                    bivcm - VC is the master
#
VCHIQ_PLATFORM := bivcm

#
# VCHIQ_SM_ALLOC - Where is the shared memory used by vchiq allocated from:
#                    ram   - shared RAM, this is the default Big Island mode
#                    vcddr - videocore DDR, this is the default Capri mode, can be
#                            used on Big Island for testing/verification 
#
VCHIQ_SM_ALLOC := ram
