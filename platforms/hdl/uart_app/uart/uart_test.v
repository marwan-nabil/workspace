`timescale 1ns/1ns
`include "uart.v"

module testbench;
    reg Reset = 0;
    reg [1:0] BaudSelection = 2'b11;
    reg [7:0] InputData = 8'b00000000;
    reg Rx = 0;
    reg Clock = 0;
    reg SendCommand = 0;
    reg HostAcknowledge = 0;

    wire [7:0] OutputData;
    wire [2:0] Errors;
    wire Tx;
    wire HostInterrupt;
    wire Tick;

    UART UARTInstance
    (
        Reset,
        BaudSelection,
        InputData,
        OutputData,
        Errors,
        Rx,
        Tx,
        Clock,
        HostInterrupt,
        SendCommand,
        HostAcknowledge,
        Tick
    );

    reg [7:0] TransmissionBuffer = 8'hFF;

    always #1 Clock = ~Clock;

    initial
    begin
        $dumpfile("uart_app.vcd");
        $dumpvars(0, testbench);

        Reset = 1;
        #1
        Reset = 0;

        InputData[7:0] <= TransmissionBuffer[7:0];

        #2
        SendCommand = 1;
        #5000

        $display("test finished...");
        $finish();
    end
endmodule