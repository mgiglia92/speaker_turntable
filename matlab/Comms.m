classdef Comms
    %UNTITLED4 Summary of this class goes here
    %   Detailed explanation goes here

    properties
        ser
        outbound
        inbound
    end

    methods
        function [output] = deserialize(obj, bytes)
    
            output = typecast(bytes, 'uint32');
        end
        
        function [output] = serialize(obj, num)
            n = typecast(int32(num), "uint32");
            output = typecast(n, 'char');
        end

        function [packet_string] = create_packet(obj, id, data)
            id_bytes = obj.serialize(id);
            id_b64 = matlab.net.base64encode(id_bytes);
            data_bytes = obj.serialize(data);
            data_b64 = matlab.net.base64encode(data_bytes);
            packet_string = append("#", id_b64, "$", data_b64, "%", data_b64, "&");
            
        end

        function [fail] = send(obj, packet_string)
            write(obj.ser, packet_string, 'char');
            fail=0;
        end

        function [data] = read_inc(obj)
            configureTerminator(obj.ser, int8('#'));
            predata = readline(obj.ser);
            configureTerminator(obj.ser, int8('&'));
            data = readline(obj.ser);
        end

        function [id_dec, data_dec] = separate_packet(obj, chars)
            ret         = split(chars, '$');
            id_b64      = ret(1);
            rest        = ret(2);
            ret         = split(rest, '%');
            data_b64    = ret(1);

            id_bytes    = matlab.net.base64decode(id_b64);
            data_bytes  = matlab.net.base64decode(data_b64);

            id_dec      = obj.deserialize(id_bytes);
            data_dec    = obj.deserialize(data_bytes);
            id_dec = typecast(id_dec, 'int32');
            data_dec = typecast(data_dec, 'int32');
        end
    end
end