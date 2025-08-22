module Transmitter
(
    Enable,
    Clock,
    DataIn,
    DataOut,
    Reset
);
    input Clock;
    input Enable;
    input [7:0] DataIn;
    input Reset;
    output DataOut;

    reg [11:0] Packet;
    reg [7:0] DataBuffer;
    reg BusyFlag, PacketReadyFlag;

    assign DataOut = Packet[0]; // the output is the LSB of the Packet shift register

    always @(negedge Reset)
    begin
        if (~Reset)
        begin
            Packet[11:0] <= 12'b111111111111;
            DataBuffer[7:0] <=8'b00000000;
            BusyFlag <= 1'b0;
            PacketReadyFlag <= 1'b0;
        end
    end

    always @(posedge Clock)
    begin
        if (Enable && ~BusyFlag)
        begin
            DataBuffer[7:0] <= DataIn[7:0];
            BusyFlag <= 1'b1;
        end
        else if (BusyFlag && ~PacketReadyFlag)
        begin
            Packet <= {2'b11, ^DataBuffer, DataBuffer[7:0], 1'b0}; // 2 stop bits and even parity
            PacketReadyFlag <= 1'b1;
        end
        else if (BusyFlag && PacketReadyFlag)
        begin
            for (integer LoopIndex = 0; LoopIndex < 11; LoopIndex++)
            begin
                Packet[11] <= 1'b1; // shift in 1's to the MSB
                Packet[LoopIndex] <= Packet[LoopIndex + 1]; // shifts the whole Packet to the right
            end

            if (Packet[11:0] == 12'b111111111111) // when the Packet is sent and the line is idle
            begin
                PacketReadyFlag <= 1'b0;
                BusyFlag <= 1'b0;
            end
        end
    end
endmodule