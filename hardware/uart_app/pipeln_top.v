module pipeln_top(clk,set_min, set_hr, set_day,set_month,tstamp,set_date_time, // digital clock ports
	adc_reading,reset,shut_down,alarm, // controller ports
	set_baud,Tx); // uart ports

input clk,set_date_time,reset;
output shut_down,alarm,Tx;
input [1:0] set_baud;  
input [7:0] adc_reading;
output [25:0] tstamp;
input [5:0] set_min;
input [4:0] set_hr, set_day;
input [3:0] set_month;
    
wire tx_enable,ticker_signal;

digital_clock clock1 (set_min, set_hr, set_day, set_month,tstamp,clk,tx_enable,ticker_signal,set_date_time);

controller fsm1 (.reading(adc_reading),.shut_down(shut_down),.alarm(alarm),.clk(clk),.reset(reset));

UART_top uart1 (.reset_tx(reset),.baud(set_baud),.inputDataBus(adc_reading),
.Tx(Tx),.clk(clk)
,.send(tx_enable),.ticker(ticker_signal));

endmodule
