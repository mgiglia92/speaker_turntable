clear
c = Comms()
c.ser = serialport('COM4', 115200)

while true

    data = c.read_inc();
    [id, data] = c.separate_packet(data);
    if id == 9
        typecast(data, 'int32')
    end
    ps =c.create_packet(5, 1);
    c.send(ps);
    ps =c.create_packet(7, -101);
    c.send(ps);
    ps = c.create_packet(9, -100);
    c.send(ps);
    pause(1);
end

clear c