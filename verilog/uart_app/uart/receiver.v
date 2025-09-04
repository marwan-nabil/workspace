module Receiver
(
    DataIn,
    DataOut,
    OverSamplingClock,
    ErrorOut,
    HostInterrupt,
    HostAcknowledge,
    Reset
);
    input DataIn;
    input OverSamplingClock;
    input HostAcknowledge;
    input Reset;

    output [7:0] DataOut;
    output [2:0] ErrorOut;
    output HostInterrupt;

    reg StartPhaseFlag;
    reg DataReadyPhaseFlag;
    reg HostAcknowledgedFlag;
    reg HostInterrupt;

    reg [11:0] InputLineShiftRegister;
    reg [3:0] BitCounter;
    reg [3:0] LoopIndex;
    reg [4:0] OversamplingCounter;
    reg [7:0] ResultRegister;
    reg [2:0] ErrorOut;

    assign DataOut = ResultRegister[7:0];

    always @(negedge Reset)
    begin
        if (~Reset)
        begin
            StartPhaseFlag <= 1'b1;
            DataReadyPhaseFlag <= 1'b0;
            HostInterrupt <= 1'b0;
            HostAcknowledgedFlag <= 1'b0;
            InputLineShiftRegister <= 12'b000000000000;
            BitCounter <= 4'b0000;
            LoopIndex <= 4'b0000;
            OversamplingCounter <= 5'b00000;
            ResultRegister <= 8'b00000000;
            ErrorOut <= 3'b000;
        end
    end

    // first stage, phase locking the OverSamplingClock with the middle of the start bit
    always @(posedge OverSamplingClock)
    begin
        if ((OversamplingCounter == 7) && StartPhaseFlag)
        begin
            StartPhaseFlag <= 1'b0; // ensures that the first stage occurs one time only
            InputLineShiftRegister[11]  <=  DataIn; // start bit entered the shift register
            BitCounter  <=  BitCounter + 1;
            OversamplingCounter  <= 5'b00000;
        end
        else if (~DataIn && StartPhaseFlag)
        begin
            OversamplingCounter <= OversamplingCounter + 1;
        end
    end

    always @(posedge OverSamplingClock)
    begin
        if (BitCounter == 12)
        begin
            // this stage would be stuck here until the
            // OversamplingCounter is Reset by the next stage
            DataReadyPhaseFlag <= 1'b1;
        end
        else if (~StartPhaseFlag && (OversamplingCounter < 16))
        begin
            OversamplingCounter <= OversamplingCounter + 1;
        end
        else if (~StartPhaseFlag && (OversamplingCounter == 16))
        begin
            for (LoopIndex = 0; LoopIndex < 11; LoopIndex++) // shifts the register to the right
            begin
                InputLineShiftRegister[LoopIndex] <= InputLineShiftRegister[LoopIndex + 1];
            end

            InputLineShiftRegister[11] <= DataIn; //shift in the new MSB
            BitCounter <=  BitCounter + 1;
            OversamplingCounter <= 5'b00000;
        end
    end

    always @(posedge OverSamplingClock)
    begin
        if (DataReadyPhaseFlag) //this stage won't start until this flag is set
        begin
            ResultRegister[7:0] <= InputLineShiftRegister[8:1];
            BitCounter <= 4'b0000; //allowing the previous stage to start again
            StartPhaseFlag <= 1'b1;  //allowing the first stage to start again
            HostInterrupt <= 1'b1; //interrupting the host processor to fetch the data word
            DataReadyPhaseFlag <= 1'b0; //ensuring this stage occurs only once
            ErrorOut[2] <= ^InputLineShiftRegister[8:1]; //signals a parity error, even parity is considered here
        end
    end

    always @(posedge OverSamplingClock) //data overrun error detection
    begin
        if (HostInterrupt && HostAcknowledgedFlag)
        begin
            HostInterrupt <= 1'b0;
            HostAcknowledgedFlag <= 1'b0;
        end
        else if (HostInterrupt && DataReadyPhaseFlag) // the next data word is ready and the host hasn't aknowledged the previous one
        begin
            ErrorOut[0] <= 1'b1; // data overrun, host failed to fetch in time
        end
    end

    always @ (posedge HostAcknowledge) // the host processor sends a pulse aknowledging the arrival of data
    begin
        HostAcknowledgedFlag <= 1'b1;
    end
endmodule