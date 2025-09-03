`timescale 1ns/1ns
`include "demo.v"

module testbench;
    reg A;
    wire B;
    demo device_under_test(A, B);

    initial
    begin
        $dumpfile("demo.vcd");
        $dumpvars(0, testbench);

        A = 0;
        #20

        A = 1;
        #20

        A = 0;
        #20

        $display("test finished..");
    end
endmodule