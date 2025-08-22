module BaudGenerator(Clock, BaudrateSelector, Tick, SamplingTick);
    input Clock;
    input [1:0] BaudrateSelector;
    output Tick;
    output SamplingTick;

    reg Tick;
    reg SamplingTick;
    reg [1:0] BaudRateState;
    reg [11:0] Counter;
    reg [3:0] TickCounter;

    // BaudrateSelector value mapping:
    //     00 == 1200
    //     01 == 2400
    //     10 == 4800
    //     11 == 9600

    initial
    begin
        Counter = 12'b000000000000;
        TickCounter = 4'b0000;
        BaudRateState = 2'b00;
        SamplingTick <= 1'b0;
        Tick <= 1'b0;
    end

    always @(BaudrateSelector)
    begin
        BaudRateState <= BaudrateSelector;
        SamplingTick <= 1'b0;
        Tick <= 1'b0;
        Counter <= 12'b000000000000;
        TickCounter <= 4'b0000;
    end

    always @(posedge Clock)
    begin
        case (BaudRateState)
            2'b00: // 1200
            begin
                if (Counter == 12'h516)
                begin
                    SamplingTick <= ~SamplingTick;
                    Counter <= 12'b000000000000;
                end
                else
                begin
                    Counter <= Counter + 1;
                end
            end

            2'b01: // 2400
            begin
                if (Counter == 12'h28B)
                begin
                    SamplingTick <= ~SamplingTick;
                    Counter <= 12'b000000000000;
                end
                else
                begin
                    Counter <= Counter + 1;
                end
            end

            2'b10: // 4800
            begin
                if (Counter==12'h145)
                begin
                    SamplingTick <= ~SamplingTick;
                    Counter <= 12'b000000000000;
                end
                else
                begin
                    Counter <= Counter+1;
                end
            end

            2'b11: // 9600
            begin
                if (Counter==12'hA2)
                begin
                    SamplingTick <= ~SamplingTick;
                    Counter <= 12'b000000000000;
                end
                else
                begin
                    Counter <= Counter+1;
                end
            end
        endcase
    end

    always @ (posedge SamplingTick)
    begin
        TickCounter <= TickCounter + 1;
        Tick <= TickCounter[3];
    end
endmodule