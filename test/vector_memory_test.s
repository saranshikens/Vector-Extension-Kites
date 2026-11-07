# Set source and destination addresses
li x10, 256
li x12, 512

# Initialize four 64-bit values in memory
li x11, 11
sd x11, 0(x10)

li x11, 22
sd x11, 8(x10)

li x11, 33
sd x11, 16(x10)

li x11, 44
sd x11, 24(x10)

# Load four lanes into v1
vle64.v v1, (x10)

# Store those lanes at the destination
vse64.v v1, (x12)