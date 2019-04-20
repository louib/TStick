import mapper

dev = mapper.device("t-Stick173")
sensor1 = dev.add_output_signal("rawpressure", 3, 'f', "V", 0.0, 5.0)

while 1:
    dev.poll(50)
    # ... update signals ...
