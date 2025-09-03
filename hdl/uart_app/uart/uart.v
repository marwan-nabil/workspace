`include "baud_generator.v"
`include "transmitter.v"
`include "receiver.v"

module UART
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
    input Rx;
    input SendCommand;
    input Clock;
    input HostAcknowledge;
    input Reset;
    input [1:0] BaudSelection;
    input [7:0] InputData;

    output [7:0] OutputData;
    output HostInterrupt;
    output Tx;
    output Tick;
    output [2:0] Errors;

    // internal wires
    wire Tick;
    wire OverSamplingTick;

    BaudGenerator BaudGeneratorInstance
    (
        .Clock(Clock),
        .BaudrateSelector(BaudSelection),
        .Tick(Tick),
        .SamplingTick(OverSamplingTick)
    );

    Transmitter TransmitterInstance
    (
        .Enable(SendCommand),
        .Clock(Tick),
        .DataIn(InputData),
        .DataOut(Tx),
        .Reset(Reset)
    );

    Receiver ReceiverInstance
    (
        .DataIn(Rx),
        .DataOut(OutputData),
        .OverSamplingClock(OverSamplingTick),
        .ErrorOut(Errors),
        .HostInterrupt(HostInterrupt),
        .HostAcknowledge(HostAcknowledge),
        .Reset(Reset)
    );
endmodule