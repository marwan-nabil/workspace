`timescale 1ns/1ns
`include "transmitter.v"

module testbench;
    reg Enable = 0;
    reg Clock = 0;
    reg [7:0] DataIn = 0;
    reg Reset = 0;

    wire DataOut;

    Transmitter TransmitterInstance
    (
        Enable,
        Clock,
        DataIn,
        DataOut,
        Reset
    );

    reg [7:0] TransmissionBuffer = 8'hFF;

    // initial Clock = 0;
    always #1 Clock = ~Clock;

    initial
    begin
        $dumpfile("uart_app.vcd");
        $dumpvars(0, testbench);

        Reset = 1;
        #1
        Reset = 0;

        DataIn[7:0] <= TransmissionBuffer[7:0];

        #2
        Enable = 1;
        #10
        Enable = 0;
        #500

        $display("test finished...");
        $finish();
    end
endmodule