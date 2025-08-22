`timescale 1ns/1ns
`include "baud_generator.v"

module testbench;
    reg Clock;
    reg [1:0] BaudRateSelector;
    wire Tick;
    wire SamplingTick;

    BaudGenerator BaudGeneratorInstance
    (
        Clock,
        BaudRateSelector,
        Tick,
        SamplingTick
    );

    initial
    begin
        $dumpfile("uart_app.vcd");
        $dumpvars(0, testbench);

        BaudRateSelector = 2'b11;
        Clock = 0;

        for (integer LoopIndex = 0; LoopIndex < 1000000; LoopIndex++)
        begin
            #1 Clock = ~Clock;
        end

        $display("test finished..");
    end
endmodule