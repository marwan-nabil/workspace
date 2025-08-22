`timescale 1ns/1ns
`include "receiver.v"

module testbench;
    reg OverSamplingClock;
    reg DataIn;
    reg HostAcknowledge;
    reg Reset;

    wire [7:0] DataOut;
    wire [2:0] ErrorOut;
    wire HostInterrupt;

    Receiver ReceiverInstance
    (
        DataIn,
        DataOut,
        OverSamplingClock,
        ErrorOut,
        HostInterrupt,
        HostAcknowledge,
        Reset
    );

    reg [31:0] TransmissionBuffer;

    initial OverSamplingClock = 0;
    always #1 OverSamplingClock = ~OverSamplingClock;

    initial
    begin
        $dumpfile("uart_app.vcd");
        $dumpvars(0, testbench);

        Reset = 1;
        #1
        Reset = 0;

        TransmissionBuffer[31:0] = 32'b00000000000000000000000010101010;

        for (integer LoopIndex = 0; LoopIndex < 32; LoopIndex++)
        begin
            DataIn <= TransmissionBuffer[LoopIndex];
            #32;
        end

        #50

        $display("test finished...");
        $finish();
    end
endmodule