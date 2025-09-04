module digital_clock (set_min, set_hr, set_day, set_month,tstamp,clk,enable_pulse,ticker,set_date_time);
  output  enable_pulse;  //enable pulse is sent every 15 min. to Tx module
  output [25:0] tstamp;
  input [5:0] set_min;
  input [4:0] set_hr, set_day;
  input [3:0] set_month;
  input set_date_time,clk,ticker; //ticker is connected to baud generator
  
  reg [25:0] clk_c; // we need 26 bits to represent the number 50 million
  reg [5:0] sec_c , min_c;
  reg [4:0] hr_c,day_c;
  reg [3:0] mon_c;
  reg [1:0] tick_c;
  reg enable_pulse,tick_flag ;
  
  assign tstamp ={mon_c,day_c,hr_c,min_c,sec_c}; // timestamp should be decoded externally, width is 26 bits
  
  // enable pulse configuration
  always @ (posedge ticker)  // enable pulse should have a width of 2 tick pulses
  begin
  if (tick_c==2)
    begin 
      tick_c <= 2'b00 ;
      enable_pulse <=1'b0;
      tick_flag <=1'b1;   // tick flag is reset every minute
    end
  if ((min_c==1 || min_c==15 || min_c==30 || min_c==46) && ~tick_flag )
    begin
      enable_pulse <=1'b1;
      tick_c <= tick_c+1;
    end
  end
  
  
  always@(posedge clk)
  begin
    if (~set_date_time)
      begin
        clk_c <= {26*(1'b0)};
        sec_c <= {6*(1'b0)};
        min_c <= set_min;
        hr_c <= set_hr;
        day_c <= set_day;
        mon_c <= set_month;
        enable_pulse <=1'b0;
      end
    else
      
    begin
      
    if(clk_c < 26'h2FAF080)
      clk_c <= clk_c +1 ;
    else 
      begin 
        clk_c <= {26*(1'b0)} ;
        sec_c <= sec_c +1 ;
      end
    
    if(sec_c == 60 && min_c != 59) //once every minute
      begin 
        min_c <= min_c + 1 ;
        tick_flag <= 1'b0;  // the tick flag would reset every minute 
        sec_c <= {6*(1'b0)} ;  // there would be an error of 1 over 50 millionth of a second each second
      end
      
    if (min_c == 59 && sec_c == 60 && hr_c != 23) //once every hour
      begin
        hr_c <= hr_c + 1 ;
        min_c <= {6*(1'b0)} ;
        tick_flag <= 1'b0;
        sec_c <= {6*(1'b0)} ;
      end
      
    if(hr_c == 23 && min_c == 59 && sec_c == 60 && day_c !=29 ) //once every day
      begin
        day_c <= day_c +1 ;
        hr_c <= {5*(1'b0)} ;
        min_c <= {6*(1'b0)} ;
        tick_flag <= 1'b0;
        sec_c <= {6*(1'b0)} ;
      end
    if(day_c==29 && hr_c == 23 && min_c == 59 && sec_c == 60 && mon_c !=11) //once every month
      begin 
        mon_c <= mon_c +1;
        day_c <= {5*(1'b0)} ;
        hr_c <= {5*(1'b0)} ;
        min_c <= {6*(1'b0)} ;
        tick_flag <= 1'b0;
        sec_c <= {6*(1'b0)} ;
      end
    
    if(mon_c==11 && day_c==29 && hr_c == 23 && min_c == 59 && sec_c == 60) //once every year
      begin
        mon_c <= {4*(1'b0)};
        day_c <= {5*(1'b0)} ;
        hr_c <= {5*(1'b0)} ;
        min_c <= {6*(1'b0)} ;
        tick_flag <= 1'b0;
        sec_c <= {6*(1'b0)} ;  
      end
      
      end
  end

endmodule   