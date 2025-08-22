module controller (reading,shut_down,alarm,clk,reset);
  input clk,reset;
  input [7:0] reading;
  output shut_down,alarm;

  reg shut_down,alarm;
  reg [1:0] state;
  
  always @(posedge clk,negedge reset)
    begin
      if(~reset) state <= 2'b00;
      else if((reading >=192 )&&(reading <= 240 )) state <= 2'b01;
      else if(reading >240 ) state <= 2'b10;
      else state <= 2'b00;
    end

  always @(state)
    begin
      case(state)
        2'b00 : begin shut_down <=1'b0 ; alarm <= 1'b0 ; end //reset
        2'b01 : begin shut_down <=1'b0 ; alarm <= 1'b1 ; end //alarm
        2'b10 : begin shut_down <=1'b1 ; alarm <= 1'b1 ; end //shutdown
        default : begin shut_down <=1'b0 ; alarm <= 1'b0 ; end
      endcase
    end
endmodule